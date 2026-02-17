/*
 * D E V I C E I . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "base/diag.hpp"
#include "ctl/countptr.hpp"
#include "render/internal/devicei.hpp"

#include "render/device.hpp"
#include "render/display.hpp"
#include "render/texture.hpp"
#include "render/surfmgr.hpp"
#include "render/drivsel.hpp"
#include "render/material.hpp"

#include "render/internal/IRenderBackend.hpp"

#include "system/pathname.hpp"
#include "render/internal/matmgr.hpp"
#include "render/internal/displayi.hpp"
#include "render/internal/surfmgri.hpp"
#include "render/internal/matbody.hpp"

#ifndef _INLINE
#include "render/internal/devicei.ipp"
#endif

RenIDeviceImpl::RenIDeviceImpl(RenDisplay* dis, RenDevice* parent)
    : RenIDisplayModeObserver(dis)
    , parent_(parent)
    , alphaBlendingEnabled_(false)
    , driverSelector_(nullptr)
    , materialFogMultiplier_(1.0)
    , debugX_(0)
    , debugY_(0)
    , backend_(Ren::IRenderBackend::create())
    , frameTimer_()
    , videoMemoryShared_(false)
    , videoMemorySharedInitialized_(false)

{
    PRE(dis);
    PRE(parent);
    frameTimer_.pause();
    frameTimer_.time(0);
    driverSelector_ = new RenDriverSelector(dis);
}

RenIDeviceImpl::~RenIDeviceImpl()
{
    delete driverSelector_;
}

// virtual
void RenIDeviceImpl::prepareForModeChange(const RenDisplay::Mode&, const RenDisplay::Mode&)
{
    // Ensure that any front and back surfaces release their COM ptrs.
    const RenIDisplay& dis = display_->displayImpl();

    RENDER_STREAM("Releasing D3D stuff owned by RenDevice.\n");
    delete surfBackBuf_;
    surfBackBuf_ = nullptr;
    delete surfFrontBuf_;
    surfFrontBuf_ = nullptr;

    RENDER_STREAM("...done.\n\n");
}

// virtual
bool RenIDeviceImpl::modeChanged(const RenDisplay::Mode&)
{
    RENDER_STREAM("Reinitialising RenDevice after mode change.\n");
    return parent_->fitToDisplay(display_);
}

// static
void RenIDeviceImpl::useDevice(RenDevice* newDev, RenI::UpdateType updateType)
{
    PRE(newDev);

    current() = newDev;
    currentPimpl() = &(newDev->impl());

    RenIMatManager::instance().useDevice(*newDev);

    const RenIDisplay& dis = newDev->display()->displayImpl();
    RenSurfaceManager::instance().impl().useDevice(newDev, updateType);
}

bool RenIDeviceImpl::hasSharedVideoMemory() const
{
    PRE(videoMemorySharedInitialized_);
    return videoMemoryShared_;
}

void RenIDeviceImpl::hasSharedVideoMemory(bool setVideoMemoryShared)
{
    videoMemoryShared_ = setVideoMemoryShared;
    videoMemorySharedInitialized_ = true;
}

void RenIDeviceImpl::clearGpuLightingState()
{
    expandedNormalsCount_ = 0;
    hasPerVertexMaterials_ = false;
}

void RenIDeviceImpl::enableAlphaBlending()
{
    PRE(parent_);
    PRE(frameCommandBuffer_.isValid());

    if (!alphaBlendingEnabled_)
    {
        using BlendFactor = Ren::BackendBlendFactor;
        parent_->recordCommand(
            Ren::Command::setBlendStateEnabled(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha));
        alphaBlendingEnabled_ = true;
    }
}

void RenIDeviceImpl::disableAlphaBlending()
{
    PRE(parent_);
    PRE(frameCommandBuffer_.isValid());

    if (alphaBlendingEnabled_)
    {
        parent_->recordCommand(Ren::Command::setBlendStateDisabled());
        alphaBlendingEnabled_ = false;
    }
}

void RenIDeviceImpl::resetFrameTimer()
{
    frameTimer_.time(0);
    frameTimer_.resume();
}

double RenIDeviceImpl::frameTime() const
{
    return frameTimer_.time();
}

void RenIDeviceImpl::setMaterialHandles(const RenMaterial& mat)
{
    if (fogOn_)
        updateFogMultiplier(mat);
}

void RenIDeviceImpl::updateFogMultiplier(const RenMaterial& mat)
{
    PRE(fogOn_);

    const RenIMatBody& body = *mat.body_;

    if (body.hasFogMultiplier())
    {
        if (body.fogMultiplier() != materialFogMultiplier_)
        {
            materialFogMultiplier_ = body.fogMultiplier();
            parent_->fogEnd(fogEnd_ * materialFogMultiplier_);
        }
    }
    else if (! fogMultiplierIsNeutral())
    {
        materialFogMultiplier_ = 1.0;
        parent_->fogEnd(fogEnd_);
    }
}

void RenIDeviceImpl::beginFrameCommandBuffer()
{
    PRE(backend_);
    PRE(!frameCommandBuffer_.isValid());
    PRE(!frameCommandBufferRecording_);

    frameCommandBuffer_ = backend_->createCommandBuffer();

    backend_->beginCommandBuffer(frameCommandBuffer_);
    frameCommandBufferRecording_ = true;
}

void RenIDeviceImpl::destroyFrameCommandBuffer()
{
    PRE(backend_);
    PRE(frameCommandBufferRecording_);

    backend_->endCommandBuffer(frameCommandBuffer_);
    frameCommandBufferRecording_ = false;

    backend_->submitCommandBuffer(frameCommandBuffer_);
    backend_->destroyCommandBuffer(frameCommandBuffer_);
    frameCommandBuffer_ = {};
}

void RenIDeviceImpl::flushFrameCommandBuffer()
{
    PRE(backend_);
    PRE(frameCommandBufferRecording_);

    backend_->endCommandBuffer(frameCommandBuffer_);
    backend_->submitCommandBuffer(frameCommandBuffer_);
    backend_->destroyCommandBuffer(frameCommandBuffer_);

    frameCommandBuffer_ = backend_->createCommandBuffer();
    backend_->beginCommandBuffer(frameCommandBuffer_);
}

void RenIDeviceImpl::beginImmediateCommandBuffer()
{
    PRE(backend_);
    PRE(!immediateCommandBuffer_.isValid());

    immediateCommandBuffer_ = backend_->createCommandBuffer();
    backend_->beginCommandBuffer(immediateCommandBuffer_);
}

void RenIDeviceImpl::endImmediateCommandBuffer()
{
    PRE(backend_);
    PRE(immediateCommandBuffer_.isValid());

    backend_->endCommandBuffer(immediateCommandBuffer_);
    backend_->submitCommandBuffer(immediateCommandBuffer_);
    backend_->destroyCommandBuffer(immediateCommandBuffer_);
    immediateCommandBuffer_ = {};
}

bool RenIDeviceImpl::immediateCommandBufferActive() const
{
    return immediateCommandBuffer_.isValid();
}

Ren::BackendCommandBufferHandle RenIDeviceImpl::currentCommandBufferHandle() const
{
    if (immediateCommandBuffer_.isValid())
        return immediateCommandBuffer_;

    PRE(frameCommandBufferRecording_);
    return frameCommandBuffer_;
}

/* End DEVICEI.CPP ***************************************************/
