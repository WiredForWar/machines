/*
 * DLYSPIN . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "mathex/Point3d.hpp"
#include "render/internal/DelayedSpinPolygon.hpp"
#include "render/internal/GLMath.hpp"
#include "render/internal/DeviceImpl.hpp"
#include "render/internal/VertexData.hpp"
#include "render/Device.hpp"

RenIDelayedSpinPolygon::RenIDelayedSpinPolygon(
    RenI::LitVtxAPtr v, // array of spun and lit vertices
    size_t nVtx, // the number of elements in v
    const RenMaterial& mat, // material to render with
    const glm::mat4& x // the global xfrom of the STF
    )
    : RenIDepthSortedItem(mat)
    , vertices_(std::move(v))
    , nVertices_(nVtx)
    , xform_(x)
{
    PRE(nVtx >= 3);

    // Pick an arbitrary vertex -- the first -- transform by the projection-
    // view matrix and use that as the basis for the depth sorting.  Using
    // the STF's base point might give better results??
    const glm::mat4& projView = RenIDeviceImpl::currentPimpl()->projViewMatrix();

    // Use the first vertex used by this group as the basis for the depth sort.
    const RenIVertex* tmp = vertices_.get();
    const RenIVertex& vtx = tmp[0];
    MexPoint3d point(vtx.x, vtx.y, vtx.z);
    MATHEX_SCALAR dummyW = 1;
    transformPt(projView, &point, &dummyW);

    depth_ = point.z();
}

RenIDelayedSpinPolygon::~RenIDelayedSpinPolygon()
{
}

void RenIDelayedSpinPolygon::render()
{
    // SetTransform is not const-correct w.r.t. its 2nd argument.
    glm::mat4* crufty = const_cast<glm::mat4*>(&xform_);
    RenDevice::current()->setModelMatrix(*crufty);

    // Pre-lit vertices already have CPU lighting baked in; disable GPU lighting.
    RenIDeviceImpl::currentPimpl()->clearGpuLightingState();

    RenIDeviceImpl::currentPimpl()->setMaterialHandles(material_);

    // A spin polygon is a single face turned to the camera about its axis, and
    // which way round it comes out depends on where the camera stands. RenMesh
    // draws them with face culling off for that reason; the sorter runs with it
    // on, so without this the ones that happen to face away are dropped -- one
    // arm of a beacon's cross drawn and the other missing.
    RenDevice::current()->recordCommand(Ren::Command::setCullFace(false));

    RenIVertex* vtx = vertices_.get();
    RenDevice::current()->renderPrimitive(vtx, nVertices_, material_);

    RenDevice::current()->recordCommand(Ren::Command::setCullFace(true));
}

void RenIDelayedSpinPolygon::print(std::ostream& o) const
{
    o << "Delayed STF, depth=" << depth() << ", " << material_;
}

/* End DLYSPIN.CPP ***************************************************/
