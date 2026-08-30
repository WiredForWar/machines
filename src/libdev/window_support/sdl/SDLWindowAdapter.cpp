#include "window_support/sdl/SDLWindowAdapter.hpp"

#include <SDL3/SDL.h>

#include "spdlog/spdlog.h"

#include <cmath>

namespace Ren
{

namespace
{

// The rate to report a mode at, and the rate the player picks it by. A mode timed
// at 59.95 Hz is a 60 Hz mode: the fraction is a timing detail of the mode rather
// than something the player is choosing, and truncating it would offer them a
// 59 Hz and a 60 Hz entry for two modes they cannot tell apart.
int roundedRefreshRate(float exactRate)
{
    return static_cast<int>(std::lround(exactRate));
}

// Which of two modes offered at the same rate to run at. The rate the desktop is
// already in wins outright, so that going fullscreen at the rate the desktop runs
// at leaves the display alone rather than retiming it for a difference nobody can
// see. Failing that the rate nearest the one asked for wins, and where no rate was
// asked for the highest one does.
bool isBetterRefreshRate(float candidate, float best, int wantedRate, float desktopRate)
{
    if (best == desktopRate)
        return false;
    if (candidate == desktopRate)
        return true;

    if (wantedRate <= 0)
        return candidate > best;

    const float wanted = static_cast<float>(wantedRate);

    return std::fabs(candidate - wanted) < std::fabs(best - wanted);
}

// The mode to run at a resolution and a rate the player was offered. Several of
// the modes a display offers at one resolution can round to the same rate, so the
// rate alone does not name one of them and this picks between them.
//
// SDL_GetClosestFullscreenDisplayMode cannot do the job: its tie break between two
// modes of the same size only lets the second win if it has the greater colour
// depth, so on a display whose modes all share one pixel format it hands back the
// highest rate on offer whatever rate it was asked for.
bool chooseDisplayMode(SDL_DisplayID display, const IWindowAdapter::DisplayMode& wanted, SDL_DisplayMode* result)
{
    int modeCount = 0;
    SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(display, &modeCount);
    if (!modes)
        return false;

    float desktopRate = 0.0F;
    if (const SDL_DisplayMode* desktopMode = SDL_GetDesktopDisplayMode(display))
        desktopRate = desktopMode->refresh_rate;

    const SDL_DisplayMode* best = nullptr;
    for (int i = 0; i < modeCount; ++i)
    {
        const SDL_DisplayMode* candidate = modes[i];

        if (candidate->w != wanted.width || candidate->h != wanted.height)
            continue;

        // A rate of zero is the caller having none to ask for, as the failsafe mode
        // does, rather than a rate no display offers.
        if (wanted.refreshRate > 0 && roundedRefreshRate(candidate->refresh_rate) != wanted.refreshRate)
            continue;

        if (!best || isBetterRefreshRate(candidate->refresh_rate, best->refresh_rate, wanted.refreshRate, desktopRate))
            best = candidate;
    }

    // The modes themselves belong to SDL and outlive the list, but copy while the
    // list is still around anyway: what SDL is handed back has to be a mode it
    // handed out, byte for byte, or it will not recognise it.
    if (best)
        *result = *best;

    SDL_free(modes);

    return best != nullptr;
}

} // namespace

SDLWindowAdapter::SDLWindowAdapter(SDL_Window* window)
    : window_(window)
{
}

SDLWindowAdapter::~SDLWindowAdapter()
{
    destroyGLContext();
}

bool SDLWindowAdapter::createGLContext(const GLContextParams& params)
{
    if (glContext_)
        return false;

    if (!window_)
        return false;

    spdlog::info(
        "SDLWindowAdapter: requesting GL {}.{} (core: {})",
        params.majorVersion,
        params.minorVersion,
        params.coreProfile);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, params.majorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, params.minorVersion);
    if (params.coreProfile)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_)
    {
        spdlog::error("SDLWindowAdapter: SDL_GL_CreateContext failed: {}", SDL_GetError());
        return false;
    }

    int depthBits{};
    int stencilBits{};
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depthBits);
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencilBits);
    spdlog::info("SDLWindowAdapter: the window's own buffer has {} depth bits and {} stencil", depthBits, stencilBits);

    return true;
}

void SDLWindowAdapter::destroyGLContext()
{
    if (glContext_)
    {
        SDL_GL_MakeCurrent(nullptr, nullptr);
        SDL_GL_DestroyContext(static_cast<SDL_GLContext>(glContext_));
        glContext_ = nullptr;
    }
}

void SDLWindowAdapter::swapBuffers()
{
    if (window_)
        SDL_GL_SwapWindow(window_);
}

bool SDLWindowAdapter::setVSyncMode(VSyncMode mode)
{
    int interval{};
    switch (mode)
    {
    case VSyncMode::Off:
        interval = 0;
        break;
    case VSyncMode::On:
        interval = 1;
        break;
    case VSyncMode::Adaptive:
        interval = -1;
        break;
    }

    if (SDL_GL_SetSwapInterval(interval))
        return true;

    spdlog::warn("SDL_GL_SetSwapInterval({}) failed: {}", interval, SDL_GetError());
    return false;
}

int SDLWindowAdapter::width() const
{
    int w{};
    int h{};
    if (window_)
        SDL_GetWindowSize(window_, &w, &h);
    return w;
}

int SDLWindowAdapter::height() const
{
    int w{};
    int h{};
    if (window_)
        SDL_GetWindowSize(window_, &w, &h);
    return h;
}

