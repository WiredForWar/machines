#pragma once

#include <algorithm>
#include <cmath>

namespace MachGui
{
// Keep controls and animation patches on the same 4:3 canvas. The surrounding
// board covers the remaining monitor area without distorting that canvas.
inline double fittedMenuScale(int width, int height)
{
    return std::max(1.0, std::min(width / 640.0, height / 480.0));
}

struct MenuSurroundArea
{
    int x, y, width, height;
};

inline MenuSurroundArea menuSurroundArea(int width, int height, int imageWidth, int imageHeight)
{
    const double scale = std::max(width / double(imageWidth), height / double(imageHeight));
    const int w = int(std::ceil(imageWidth * scale));
    const int h = int(std::ceil(imageHeight * scale));
    return {(width - w) / 2, (height - h) / 2, w, h};
}
}
