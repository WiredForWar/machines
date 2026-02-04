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

#include "render/OpenGL/RenderBackendGL.hpp"

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
    , backend_(std::make_unique<Ren::OpenGL::RenderBackendGL>())
    , glFramebuffers_{0,}
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

void RenIDeviceImpl::releaseGLProgram(Ren::ProgramId id)
{
    renderBackend().releaseProgram(id);
}

RenIRenderBackend& RenIDeviceImpl::renderBackend()
{
    return *backend_;
}

const RenIRenderBackend& RenIDeviceImpl::renderBackend() const
{
    return *backend_;
}

Ren::FramebufferId RenIDeviceImpl::addGLFramebuffer(GLuint framebuffer)
{
    if (framebuffer == 0)
        return 0;

    glFramebuffers_.push_back(framebuffer);
    return static_cast<Ren::FramebufferId>(glFramebuffers_.size() - 1);
}

GLuint RenIDeviceImpl::glFramebufferHandle(Ren::FramebufferId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= glFramebuffers_.size())
        return 0;

    return glFramebuffers_[idx];
}

void RenIDeviceImpl::releaseGLFramebuffer(Ren::FramebufferId id)
{
    if (id == 0)
        return;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= glFramebuffers_.size())
        return;

    const GLuint framebuffer = glFramebuffers_[idx];
    if (framebuffer != 0)
    {
        glDeleteFramebuffers(1, &framebuffer);
        glFramebuffers_[idx] = 0;
    }
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
