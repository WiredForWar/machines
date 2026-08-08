#pragma once

#include "render/IWindowAdapter.hpp"

struct SDL_Window;

namespace Ren
{

// Window adapter for SDL. Used by the game and PlanetEd.
// Implements IGLRenderSurface (GL context lifecycle) plus the windowing and
// display mode methods of IWindowAdapter. Both derive IRenderSurface virtually,
// so this class has a single copy of it.
class SDLWindowAdapter final
    : public virtual IWindowAdapter
    , public virtual IGLRenderSurface
{
public:
    // The adapter does NOT own the SDL_Window; the caller manages its lifetime.
    explicit SDLWindowAdapter(SDL_Window* window);
    ~SDLWindowAdapter() override;

    // --- IGLRenderSurface ---
    bool createGLContext(const GLContextParams& params) override;
    void destroyGLContext() override;

    // --- Presentation ---
    void swapBuffers() override;
    bool setVSyncMode(VSyncMode mode) override;

    // --- Surface geometry ---
    int width() const override;
    int height() const override;

    // --- Display modes ---
    std::vector<DisplayMode> availableDisplayModes() const override;
    DisplayMode desktopDisplayMode() const override;
    bool useMode(const DisplayMode& mode) override;

    // --- Window mode ---
    bool setWindowMode(WindowMode mode) override;
    WindowMode windowMode() const override;

    // --- Cursor / input ---
    void setCursorGrabEnabled(bool enabled) override;
    bool setRelativeMouseModeEnabled(bool enabled) override;
    bool hasMouseFocus() const override;

private:
    SDL_Window* window_{};
    // SDL_GLContext, kept opaque so that this header does not pull in SDL.
    void* glContext_{};
    WindowMode windowMode_{WindowMode::Windowed};
};

} // namespace Ren
