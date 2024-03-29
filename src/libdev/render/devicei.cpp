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

/* End DEVICEI.CPP ***************************************************/
