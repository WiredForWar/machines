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
#include "render/render.hpp"
#include "render/DriverSelector.hpp"
#include "render/Surface.hpp"
#include "render/internal/DisplayImpl.hpp"

#include <SDL3/SDL.h>

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
RenDisplay::RenDisplay(SDL_Window* wnd)
    : pImpl_(new RenIDisplay(wnd))
{
    CB_RenDisplay_DEPIMPL();
    PRE(Ren::initialised());

    // Initially, set up a primary DD driver.  If useFullScreen is subsequently
    // called, then this driver will potentially be replaced by one which is not
    // the primary driver.
    isPrimaryDriver_ = true;

    // Initially start in normal screen mode.  (I think it may be necessary
    // to call this to specify the window handle, even if we are running in
    // a window, rather than in fullscreen mode.)

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

    if (const SDL_DisplayMode* desktopMode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay()))
    {
        spdlog::info(
            "Current desktop display mode: {}x{}@{}bpp ({} Hz)",
            desktopMode->w,
            desktopMode->h,
            SDL_BITSPERPIXEL(desktopMode->format),
            desktopMode->refresh_rate);
    }
    else
    {
        spdlog::warn("Unable to get the current desktop display mode");
    }

    int displayCount = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&displayCount);
    modeList_.clear();
    modeList_.reserve(displayCount);

    RENDER_STREAM("Number of displays: " << displayCount << std::endl);
    for (int displayIndex = 0; displays && displayIndex < displayCount; displayIndex++)
    {
        int modesCount = 0;
        SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(displays[displayIndex], &modesCount);
        for (int modeIndex = 0; modes && modeIndex < modesCount; modeIndex++)
        {
            const SDL_DisplayMode* mode = modes[modeIndex];

            // Add modes with matching refresh rate only
            // if(desktopMode.w && (desktopMode.refresh_rate == mode.refresh_rate) )
            {
                RenDisplay::Mode m(
                    mode->w,
                    mode->h,
                    SDL_BITSPERPIXEL(mode->format),
                    static_cast<int>(mode->refresh_rate),
                    mode->format);
                modeList_.push_back(m);
            }
        }
        SDL_free(modes);
    }
    SDL_free(displays);

    std::sort(modeList_.begin(), modeList_.end());

    lowestAllowedMode_ = modeList_.front();
    highestAllowedMode_ = modeList_.back();
}

const RenDisplay::Mode RenDisplay::getDesktopDisplayMode() const
{
    const SDL_DisplayMode* desktopMode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
    if (!desktopMode)
    {
        // Got no current desktop mode
        return Mode();
    }
    return Mode(
        desktopMode->w,
        desktopMode->h,
        SDL_BITSPERPIXEL(desktopMode->format),
        static_cast<int>(desktopMode->refresh_rate),
        desktopMode->format);
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
    fullscreen_ = !SDL_SetWindowFullscreen(window(), false);

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

    bool success = true;
    if (fullscreen_)
    {
        // SDL3 requires a mode obtained from SDL; pick the closest supported one.
        SDL_DisplayMode closestMode;
        success = SDL_GetClosestFullscreenDisplayMode(
            SDL_GetDisplayForWindow(window()),
            m.width(),
            m.height(),
            m.refreshRate(),
            false,
            &closestMode);
        if (success)
            success = SDL_SetWindowFullscreenMode(window(), &closestMode);
        // SDL_SetWindowBordered(window(), false);
        SDL_SetWindowSize(window(), m.width(), m.height());
        SDL_SetWindowPosition(window(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        fullscreen_ = SDL_SetWindowFullscreen(window(), true);
    }
    else
    {
        SDL_SetWindowFullscreen(window(), false);
        SDL_SetWindowSize(window(), m.width(), m.height());
        SDL_SetWindowPosition(window(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
    SDL_ShowWindow(window());
    // The size and the fullscreen state are applied asynchronously by the
    // windowing system, so wait for them to land before setting up the
    // viewport. This has to happen after the window is shown: Wayland cannot
    // map a toplevel directly in the fullscreen state, so the state requested
    // while the window was hidden is only sent to the compositor once the
    // window has been mapped, and until it is acknowledged the window is in
    // its windowed state.
    SDL_SyncWindow(window());

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

SDL_Window* RenDisplay::window()
{
    CB_RenDisplay_DEPIMPL();
    return pImpl_->pWnd_;
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

    SDL_GL_SwapWindow(window());

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
    SDL_SetWindowMouseGrab(pImpl_->pWnd_, enabled);
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

using GammaRamp = struct
{
    Uint16 red[256], green[256], blue[256];
};
static void setGammaRamp(double gamma, GammaRamp* pGammaRamp)
{
    PRE(gamma > 0);
    PRE(pGammaRamp);

    GammaRamp& gammaRamp = *pGammaRamp;
    const double correction = 1 / gamma;

    for (int i = 0; i != 256; ++i)
    {
        const double oldR = i / 255.0;
        const double oldG = i / 255.0;
        const double oldB = i / 255.0;

        const double newR = pow(oldR, correction);
        const double newG = pow(oldG, correction);
        const double newB = pow(oldB, correction);

        // This shouldn't be necessary for RGB's in the range [0,1].
        const double clampR = (newR > 1.0) ? 1.0 : newR;
        const double clampG = (newG > 1.0) ? 1.0 : newG;
        const double clampB = (newB > 1.0) ? 1.0 : newB;

        gammaRamp.red[i] = (65535 * clampR + 0.5);
        gammaRamp.green[i] = (65535 * clampG + 0.5);
        gammaRamp.blue[i] = (65535 * clampB + 0.5);
    }
}

void RenDisplay::gammaCorrection(const double& gamma)
{
    CB_RenDisplay_DEPIMPL();
    PRE(supportsGammaCorrection());
    PRE(gamma > 0);

    if (gammaCorrection_ == gamma)
        return;

    // create a gamma ramp
    GammaRamp gammaRamp;
    setGammaRamp(gamma, &gammaRamp);

    // TODO: SDL3 removed SDL_SetWindowGammaRamp() (modern display servers do
    // not support per-window gamma ramps). Gamma should be reimplemented in
    // the renderer, e.g. as an output shader pass. Until then the capability
    // is reported as unsupported, so this is unreachable in practice.
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
