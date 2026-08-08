#pragma once

#include <string_view>

namespace Ren
{

// How a window covers the display.
enum class WindowMode
{
    // The window owns the display and puts it into the mode asked for, so the
    // resolution and the refresh rate are both the caller's to choose.
    Fullscreen,

    // The window covers the display at the resolution the display is already in.
    // Neither the resolution nor the refresh rate is the caller's to pick.
    Borderless,

    // The window is one of several on the display. Its size is the caller's to
    // choose; the refresh rate is not.
    Windowed,
};

inline constexpr WindowMode AllWindowModes[] = {
    WindowMode::Fullscreen,
    WindowMode::Borderless,
    WindowMode::Windowed,
};

std::string_view toString(WindowMode mode);

} // namespace Ren
