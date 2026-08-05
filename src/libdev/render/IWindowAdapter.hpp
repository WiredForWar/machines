#pragma once

#include "render/IRenderSurface.hpp"

#include <cstdint>
#include <vector>

namespace Ren
{

// A render surface backed by a real window, which can therefore also enumerate
// and apply display modes. Only RenDisplay and the application need this much;
// render backends take the narrower IRenderSurface, so that a host without a
// window of its own does not have to stub these out.
class IWindowAdapter : public virtual IRenderSurface
{
public:
    // --- Display modes ---

    struct DisplayMode
    {
        int width{};
        int height{};
        int depth{};
        int refreshRate{};
        uint32_t format{};
    };

    virtual std::vector<DisplayMode> availableDisplayModes() const = 0;
    virtual DisplayMode desktopDisplayMode() const = 0;
    virtual bool useMode(const DisplayMode& mode) = 0;

    virtual bool setFullscreen(bool enabled) = 0;
    virtual bool isFullscreen() const = 0;

    // --- Cursor / input ---

    virtual void setCursorGrabEnabled(bool enabled) = 0;

    // Switch the pointer to relative reporting, hiding it and freeing it from the window
    // edges so travel keeps arriving once the pointer would have left. Returns false, and
    // leaves the mode unchanged, if the window cannot honour the request.
    virtual bool setRelativeMouseModeEnabled(bool enabled) = 0;

    virtual bool hasMouseFocus() const = 0;
};

} // namespace Ren
