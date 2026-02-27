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
    if (fullscreen_)
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
        fullscreen_ = SDL_SetWindowFullscreen(window_, true);
    }
    else
    {
        SDL_SetWindowFullscreen(window_, false);
        SDL_SetWindowSize(window_, mode.width, mode.height);
        SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
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

bool SDLWindowAdapter::setFullscreen(bool enabled)
{
    if (!window_)
        return false;

    if (enabled)
    {
        // Deferred: the mode to go fullscreen with is only known in useMode().
        fullscreen_ = true;
    }
    else
    {
        fullscreen_ = !SDL_SetWindowFullscreen(window_, false);
    }

    return fullscreen_ == enabled;
}

bool SDLWindowAdapter::isFullscreen() const
{
    return fullscreen_;
}

void SDLWindowAdapter::setCursorGrabEnabled(bool enabled)
{
    if (window_)
        SDL_SetWindowMouseGrab(window_, enabled);
}

bool SDLWindowAdapter::hasMouseFocus() const
{
    return SDL_GetMouseFocus() != nullptr;
}

} // namespace Ren
