#include "window_support/sdl/SDLWindowAdapter.hpp"

#include <SDL3/SDL.h>

#include "spdlog/spdlog.h"

namespace Ren
{

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
                    .refreshRate = static_cast<int>(mode->refresh_rate),
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
        .refreshRate = static_cast<int>(mode->refresh_rate),
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
        // SDL3 requires a mode obtained from SDL; pick the closest supported one.
        SDL_DisplayMode closestMode;
        success = SDL_GetClosestFullscreenDisplayMode(
            SDL_GetDisplayForWindow(window_),
            mode.width,
            mode.height,
            static_cast<float>(mode.refreshRate),
            false,
            &closestMode);
        if (success)
            success = SDL_SetWindowFullscreenMode(window_, &closestMode);
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

Ren::IWindowAdapter::WindowMode SDLWindowAdapter::windowMode() const
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
