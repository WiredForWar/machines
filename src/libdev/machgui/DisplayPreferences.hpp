#pragma once

#include "render/render.hpp"
#include "render/WindowMode.hpp"

namespace MachGui
{

struct DisplayPreferences
{
    Ren::WindowMode windowMode{Ren::WindowMode::Fullscreen};
    Ren::Size resolution{}; // Zero means follow the desktop.
    int refreshRate{};
};

// A displayed fallback is not a new preference. Rebuilding a list, or accepting
// unrelated options, must not pin an automatic size or replace an unavailable
// saved mode. Borderless takes its size and rate from the desktop.
DisplayPreferences applyDisplayChoices(
    DisplayPreferences saved,
    DisplayPreferences shown,
    bool resolutionChosen,
    bool refreshRateChosen);

} // namespace MachGui
