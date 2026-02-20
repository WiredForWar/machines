/*
 * P A I N T E R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#pragma once

#include "render/render.hpp"

#include <string_view>

class RenColour;
class RenSurface;

namespace Ren {

class Font;
class TextOptions;

// A 2D drawing API that operates on a target RenSurface.
class Painter
{
public:
    explicit Painter(RenSurface& target);

    // Primitive drawing
    void filledRectangle(const Rect& area, const RenColour& colour) const;
    void hollowRectangle(const Rect& area, const RenColour& colour, int thickness) const;
    void ellipse(const Rect& area, const RenColour& outline, const RenColour& fill) const;

    // Line drawing
    void line(const Point& from, const Point& to, const RenColour& colour, int thickness) const;
    void horizontalLine(const Point& start, int length, const RenColour& colour, int thickness) const;
    void verticalLine(const Point& start, int height, const RenColour& colour, int thickness) const;

    // Text drawing
    void drawText(int x, int y, std::string_view text, const Font& font, const TextOptions& options) const;

private:
    RenSurface& target_;
};

} // namespace Ren
