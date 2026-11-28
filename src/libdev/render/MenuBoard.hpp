#pragma once
#include "render/Painter.hpp"
#include "render/Surface.hpp"
#include "render/MenuPresentation.hpp"
#include <algorithm>

namespace Ren
{
// All regions sample ONE continuous board. At 16:9 they share the same uniform
// scale. Wider monitors extend its edge circuitry while preserving menu panels.
inline void drawMenuBoard(Painter& painter, const RenSurface& image, int width, int height, int offsetX = 0)
{
    const auto placement = menuBoardPlacement(width, height);
    const int centerWidth = placement.width, centerHeight = placement.height;
    const int left = placement.left, top = placement.top;
    const int edge = int(image.width()) / 8;
    const int middle = int(image.width()) - edge * 2;
    auto slice = [&](int sourceX, int sourceWidth, int destX, int destWidth) {
        const auto clip = clippedMenuSlice(sourceX, sourceWidth, destX + offsetX, destWidth, width);
        if (clip.destWidth > 0)
            painter.stretchBlit(image, Rect(clip.sourceX, 0, clip.sourceWidth, image.height()),
                Rect(clip.destX, top, clip.destWidth, centerHeight), BlitMode::Replace);
    };
    if (left > 0)
        slice(0, edge, 0, left);
    slice(edge, middle, left, centerWidth);
    const int right = left + centerWidth;
    if (right < width)
        slice(edge + middle, edge, right, width - right);
}
}
