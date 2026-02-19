/*
 * A L P H A G R P . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */


#include "render/Capabilities.hpp"
#include "render/internal/GLMath.hpp"
#include "render/internal/AlphaGroup.hpp"
#include "render/internal/TriangleGroup.hpp"
#include "render/internal/VertexData.hpp"
#include "render/internal/DeviceImpl.hpp"
#include "render/internal/CapabilitiesImpl.hpp"
#include "render/internal/MaterialManager.hpp"
#include "render/Device.hpp"

RenIDelayedAlphaGroup::RenIDelayedAlphaGroup(
    const RenIMaterialGroup* g,
    RenI::LitVtxAPtr v,
    const RenMaterial& m,
    const glm::mat4& x,
    RenI::GpuMeshLightingSnapshot gpuSnapshot)
    : RenIDepthSortedItem(m)
    , group_(g)
    , vertices_(std::move(v))
    , xform_(x)
    , gpuSnapshot_(std::move(gpuSnapshot))
{
    PRE(group_);
    PRE(vertices_.get());

    // If this material is sorted on intra-mesh priority, then the depth must
    // agree with the depths of other groups in the mesh.  Hence, we must use
    // a fixed vertex index.  This is checked by an assertion in
    // RenIDepthSortedItem::operator<.
    static const Ren::VertexIdx zero = 0;
    const Ren::VertexIdx idx = (m.intraMeshAlphaPriority()) ? zero : group_->firstVertexIdx();

    glm::mat4 mx = RenIDeviceImpl::currentPimpl()->projViewMatrix() * xform_;

    // Use the first vertex used by this group as the basis for the depth sort.
    const RenIVertex* tmp = vertices_.get();
    const RenIVertex& vtx = tmp[idx];
    MexPoint3d point(vtx.x, vtx.y, vtx.z);
    MATHEX_SCALAR dummyW = 1;
    transformPt(mx, &point, &dummyW);

    depth_ = point.z();
}

// virtual
RenIDelayedAlphaGroup::~RenIDelayedAlphaGroup()
{
}

// virtual
void RenIDelayedAlphaGroup::render()
{
    // SetTransform is not const-correct w.r.t. its 2nd argument.
    glm::mat4* crufty = const_cast<glm::mat4*>(&xform_);
    RenDevice::current()->setModelMatrix(*crufty);

    // Restore per-mesh GPU lighting arrays (normals + per-vertex materials)
    // that were snapshotted when this delayed group was created.
    if (gpuSnapshot_.normalsCount > 0)
        RenIDeviceImpl::currentPimpl()->restoreGpuMeshSnapshot(gpuSnapshot_);

    const bool doZBias = material_.interMeshCoplanar();
    if (doZBias)
    {
        const int zBias = material_.coplanarPriority() - RenIMatManager::instance().minCoplanarValue();
        ASSERT_INFO(zBias);
        ASSERT(zBias >= 0, "Illegal zbias value in alpha sorter.");
        ASSERT(zBias <= 16, "Illegal zbias value in alpha sorter.");

        RenDevice::current()->recordCommand(Ren::Command::setPolygonOffsetFill(true));
        RenDevice::current()->recordCommand(Ren::Command::setPolygonOffset(-zBias, 1.0f));
    }

    if (!material_.usesBilinear() && !material_.texture().isEmpty()) // This fixes issue with gun barrels rendering
    {
        RenDevice::current()->recordCommand(Ren::Command::setDepthMaskWritable(true));
        group_->render(vertices_, material_);
        RenDevice::current()->recordCommand(Ren::Command::setDepthMaskWritable(false));
    }
    else
        group_->render(vertices_, material_);

    // Unlike the coplanar groups, we cannot assume that the next item will set its
    // zbias.  So reset the drvier's state.  Perhaps we should track the current
    // value to avoid state changes.  Then agian, thre probably aren't enough of
    // them for it to be worthwhile.
    if (doZBias)
    {
        RenDevice::current()->recordCommand(Ren::Command::setPolygonOffsetFill(false));
        RenDevice::current()->recordCommand(Ren::Command::setPolygonOffset(0.0f, 1.0f));
    }
}

void RenIDelayedAlphaGroup::print(std::ostream& o) const
{
    o << "Delayed, depth=" << depth() << ", meshId=" << meshId() << ", " << material_;
}

/* End DELAYGRP.CPP *************************************************/
