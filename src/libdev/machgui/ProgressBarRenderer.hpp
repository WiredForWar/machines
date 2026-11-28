#pragma once

#include "render/Painter.hpp"
#include "render/Colour.hpp"
#include <algorithm>
#include <cmath>

namespace MachGui
{
inline void drawProgressBarFrame(Ren::Painter& painter, const Ren::Rect& fill, double scale,
    const RenColour& colour)
{
    const int pad = std::max(1, int(std::lround(2 * scale)));
    const Ren::Rect frame(fill.originX - pad, fill.originY - pad,
        fill.width + 2 * pad, fill.height + 2 * pad);
    // Replace only the bitmap's bar outline. Text and backdrop are untouched.
    painter.filledRectangle(frame, RenColour::black());
    painter.hollowRectangle(frame, colour, std::max(1, int(std::lround(scale * 0.35))));
}

inline void drawProgressBarFill(Ren::Painter& painter, const Ren::Rect& area, int width,
    const RenColour& colour)
{
    painter.filledRectangle(area, RenColour::black());
    width = std::clamp(width, 0, area.width);
    if (width == 0)
        return;
    // A continuous bevel shaded per display pixel, rather than enlarged pixels
    // from the low-resolution source. Retain the original ivory/gold palette.
    for (int row = 0; row < area.height; ++row)
    {
        const float shade = 1.0f - 0.22f * row / std::max(1, area.height - 1);
        painter.filledRectangle(Ren::Rect(area.originX, area.originY + row, width, 1),
            RenColour(colour.r() * shade, colour.g() * shade, colour.b() * shade));
    }
}
}
