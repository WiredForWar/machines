#include "machgui/ScaleFactor.hpp"

#include <cstdint>

namespace MachGui
{

bool scaleFactorFits(int scaleFactorPercents, Ren::Size windowSize)
{
    if (scaleFactorPercents == 0)
        return true;

    return scaleFactorPercents > 0
        && int64_t(MenuSize.width) * scaleFactorPercents <= int64_t(windowSize.width) * 100
        && int64_t(MenuSize.height) * scaleFactorPercents <= int64_t(windowSize.height) * 100;
}

int resolveScaleFactor(int preferredPercents, Ren::Size windowSize)
{
    int automatic = 100;
    for (const int percents : AllScaleFactorValues)
    {
        if (percents == 0 || !scaleFactorFits(percents, windowSize))
            continue;

        if (percents == preferredPercents)
            return percents;

        automatic = percents;
    }
    return automatic;
}

} // namespace MachGui