std::vector<IWindowAdapter::DisplayMode> SDLWindowAdapter::availableDisplayModes() const
{
    std::vector<DisplayMode> result;

    int displayCount = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&displayCount);

    for (int displayIndex = 0; displays && displayIndex < displayCount; ++displayIndex)
    {
        int modesCount = 0;
        SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(displays[displayIndex], &modesCount);
        for (int modeIndex = 0; modes && modeIndex < modesCount; ++modeIndex)
        {
            const SDL_DisplayMode* mode = modes[modeIndex];
            result.push_back(
                DisplayMode{
                    .width = mode->w,
                    .height = mode->h,
                    .depth = static_cast<int>(SDL_BITSPERPIXEL(mode->format)),
                    .refreshRate = roundedRefreshRate(mode->refresh_rate),
                    .format = static_cast<uint32_t>(mode->format),
                });
        }
        SDL_free(modes);
    }
    SDL_free(displays);

    return result;
}

IWindowAdapter::DisplayMode SDLWindowAdapter::desktopDisplayMode() const
{
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
    if (!mode)
        return {};

    return DisplayMode{
        .width = mode->w,
        .height = mode->h,
        .depth = static_cast<int>(SDL_BITSPERPIXEL(mode->format)),
        .refreshRate = roundedRefreshRate(mode->refresh_rate),
        .format = static_cast<uint32_t>(mode->format),
    };
}

bool SDLWindowAdapter::useMode(const DisplayMode& mode)
{
    if (!window_)
        return false;

    bool success = true;
    switch (windowMode_)
    {
    case WindowMode::Fullscreen:
    {
        // SDL3 only takes a mode it handed out itself, so pick one of those.
        SDL_DisplayMode exclusiveMode;
        if (!chooseDisplayMode(SDL_GetDisplayForWindow(window_), mode, &exclusiveMode))
        {
            // The display offers nothing at the size asked for, which is what a
            // resolution recorded against a display that has since been swapped
            // looks like. Borderless brings its own size along so it is the one
            // fullscreen state always available, and taking it says so, where
            // settling on some other size would hand back a resolution the player
            // never chose without ever mentioning it.
            spdlog::warn(
                "SDLWindowAdapter: no fullscreen mode is {}x{}; going borderless instead",
                mode.width,
                mode.height);

            success = SDL_SetWindowFullscreenMode(window_, nullptr);
            if (success)
                success = SDL_SetWindowFullscreen(window_, true);
            windowMode_ = success ? WindowMode::Borderless : WindowMode::Windowed;
            break;
        }

        // The rate is logged as SDL states it, since it is the mode's real timing
        // that the rate the player picked has been rounded away from.
        spdlog::info(
            "SDLWindowAdapter: taking the {}x{} mode timed at {:.3f} Hz for {} Hz",
            exclusiveMode.w,
            exclusiveMode.h,
            exclusiveMode.refresh_rate,
            mode.refreshRate);

        success = SDL_SetWindowFullscreenMode(window_, &exclusiveMode);
        SDL_SetWindowSize(window_, mode.width, mode.height);
        SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        if (!SDL_SetWindowFullscreen(window_, true))
            windowMode_ = WindowMode::Windowed;
        break;
    }

    case WindowMode::Borderless:
        // A null fullscreen mode leaves the display in the mode it is already in,
        // which is what makes this borderless rather than exclusive.
        success = SDL_SetWindowFullscreenMode(window_, nullptr);
        if (success)
            success = SDL_SetWindowFullscreen(window_, true);
        if (!success)
            windowMode_ = WindowMode::Windowed;
        break;

    case WindowMode::Windowed:
        SDL_SetWindowFullscreen(window_, false);
        SDL_SetWindowSize(window_, mode.width, mode.height);
        SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        break;
    }

    SDL_ShowWindow(window_);
    // The size and the fullscreen state are applied asynchronously by the
    // windowing system, so wait for them to land before the caller sets up the
    // viewport. This has to happen after the window is shown: Wayland cannot
    // map a toplevel directly in the fullscreen state, so the state requested
    // while the window was hidden is only sent to the compositor once the
    // window has been mapped, and until it is acknowledged the window is in
    // its windowed state.
    SDL_SyncWindow(window_);

    return success;
}

bool SDLWindowAdapter::setWindowMode(WindowMode mode)
{
    if (!window_)
        return false;

    if (mode == WindowMode::Windowed && windowMode_ != WindowMode::Windowed)
    {
        if (SDL_SetWindowFullscreen(window_, false))
            windowMode_ = WindowMode::Windowed;
    }
    else
    {
        // Covering the display is deferred to useMode(), which is where the mode to
        // cover it with becomes known.
        windowMode_ = mode;
    }

    return windowMode_ == mode;
}

Ren::WindowMode SDLWindowAdapter::windowMode() const
{
    return windowMode_;
}

void SDLWindowAdapter::setCursorGrabEnabled(bool enabled)
{
    if (window_)
        SDL_SetWindowMouseGrab(window_, enabled);
}

bool SDLWindowAdapter::setRelativeMouseModeEnabled(bool enabled)
{
    if (window_ && SDL_SetWindowRelativeMouseMode(window_, enabled))
        return true;

    spdlog::warn("Unable to {} relative mouse mode: {}", enabled ? "enable" : "disable", SDL_GetError());
    return false;
}

bool SDLWindowAdapter::hasMouseFocus() const
{
    return SDL_GetMouseFocus() != nullptr;
}

} // namespace Ren
