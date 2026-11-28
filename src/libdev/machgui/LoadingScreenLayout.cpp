#include "machgui/LoadingScreenLayout.hpp"

#include "machgui/ScaleFactor.hpp"

#include <algorithm>
#include <cmath>

namespace MachGui
{
Ren::Rect loadGameProgressArea(Ren::Point menuOrigin, double menuScale)
{
    return Ren::Rect(menuOrigin + Ren::Point{int(std::lround(275 * menuScale)), int(std::lround(250 * menuScale))},
        Ren::Size{int(std::lround(87 * menuScale)), int(std::lround(5 * menuScale))});
}

LoadingScreenLayout loadingScreenLayout(Ren::Size windowSize, double scale)
{
    const Ren::Size backdropSize = MenuSize * scale;
    const Ren::Point origin{
        std::max(0, windowSize.width - backdropSize.width) / 2,
        std::max(0, windowSize.height - backdropSize.height) / 2};

    return {
        Ren::Rect(origin, backdropSize),
        Ren::Point{32, 129} * scale,
        origin + Ren::Point{32, 115} * scale,
        Ren::Rect(origin + Ren::Point{98, 362} * scale, Ren::Size{441, 5} * scale)};
}

} // namespace MachGui
