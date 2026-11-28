#include "machgui/DisplayPreferences.hpp"

namespace MachGui
{

DisplayPreferences applyDisplayChoices(
    DisplayPreferences saved,
    DisplayPreferences shown,
    bool resolutionChosen,
    bool refreshRateChosen)
{
    saved.windowMode = shown.windowMode;
    if (shown.windowMode == Ren::WindowMode::Borderless)
        return saved;

    if (resolutionChosen)
        saved.resolution = shown.resolution;
    if (resolutionChosen || refreshRateChosen)
        saved.refreshRate = shown.refreshRate;
    return saved;
}

} // namespace MachGui
