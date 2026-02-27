/*
 * D I S P L A Y . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include <stdio.h>
#include <iostream>
#include "base/Diag.hpp"
#include "ctl/CountedPtr.hpp"
#include "ctl/List.hpp"
#include "ctl/Algorithm.hpp"
#include "system/PathName.hpp"
#include "render/Display.hpp"
#include "render/IWindowAdapter.hpp"
#include "render/render.hpp"
#include "render/DriverSelector.hpp"
#include "render/Surface.hpp"
#include "render/internal/DisplayImpl.hpp"
#include "render/internal/DisplayImpl.hpp"

#include "spdlog/spdlog.h"

#include "device/Timer.hpp"

#define CB_RenDisplay_DEPIMPL()                                                                                        \
    CB_DEPIMPL(std::vector<RenDisplay::Mode>, modeList_);                                                              \
    CB_DEPIMPL(RenDisplay::Mode, currentMode_);                                                                        \
    CB_DEPIMPL(bool, fullscreen_);                                                                                     \
    CB_DEPIMPL(uint32_t, frameNo_);                                                                                    \
    CB_DEPIMPL(bool, supportsGammaCorrection_);                                                                        \
    CB_DEPIMPL(double, gammaCorrection_);                                                                              \
    CB_DEPIMPL(RenDisplay::Mode, lowestAllowedMode_);                                                                  \
    CB_DEPIMPL(RenDisplay::Mode, highestAllowedMode_);                                                                 \
    CB_DEPIMPL(bool, isPrimaryDriver_);

//////////////////////////////////////////////////////////////////////////
RenDisplay::RenDisplay(Ren::IWindowAdapter* adapter)
    : pImpl_(new RenIDisplay(adapter))
{
    CB_RenDisplay_DEPIMPL();
    PRE(Ren::initialised());

    isPrimaryDriver_ = true;

    TEST_INVARIANT;
}

RenDisplay::~RenDisplay()
{
    TEST_INVARIANT;
    CB_RenDisplay_DEPIMPL();

    if (fullscreen_)
    {
        resetToNormalScreen();
    }
    delete pImpl_;
}

void RenDisplay::buildDisplayModesList()
{
    CB_RenDisplay_DEPIMPL();

    auto* adapter = pImpl_->adapter_;
    if (!adapter)
        return;

    const auto desktopMode = adapter->desktopDisplayMode();
    if (desktopMode.width > 0)
    {
        spdlog::info(
            "Current desktop display mode: {}x{}@{}bpp ({} Hz)",
            desktopMode.width,
            desktopMode.height,
            desktopMode.depth,
            desktopMode.refreshRate);
    }
    else
    {
        spdlog::warn("Unable to get the current desktop display mode");
    }

    const auto adapterModes = adapter->availableDisplayModes();
    modeList_.clear();
    modeList_.reserve(adapterModes.size());

    RENDER_STREAM("Number of display modes: " << adapterModes.size() << std::endl);
    for (const auto& am : adapterModes)
    {
        modeList_.push_back(Mode(am.width, am.height, am.depth, am.refreshRate, am.format));
    }

    std::sort(modeList_.begin(), modeList_.end());

    if (!modeList_.empty())
    {
        lowestAllowedMode_ = modeList_.front();
        highestAllowedMode_ = modeList_.back();
    }
}

const RenDisplay::Mode RenDisplay::getDesktopDisplayMode() const
{
    auto* adapter = pImpl_->adapter_;
    if (!adapter)
        return Mode();

    // Got no current desktop mode
    const auto dm = adapter->desktopDisplayMode();
    if (dm.width == 0)
        return Mode();

    return Mode(dm.width, dm.height, dm.depth, dm.refreshRate, dm.format);
}

const RenDisplay::Mode RenDisplay::getFailSafeDisplayMode() const
{
    return Mode(640, 480, 0);
}

bool RenDisplay::useFullScreen()
{
    CB_RenDisplay_DEPIMPL();
    if (fullscreen_)
        return true;

    auto* adapter = pImpl_->adapter_;
    if (adapter)
        adapter->setFullscreen(true);
    fullscreen_ = true;

    // If we suceed in going fullscreen, then list the display modes
    // available in fullscreen.
    if (fullscreen_)
    {
        std::sort(modeList_.begin(), modeList_.end());

        lowestAllowedMode_ = modeList_.front();
        highestAllowedMode_ = modeList_.back();
    }

    return fullscreen_;
}

void RenDisplay::resetToNormalScreen()
{
    CB_RenDisplay_DEPIMPL();
    auto* adapter = pImpl_->adapter_;
    if (adapter)
        adapter->setFullscreen(false);
    fullscreen_ = adapter ? adapter->isFullscreen() : false;

    // Return to the primary DD driver.
    if (!fullscreen_)
        modeList_.erase(modeList_.begin(), modeList_.end());
}

bool RenDisplay::fullScreen() const
{
    CB_RenDisplay_DEPIMPL();
    return fullscreen_;
}

const std::vector<RenDisplay::Mode>& RenDisplay::modeList() const
{
    CB_RenDisplay_DEPIMPL();
    return modeList_;
}

bool RenDisplay::useMode(const RenDisplay::Mode& m)
{
    CB_RenDisplay_DEPIMPL();
    PRE(m.width() > 0);
    PRE(m.height() > 0);
    PRE(m.bitDepth() > 0);

    if (m == currentMode_)
        return true;

    spdlog::info(
        "Setting display mode to: {}x{}@{}bpp ({} Hz; fullscreen: {})",
        m.width(),
        m.height(),
        m.bitDepth(),
        m.refreshRate(),
        fullscreen_);

    pImpl_->prepareForModeChange(m);

    Mode saveMode = currentMode_;
    currentMode_ = m;

    auto* adapter = pImpl_->adapter_;
    const Ren::IWindowAdapter::DisplayMode adapterMode{
        m.width(), m.height(), m.bitDepth(), m.refreshRate(), m.format_,
    };
    bool success = adapter ? adapter->useMode(adapterMode) : false;
    if (adapter)
        fullscreen_ = adapter->isFullscreen();

    if (!success || (! pImpl_->modeChanged()))
    {
        // Failed, attempt to restore state before attempt to change mode
        RENDER_STREAM("Failed to switch to mode: " << m << std::endl);
        RENDER_STREAM("Try restoring mode: " << saveMode << std::endl);
        RENDER_INDENT(2);
        pImpl_->prepareForModeChange(saveMode);
        currentMode_ = saveMode;
        bool restoreSuccess;
        ASSERT(restoreSuccess, "Could not restore mode");
        restoreSuccess = pImpl_->modeChanged();
        ASSERT(restoreSuccess, "Could not restore mode");
        RENDER_INDENT(-2);
        RENDER_STREAM("Restored mode " << saveMode << std::endl);
        return false;
    }
    else
    {

        RENDER_STREAM("Switched to mode: " << m << std::endl);
    }

    return true;
}

const RenDisplay::Mode RenDisplay::findMode(int width, int height, int refreshRate)
{
    CB_RenDisplay_DEPIMPL();
    PRE(width > 0);
    PRE(height > 0);
    // PRE(depth  > 0);

    const Mode newMode(width, height, refreshRate);
    Modes::const_iterator it = find(modeList_.begin(), modeList_.end(), newMode);

    // Can't use it if it's not in the list.
    if (it == modeList_.end())
    {
        spdlog::info("Unable to find display mode {}x{} ({} Hz)", width, height, refreshRate);
        return Mode();
    }

    return *it;
}

const RenDisplay::Mode RenDisplay::getWindowedMode(int width, int height) const
{
    return Mode(width, height, 0);
}

bool RenDisplay::useLowerMode()
{
    CB_RenDisplay_DEPIMPL();
    PRE(modeList().size() > 0);

    // Locate the current mode in the list.
    Modes::iterator lowestIt = find(modeList_.begin(), modeList_.end(), lowestAllowedMode_);
    Modes::const_iterator it = find(lowestIt, modeList_.end(), currentMode_);
    ASSERT(it != modeList_.end(), logic_error());

    // Search backwards for a lower resolution at the same bit depth.
    while (it != lowestIt)
    {
        --it;
        const Mode& newMode = *it;
        if (newMode.bitDepth() == currentMode_.bitDepth() && newMode.pixels() < currentMode_.pixels())
        {
            return useMode(newMode);
        }
    }

    // No suitable mode was found.
    return false;
}

bool RenDisplay::useNearestMode(int pixels, int bitDepth)
{
    CB_RenDisplay_DEPIMPL();
    PRE(modeList().size() > 0);

    // Traverse the list in order of increasing resolution.
    Modes::const_iterator it = find(modeList_.begin(), modeList_.end(), lowestAllowedMode_);

    const Mode* bestFit = nullptr;
    while (it != modeList_.end())
    {
        const Mode& thisMode = *it;
        if (thisMode.bitDepth() == bitDepth && thisMode.pixels() <= pixels)
            bestFit = &thisMode;
        ++it;
    }

    // Check to see if a suitable mode was found.
    if (bestFit)
        return useMode(*bestFit);
    else
        return false;
}

bool RenDisplay::useLowestMode(int bitDepth)
{
    CB_RenDisplay_DEPIMPL();
    PRE(modeList().size() > 0);

    // Traverse the list in order of increasing resolution.
    Modes::const_iterator it = find(modeList_.begin(), modeList_.end(), lowestAllowedMode_);

    while (it != modeList_.end())
    {
        const Mode& thisMode = *it;
        if (thisMode.bitDepth() == bitDepth)
            return useMode(thisMode);

        ++it;
    }

    return false;
}

const RenDisplay::Mode& RenDisplay::currentMode() const
{
    CB_RenDisplay_DEPIMPL();
    return currentMode_;
}

const RenDisplay::Mode& RenDisplay::lowestAllowedMode() const
{
    CB_RenDisplay_DEPIMPL();
    return lowestAllowedMode_;
}

bool RenDisplay::lowestAllowedMode(int width, int height, int depth)
{
    CB_RenDisplay_DEPIMPL();
    // PRE(directDraw_);
    PRE(width > 0);
    PRE(height > 0);
    PRE(depth > 0);

    const Mode newMode(width, height, 0);
    Modes::const_iterator it = find(modeList_.begin(), modeList_.end(), newMode);

    // Can't use it if it's not in the list.
    if (it == modeList_.end())
        return false;

    return lowestAllowedMode(newMode);
}

bool RenDisplay::lowestAllowedMode(const RenDisplay::Mode& lowest)
{
    CB_RenDisplay_DEPIMPL();

    if (lowestAllowedMode_ == lowest)
        return true;

    bool found = false;
    Modes::const_iterator highestIt = find(modeList_.begin(), modeList_.end(), highestAllowedMode_);
    Modes::const_iterator it = modeList_.begin();
    ASSERT(highestIt != modeList_.end(), "Invalid highestAllowedMode_ ");
    ++highestIt;

    while (it != highestIt && ! found)
    {
        if (lowest == *it)
        {
            lowestAllowedMode_ = *it;
            found = true;
        }
        ++it;
    }

    if (found)
    {
        RENDER_STREAM("Lowest allowed display mode: " << lowestAllowedMode_ << std::endl);
        if (currentMode() < lowest)
        {
            bool setMode = useMode(lowest);
            ASSERT(setMode, "could not switch to lowest allowed mode");
        }
    }
    else
    {
        RENDER_STREAM("Could not find the lowest mode in list " << std::endl);
    }

    return true;
}

const RenDisplay::Mode& RenDisplay::highestAllowedMode() const
{
    CB_RenDisplay_DEPIMPL();
    return highestAllowedMode_;
}

bool RenDisplay::setHighestAllowedMode()
{
    CB_RenDisplay_DEPIMPL();
    // bool result = setHighestAllowedMode( pImpl_->displayVideoMemory() );
    return true;
}

bool RenDisplay::setHighestAllowedMode(uint32_t maxMemory)
{
    CB_RenDisplay_DEPIMPL();

    bool found = false;
    uint32_t memoryRequiredByMode;

    Modes::const_iterator lowestIt = find(modeList_.begin(), modeList_.end(), lowestAllowedMode_);
    Modes::const_iterator it = modeList_.end();

    while (it != lowestIt && ! found)
    {
        --it;
        memoryRequiredByMode = 3 * (*it).memoryRequired();
        if (memoryRequiredByMode <= maxMemory && (*it).bitDepth() > 8)
        {
            highestAllowedMode_ = *it;
            found = true;
        }
    }

    if (found)
    {
        RENDER_STREAM(
            "Memory to fit modes in: " << maxMemory << " (" << (maxMemory / 0x100000) << " Mbytes)" << std::endl);
        RENDER_STREAM(
            "Highest display mode fitting in memory: " << highestAllowedMode_ << " uses " << memoryRequiredByMode
                                                       << " (" << (memoryRequiredByMode / 0x100000) << " Mbytes)"
                                                       << std::endl);
        if (currentMode() > highestAllowedMode())
        {
            bool setMode = useMode(highestAllowedMode());
            ASSERT(setMode, "could not switch to highest allolwed mode");
        }
    }
    else
    {
        RENDER_STREAM(
            "Could not find a display mode fitting in memory: " << maxMemory << " (" << (maxMemory / 0x100000)
                                                                << " Mbytes)" << std::endl);
    }
    RENDER_STREAM(std::endl);

    return found;
}

bool RenDisplay::createSurfaces(MemoryType memType, int zbDepth)
{
    CB_RenDisplay_DEPIMPL();

    // Try to create a set of surfaces for the current mode.  If this fails,
    // due to lack of video memory, repeatedly choose a lower resolution.
    if (! fallibleCreateSurfaces(memType, zbDepth))
        return false;

    RENDER_STREAM("Created surfaces in mode " << currentMode_ << ":\n");
    // RENDER_STREAM("  front  COM=" << (void*) primaryBuf_ << "\n");
    // RENDER_STREAM("  back   COM=" << (void*) backBuf_ << "\n");
    // RENDER_STREAM("  front2 COM=" << (void*) primaryBuf2_ << "\n");
    // RENDER_STREAM("  back2  COM=" << (void*) backBuf2_ << "\n");
    // RENDER_STREAM("  Z      COM=" << (void*) zBuf_ << "\n\n");

    return true;
}

bool RenDisplay::fallibleCreateSurfaces(MemoryType memType, int zbDepth)
{
    CB_RenDisplay_DEPIMPL();

    static bool first = true;
    switch (memType)
    {
        default:
            ASSERT(0, logic_error("Unknown memory type."));
    }

    // If in fullscreen mode, create complex flipping primary surface.
    if (fullscreen_)
    {
        RENDER_STREAM("About to create front and back buffers.\n");
    }
    else
    {
    }

    if (zbDepth > 0)
    {
    }

    return true;
}

Ren::IWindowAdapter* RenDisplay::adapter()
{
    return pImpl_->adapter_;
}

const Ren::IWindowAdapter* RenDisplay::adapter() const
{
    return pImpl_->adapter_;
}

RenIDisplay& RenDisplay::displayImpl()
{
    CB_RenDisplay_DEPIMPL();
    PRE(pImpl_ != nullptr)

    return *pImpl_;
}

const RenIDisplay& RenDisplay::displayImpl() const
{
    CB_RenDisplay_DEPIMPL();
    PRE(pImpl_ != nullptr)

    return *pImpl_;
}

bool RenDisplay::checkForLostSurfaces()
{
    CB_RenDisplay_DEPIMPL();
    // Restore the primary surface if it has been lost.
    // This also restores any implicitly created attached surfaces,
    // i.e. the back buffer.
    return true;
}

void RenDisplay::startFrame()
{
    DevTimer cursorTimer;
    pImpl_->restoreUnderCursor();
    RENDER_STREAM("Cursor restore took " << 1000 * cursorTimer.time() << "(ms)\n");
}

void RenDisplay::flipBuffers()
{
    CB_RenDisplay_DEPIMPL();

    DevTimer flipTimer;

    if (pImpl_->adapter_)
        pImpl_->adapter_->swapBuffers();

    RENDER_STREAM("Buffer flip took " << 1000 * flipTimer.time() << "(ms)\n");

    ++frameNo_;
}

void RenDisplay::useCursor(const RenCursor2d* c)
{
    pImpl_->useCursor(c);
}

const RenCursor2d* RenDisplay::currentCursor() const
{
    return pImpl_->currentCursor();
}

void RenDisplay::setCursorGrabEnabled(bool enabled)
{
    if (pImpl_->adapter_)
        pImpl_->adapter_->setCursorGrabEnabled(enabled);
}

uint32_t RenDisplay::frameNumber() const
{
    CB_RenDisplay_DEPIMPL();
    return frameNo_;
}

bool RenDisplay::isPrimaryDriver() const
{
    return pImpl_->isPrimaryDriver_;
}

bool RenDisplay::supportsGammaCorrection() const
{
    return pImpl_->supportsGammaCorrection_;
}

void RenDisplay::supportsGammaCorrection(bool doSupport)
{
    pImpl_->supportsGammaCorrection_ = doSupport;
}

void RenDisplay::gammaCorrection(const double& gamma)
{
    CB_RenDisplay_DEPIMPL();
    PRE(supportsGammaCorrection());
    PRE(gamma > 0);

    if (gammaCorrection_ == gamma)
        return;

    // TODO: Per-window gamma ramps are not supported by modern display servers,
    // so the windowing layer no longer exposes them. Gamma should be
    // reimplemented in the renderer, e.g. as an output shader pass. Until then
    // the capability is reported as unsupported, so this is unreachable.

    gammaCorrection_ = gamma;
}

const double& RenDisplay::gammaCorrection() const
{
    CB_RenDisplay_DEPIMPL();
    PRE(supportsGammaCorrection());
    return gammaCorrection_;
}

bool operator<(const RenDisplay::Mode& mode1, const RenDisplay::Mode& mode2)
{
    // Sort of number of pixels first, then on depth.
    if (mode1.pixels() < mode2.pixels())
        return true;
    else if (mode1.pixels() > mode2.pixels())
        return false;
    else
        return false;
}

bool operator>(const RenDisplay::Mode& mode1, const RenDisplay::Mode& mode2)
{
    return mode2 < mode1;
}

bool operator>=(const RenDisplay::Mode& mode1, const RenDisplay::Mode& mode2)
{
    // TODO: compare depth?
    return mode1.pixels() >= mode2.pixels();
}

void RenDisplay::CLASS_INVARIANT
{
    CB_RenDisplay_DEPIMPL();
    INVARIANT(pImpl_);
    // If we've gone into fullscreen mode, there must be at least one
    // screen mode available.
    INVARIANT(implies(fullscreen_, modeList_.size() > 0));

    // If there are any entries in the list of modes, then we ought to
    // have picked one as the current mode.
    INVARIANT(implies(modeList_.size() > 0, currentMode_.width() > 0));
}

std::ostream& operator<<(std::ostream& o, const RenDisplay::Mode& m)
{
    o << "display mode (" << m.width() << "x" << m.height() << "x" << m.bitDepth() << ")";
    return o;
}

std::ostream& operator<<(std::ostream& o, const RenDisplay& t)
{
    o << "Modes available for display:\n";

    int i = 0;
    const auto& modes = t.modeList();
    for (const RenDisplay::Mode& mode : modes)
    {
        o << i << "\t" << mode << "\n";
        i++;
    }

    return o;
}

/* End DISPLAY.CPP **************************************************/
