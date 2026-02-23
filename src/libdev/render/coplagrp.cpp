/*
 * D E L A Y C O P . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/device.hpp"
#include "render/internal/coplagrp.hpp"
#include "render/internal/trigroup.hpp"
#include "render/internal/vtxdata.hpp"
#include "render/internal/devicei.hpp"
#include "render/internal/matmgr.hpp"

RenIDelayedCoplanarGroup::RenIDelayedCoplanarGroup(
    const RenIMaterialGroup* g,
    RenI::LitVtxAPtr v,
    const RenMaterial& m,
    const glm::mat4& x,
    RenI::GpuMeshLightingSnapshot gpuSnapshot)
    : RenIPrioritySortedItem(m)
    , group_(g)
    , vertices_(std::move(v))
    , xform_(x)
    , gpuSnapshot_(std::move(gpuSnapshot))
{
    PRE(group_);
    PRE(vertices_.get());
}

// virtual
RenIDelayedCoplanarGroup::~RenIDelayedCoplanarGroup()
{
}

// virtual
void RenIDelayedCoplanarGroup::render()
{
    glm::mat4* crufty = _CONST_CAST(glm::mat4*, &xform_);
    RenDevice::current()->setModelMatrix(*crufty);

    // Restore per-mesh GPU lighting arrays (normals + per-vertex materials)
    // that were snapshotted when this delayed group was created.
    if (gpuSnapshot_.normalsCount > 0)
        RenIDeviceImpl::currentPimpl()->restoreGpuMeshSnapshot(gpuSnapshot_);

    {
        const int zBias = material_.coplanarPriority() - RenIMatManager::instance().minCoplanarValue();
        ASSERT_INFO(zBias);
        ASSERT_INFO(material_.coplanarPriority());
        ASSERT_INFO(RenIMatManager::instance().minCoplanarValue());
        ASSERT_INFO(RenIMatManager::instance().maxCoplanarValue());
        ASSERT(zBias >= 0, "Illegal zbias value in coplanar sorter.");
        ASSERT(zBias <= 16, "Illegal zbias value in coplanar sorter.");

        RenDevice::current()->recordCommand(Ren::Command::setPolygonOffsetFill(true));
        RenDevice::current()->recordCommand(Ren::Command::setPolygonOffset(static_cast<float>(-zBias), 1.0f));
    }

    group_->render(vertices_, material_);

    // Assume that we don't need to reset the zbias because the next item rendered
    // will probably be another coplanar group.  Cf. alpha groups.
    // TBD: post-conditions etc. would be nice.
}

void RenIDelayedCoplanarGroup::print(std::ostream& o) const
{
    o << "Delayed co-planar, depth=" << depth() << ", " << material_;
}

/* End DELAYGRP.CPP *************************************************/
