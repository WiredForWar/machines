#pragma once

#include "render/IWindowAdapter.hpp"
#include "render/render.hpp"

#include <vector>

namespace Ren
{

// The resolutions a display offers and the refresh rates it offers each of them
// at, with the repeats a raw list of display modes carries collapsed into one
// entry apiece.
//
// Both lists run best first: the largest resolution and the highest refresh rate
// come first, so the head of each is the one to offer by default.
class DisplayModeCatalogue
{
public:
    DisplayModeCatalogue() = default;
    explicit DisplayModeCatalogue(const std::vector<IWindowAdapter::DisplayMode>& modes);

    bool isEmpty() const;

    const std::vector<Size>& resolutions() const;
    bool offers(Size resolution) const;

    // Empty for a resolution that is not offered.
    const std::vector<int>& refreshRates(Size resolution) const;

    // The highest rate the resolution is offered at, or zero if it is not offered.
    int bestRefreshRate(Size resolution) const;

private:
    // Held apart so that the resolutions can be handed out on their own. The two
    // run in step: rate list n belongs to resolution n.
    std::vector<Size> resolutions_;
    std::vector<std::vector<int>> refreshRates_;
};

} // namespace Ren
