#include "world4d/Scene/ShadowVolume.hpp"

#include "world4d/Entity/Composite.hpp"
#include "world4d/Entity/Entity.hpp"

#include "mathex/AlignedBox3d.hpp"
#include "mathex/Quad3d.hpp"
#include "mathex/Transform3d.hpp"
#include "mathex/mathex.hpp"

#include <cmath>

namespace
{

// The enclosing sphere of an entity, in world coordinates. The same conservative
// stand-in the camera's own volume tests against: a box rotated into world space
// is not a box, and the sphere around it is.
void entitySphere(const W4dEntity& entity, MexPoint3d* pCentre, MATHEX_SCALAR* pSqrRadius)
{
    const MexAlignedBox3d& bv
        = (entity.isComposite() ? entity.asComposite().compositeBoundingVolume() : entity.boundingVolume());

    const MexPoint3d& minCorner = bv.minCorner();
    const MexPoint3d& maxCorner = bv.maxCorner();

    MexPoint3d centroid(
        0.5 * (minCorner.x() + maxCorner.x()),
        0.5 * (minCorner.y() + maxCorner.y()),
        0.5 * (minCorner.z() + maxCorner.z()));
    entity.globalTransform().transform(&centroid);

    const MATHEX_SCALAR xDiff = maxCorner.x() - minCorner.x();
    const MATHEX_SCALAR yDiff = maxCorner.y() - minCorner.y();
    const MATHEX_SCALAR zDiff = maxCorner.z() - minCorner.z();

    *pCentre = centroid;
    *pSqrRadius = 0.25 * (xDiff * xDiff + yDiff * yDiff + zDiff * zDiff);
}

}

void W4dShadowVolume::addCascadeReach(
    const MexVec3& lightDirection,
    const MexPoint3d& centre,
    MATHEX_SCALAR extent,
    MATHEX_SCALAR reach)
{
    MexVec3 forward(lightDirection);
    forward.makeUnitVector();

    // Any up will do so long as it is not the light direction itself, which
    // would leave the cross product without a plane to work in.
    const MexVec3 up = (std::abs(forward.z()) > 0.99) ? MexVec3(0.0, 1.0, 0.0) : MexVec3(0.0, 0.0, 1.0);

    MexVec3 right = MexVec3::crossProduct(up, forward);
    right.makeUnitVector();
    MexVec3 across = MexVec3::crossProduct(forward, right);
    across.makeUnitVector();

    const auto offset = [&centre](const MexVec3& along, MATHEX_SCALAR distance) {
        return MexPoint3d(
            centre.x() + along.x() * distance,
            centre.y() + along.y() * distance,
            centre.z() + along.z() * distance);
    };

    // Widened by the diagonal of a square. The cascades choose their own pair
    // of axes across the light -- for a ground camera, from where the camera is
    // facing -- and there is no reason for the pair chosen here to agree. Two
    // squares of the same half-width turned against each other do not contain
    // one another, and the corners that stick out are exactly where a caster
    // would be dropped, so this one is grown until it holds the other whatever
    // the angle between them.
    const MATHEX_SCALAR held = extent * Mathex::SQRT_2;

    Region region;
    region.push_back({ offset(right, held), right });
    region.push_back({ offset(right, -held), MexVec3(-right.x(), -right.y(), -right.z()) });
    region.push_back({ offset(across, held), across });
    region.push_back({ offset(across, -held), MexVec3(-across.x(), -across.y(), -across.z()) });

    // Closed past the far side of the ground the cascades cover, and left open
    // on the side the light comes from: there is no distance at which a caster
    // stops casting.
    region.push_back({ offset(forward, reach), forward });

    cascades_ = std::move(region);
}

bool W4dShadowVolume::holdsSphere(const Region& region, const MexPoint3d& centre, MATHEX_SCALAR sqrRadius)
{
    for (const Plane& plane : region)
    {
        const MATHEX_SCALAR h = MexVec3(plane.point, centre).dotProduct(plane.outward);

        // Wholly on the outward side of one plane, and further off it than the
        // sphere is wide, is wholly outside a convex region.
        if (h > 0.0 && h * h > sqrRadius)
            return false;
    }

    return true;
}

bool W4dShadowVolume::holdsQuad(const Region& region, const MexQuad3d& quad)
{
    for (const Plane& plane : region)
    {
        bool allOutside = true;
        for (uint i = 0; i < 4 && allOutside; ++i)
        {
            if (MexVec3(plane.point, quad.vertex(i)).dotProduct(plane.outward) <= 0.0)
                allOutside = false;
        }

        if (allOutside)
            return false;
    }

    return true;
}

bool W4dShadowVolume::reachesSphere(const MexPoint3d& centre, MATHEX_SCALAR radius) const
{
    for (const Reach& reach : reaches_)
    {
        const MATHEX_SCALAR farthest = reach.radius + radius;
        if (reach.centre.sqrEuclidianDistance(centre) > farthest * farthest)
            continue;

        // Wholly behind the face the light emits from cannot stand in its way.
        if (MexVec3(reach.centre, centre).dotProduct(reach.facing) < -radius)
            continue;

        return true;
    }

    return false;
}

bool W4dShadowVolume::intersects(const W4dEntity& entity) const
{
    MexPoint3d centre;
    MATHEX_SCALAR sqrRadius{};
    entitySphere(entity, &centre, &sqrRadius);

    if (reachesSphere(centre, std::sqrt(sqrRadius)))
        return true;

    return cascades_.has_value() && holdsSphere(*cascades_, centre, sqrRadius);
}

bool W4dShadowVolume::canSee(const MexQuad3d& quad) const
{
    // The quad as the sphere around it, which is all the reach test needs and
    // cheaper than a real quad test: an aperture the volume nearly reaches
    // admits a domain it nearly needed, and that errs the safe way.
    MexPoint3d centre(0.0, 0.0, 0.0);
    for (uint i = 0; i < 4; ++i)
    {
        const MexPoint3d& vertex = quad.vertex(i);
        centre.x(centre.x() + 0.25 * vertex.x());
        centre.y(centre.y() + 0.25 * vertex.y());
        centre.z(centre.z() + 0.25 * vertex.z());
    }

    MATHEX_SCALAR sqrRadius = 0.0;
    for (uint i = 0; i < 4; ++i)
        sqrRadius = std::max(sqrRadius, centre.sqrEuclidianDistance(quad.vertex(i)));

    if (reachesSphere(centre, std::sqrt(sqrRadius)))
        return true;

    return cascades_.has_value() && holdsQuad(*cascades_, quad);
}
