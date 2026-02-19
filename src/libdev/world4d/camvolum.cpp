/*
 * C A M V O L U M . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "world4d/camvolum.hpp"
#include "world4d/composit.hpp"
#include "world4d/camera.hpp"

#include "mathex/quad3d.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/abox3d.hpp"

W4dCameraVolume::W4dCameraVolume(const W4dCamera& camera)
    : pCamera_(&camera)
    , clipFarPlane_(true)
{
    // Update the data
    update();

    TEST_INVARIANT;
}

W4dCameraVolume::~W4dCameraVolume()
{
    TEST_INVARIANT;
    // Nothing to do
}

void W4dCameraVolume::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const W4dCameraVolume& t)
{

    o << "W4dCameraVolume " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "W4dCameraVolume " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void W4dCameraVolume::update()
{
    // Get the camera's global transform
    const MexTransform3d& cameraTransform = pCamera_->globalTransform();

    // Set up the front and back clipping plane points and the eye point
    // in global coords
    nearPlanePoint_.setPoint(pCamera_->hitherClipDistance(), 0.0, 0.0);
    farPlanePoint_.setPoint(pCamera_->yonClipDistance(), 0.0, 0.0);
    eyePoint_.setPoint(0.0, 0.0, 0.0);

    cameraTransform.transform(&nearPlanePoint_);
    cameraTransform.transform(&farPlanePoint_);
    cameraTransform.transform(&eyePoint_);

    // Get the vertical and horizontal field of view angles, and hence their
    // sines and cosines.
    double halfAlpha = 0.5 * pCamera_->horizontalFOVAngle();
    double halfBeta = 0.5 * pCamera_->verticalFOVAngle();
    MATHEX_SCALAR sinHalfAlpha = sin(halfAlpha);
    MATHEX_SCALAR cosHalfAlpha = cos(halfAlpha);
    MATHEX_SCALAR sinHalfBeta = sin(halfBeta);
    MATHEX_SCALAR cosHalfBeta = cos(halfBeta);

    // Set up the plane normals and convert to global
    lineOfSight_.setVector(1.0, 0.0, 0.0);
    horizontalRightNormal_.setVector(-sinHalfAlpha, cosHalfAlpha, 0.0);
    horizontalLeftNormal_.setVector(-sinHalfAlpha, -cosHalfAlpha, 0.0);
    verticalUpNormal_.setVector(-sinHalfBeta, 0.0, cosHalfBeta);
    verticalDownNormal_.setVector(-sinHalfBeta, 0.0, -cosHalfBeta);

    cameraTransform.transform(&lineOfSight_);
    cameraTransform.transform(&horizontalRightNormal_);
    cameraTransform.transform(&horizontalLeftNormal_);
    cameraTransform.transform(&verticalUpNormal_);
    cameraTransform.transform(&verticalDownNormal_);
}

bool W4dCameraVolume::intersects(const W4dEntity& entity) const
{
    // Get the local bounding volume of the entity. If it is a composite, get the
    // composite bounding volume
    const MexAlignedBox3d& bv
        = (entity.isComposite() ? entity.asComposite().compositeBoundingVolume() : entity.boundingVolume());

    // Hence get the centroid of the bounding volume, and convert to global coords
    const MexPoint3d& minCorner = bv.minCorner();
    const MexPoint3d& maxCorner = bv.maxCorner();
    MATHEX_SCALAR xMin = minCorner.x();
    MATHEX_SCALAR yMin = minCorner.y();
    MATHEX_SCALAR zMin = minCorner.z();
    MATHEX_SCALAR xMax = maxCorner.x();
    MATHEX_SCALAR yMax = maxCorner.y();
    MATHEX_SCALAR zMax = maxCorner.z();
    MexPoint3d centroid(0.5 * (xMin + xMax), 0.5 * (yMin + yMax), 0.5 * (zMin + zMax));
    entity.globalTransform().transform(&centroid);

    // Also get the squared radius of the enclosing sphere
    MATHEX_SCALAR xDiff = xMax - xMin;
    MATHEX_SCALAR yDiff = yMax - yMin;
    MATHEX_SCALAR zDiff = zMax - zMin;
    MATHEX_SCALAR sqrRadius = 0.25 * (xDiff * xDiff + yDiff * yDiff + zDiff * zDiff);

    // Now check near clipping plane
    MATHEX_SCALAR h = MexVec3(nearPlanePoint_, centroid).dotProduct(lineOfSight_);
    bool result = h > 0.0 || (h * h < sqrRadius);

    if (result)
    {
        // Check far clipping plane if enabled
        if (clipFarPlane_)
        {
            h = MexVec3(farPlanePoint_, centroid).dotProduct(lineOfSight_);
            result = h < 0.0 || (h * h < sqrRadius);
        }

        if (result)
        {
            // get the vector from eye point to centroid
            MexVec3 v(eyePoint_, centroid);

            // Check right horizontal clipping
            h = v.dotProduct(horizontalRightNormal_);
            result = h < 0.0 || (h * h < sqrRadius);

            if (result)
            {
                // Check left horizontal clipping
                h = v.dotProduct(horizontalLeftNormal_);
                result = h < 0.0 || (h * h < sqrRadius);

                if (result)
                {
                    // Check up vertical clipping
                    h = v.dotProduct(verticalUpNormal_);
                    result = h < 0.0 || (h * h < sqrRadius);

                    // The down vertical plane test is deliberately skipped.
                    // For a zenith camera, this plane clips flat ground-level
                    // entities whose bounding sphere centroid falls just outside
                    // the plane.  The GPU handles actual screen-edge clipping.
                }
            }
        }
    }

    return result;
}

bool W4dCameraVolume::canSee(const MexQuad3d& quad) const
{
    // Test the portal quad against the full frustum (4 side planes + behind
    // eye + far plane).  If all 4 vertices of the quad lie outside any single
    // plane, the portal is entirely outside the frustum and we skip traversal.
    //
    // The near plane is deliberately skipped so that portals between the eye
    // and the near clip plane are still traversed (e.g. the camera's own room
    // boundary).
    //
    // Sign convention (same as intersects): for a side plane normal N,
    //   dot(N, P - eyePoint) < 0  =>  P is on the inside (visible) side.
    //   dot(N, P - eyePoint) >= 0 =>  P is on the outside (culled) side.
    // For the line-of-sight direction:
    //   dot(lineOfSight, P - eyePoint) < 0  =>  P is behind the camera.
    //   dot(lineOfSight, P - farPlane) > 0   =>  P is beyond the far plane.

    // 1. Behind-camera test: reject if all vertices are behind the eye.
    {
        bool allBehind = true;
        for (uint i = 0; i < 4 && allBehind; ++i)
        {
            const MATHEX_SCALAR h = MexVec3(eyePoint_, quad.vertex(i)).dotProduct(lineOfSight_);
            if (h >= 0.0)
                allBehind = false;
        }
        if (allBehind)
            return false;
    }

    // 2. Far plane test: reject if all vertices are beyond the yon clip plane.
    if (clipFarPlane_)
    {
        bool allBeyond = true;
        for (uint i = 0; i < 4 && allBeyond; ++i)
        {
            const MATHEX_SCALAR h = MexVec3(farPlanePoint_, quad.vertex(i)).dotProduct(lineOfSight_);
            if (h <= 0.0)
                allBeyond = false;
        }
        if (allBeyond)
            return false;
    }

    // 3. Four side frustum planes (left, right, up, down).
    const MexVec3* planes[4] = {
        &horizontalRightNormal_,
        &horizontalLeftNormal_,
        &verticalUpNormal_,
        &verticalDownNormal_,
    };

    for (const MexVec3* plane : planes)
    {
        bool allOutside = true;
        for (uint i = 0; i < 4 && allOutside; ++i)
        {
            const MATHEX_SCALAR h = MexVec3(eyePoint_, quad.vertex(i)).dotProduct(*plane);
            if (h < 0.0)
                allOutside = false;
        }
        if (allOutside)
            return false;
    }

    return true;
}

void W4dCameraVolume::isYonClippingEnabled(bool isIt)
{
    clipFarPlane_ = isIt;
}

bool W4dCameraVolume::isYonClippingEnabled() const
{
    return clipFarPlane_;
}

/* End CAMVOLUM.CPP *************************************************/
