/*
 * D E L A Y C O P . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/Device.hpp"
#include "render/Capabilities.hpp"
#include "render/internal/CoplanarGroup.hpp"
#include "render/internal/TriangleGroup.hpp"
#include "render/internal/VertexData.hpp"
#include "render/internal/DeviceImpl.hpp"
#include "render/internal/CapabilitiesImpl.hpp"
#include "render/internal/MaterialManager.hpp"

RenIDelayedCoplanarGroup::RenIDelayedCoplanarGroup(
    const RenIMaterialGroup* g,
    RenI::LitVtxAPtr v,
    const RenMaterial& m,
    const glm::mat4& x)
    : RenIPrioritySortedItem(m)
    , group_(g)
    , vertices_(std::move(v))
    , xform_(x)
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
    glm::mat4* crufty = const_cast<glm::mat4*>(&xform_);
    RenDevice::current()->setModelMatrix(*crufty);

    const RenICapabilities* caps = RenIDeviceImpl::currentPimpl()->capabilities().internal();
    ASSERT(caps, "No internal device capabilities defined.");

    // Apply a zBias value only if the device supports it (to avoid state changes).
    if (caps->supportsZBias())
    {
        const int zBias = material_.coplanarPriority() - RenIMatManager::instance().minCoplanarValue();
        ASSERT_INFO(zBias);
        ASSERT_INFO(material_.coplanarPriority());
        ASSERT_INFO(RenIMatManager::instance().minCoplanarValue());
        ASSERT_INFO(RenIMatManager::instance().maxCoplanarValue());
        ASSERT(zBias >= caps->minZBias(), "Illegal zbias value in coplanar sorter.");
        ASSERT(zBias <= caps->maxZBias(), "Illegal zbias value in coplanar sorter.");

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-(GLfloat)zBias, 1.0);
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
