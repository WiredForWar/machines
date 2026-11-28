#pragma once
#include <algorithm>
#include <cmath>

namespace Ren
{
struct MenuBoardPlacement { int left, top, width, height; };
struct MenuBoardSlice { int sourceX, sourceWidth, destX, destWidth; };
inline MenuBoardSlice clippedMenuSlice(int sx, int sw, int dx, int dw, int viewportWidth)
{
    const int left = std::max(0, dx), right = std::min(viewportWidth, dx + dw);
    if (dw <= 0 || right <= left) return {};
    int sourceLeft = sx + int(std::round(double(left - dx) * sw / dw));
    int sourceRight = sx + int(std::round(double(right - dx) * sw / dw));
    sourceLeft = std::min(sourceLeft, sx + sw - 1);
    sourceRight = std::max(sourceLeft + 1, std::min(sourceRight, sx + sw));
    return {sourceLeft, sourceRight-sourceLeft, left, right-left};
}
inline MenuBoardPlacement menuBoardPlacement(int width, int height)
{
    const double scale = std::min(width / 640.0, height / 480.0);
    const int w = int(640 * scale), h = int(480 * scale);
    return {(width - w) / 2, (height - h) / 2, w, h};
}
inline int menuTransitionOffset(unsigned long frame, unsigned long count, int width)
{
    const double t = count > 1 ? std::clamp(double(frame) / (count - 1), 0.0, 1.0) : 1.0;
    return int(std::round(t * t * (3.0 - 2.0 * t) * width));
}
}
