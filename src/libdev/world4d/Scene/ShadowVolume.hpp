#pragma once

#include "mathex/Point3d.hpp"
#include "mathex/Vec3.hpp"

#include <optional>
#include <vector>

class MexQuad3d;
class W4dEntity;

// The volume a shadow pass must draw, which is not the volume the camera can
// see. What has to be recorded is whatever stands between a light and the
// surfaces on screen, so geometry behind the camera or off the side of it
// belongs in a shadow map even though it belongs in no frame.
//
// The shape is the ground the cascades cover, opened out along the light
// direction and closed everywhere else. A caster matters if its shadow could
// land in that ground, so leaving the side facing the light open is the point
// -- a ridge is no less a caster for standing high above what it shades -- and
// closing the other four sides is what keeps this cheaper than drawing the
// world.
//
// A plane is stored as a point it passes through and the normal pointing out
// of the volume, so a positive distance means outside. Both tests are
// conservative: they answer "may be inside", never "is inside".
class W4dShadowVolume
{
public:
    W4dShadowVolume() = default;

    // Adds the box the cascades cover, opened out along the direction the light
    // travels. `centre` is the middle of the covered ground, `extent` is how
    // far the cover reaches either side of it, and `reach` is how far past that
    // ground there is still something for a shadow to fall on.
    void addCascadeReach(
        const MexVec3& lightDirection,
        const MexPoint3d& centre,
        MATHEX_SCALAR extent,
        MATHEX_SCALAR reach);

    // True if the entity may be inside, so may cast into what the camera sees.
    bool intersects(const W4dEntity& entity) const;

    // True if any part of the quad may be inside. The portal walk asks this of
    // an aperture to decide whether to carry on through it.
    bool canSee(const MexQuad3d& quad) const;

private:
    struct Plane
    {
        MexPoint3d point;
        MexVec3 outward;
    };

    // A convex region, bounded by the planes in it. A point is inside when it
    // is inside every plane.
    using Region = std::vector<Plane>;

    // Half a sphere: within `radius` of `centre` and not behind its `facing`.
    struct Reach
    {
        MexPoint3d centre;
        MATHEX_SCALAR radius{};
        MexVec3 facing;
    };

    static bool holdsSphere(const Region& region, const MexPoint3d& centre, MATHEX_SCALAR sqrRadius);
    static bool holdsQuad(const Region& region, const MexQuad3d& quad);

    bool reachesSphere(const MexPoint3d& centre, MATHEX_SCALAR radius) const;

    std::vector<Reach> reaches_;
    std::optional<Region> cascades_;
};
