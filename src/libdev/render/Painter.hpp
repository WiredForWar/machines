/*
 * P A I N T E R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#pragma once

#include "render/Alignment.hpp"
#include "render/render.hpp"

#include <optional>
#include <string_view>

class RenColour;
class RenDevice;
class RenScopedImmediateCommands;
class RenSurface;

namespace Ren {

class BmpFont;
class Font;
class TextOptions;

// A 2D drawing API that operates on a target RenSurface.
class Painter
{
public:
    explicit Painter(RenSurface& target);
    ~Painter();

    Painter(const Painter&) = delete;
    Painter& operator=(const Painter&) = delete;

    // Primitive drawing
    void filledRectangle(const Rect& area, const RenColour& colour) const;
    void clearRectangle(const Rect& area) const;
    void hollowRectangle(const Rect& area, const RenColour& colour, int thickness) const;
    void ellipse(const Rect& area, const RenColour& outline, const RenColour& fill) const;

    // Line drawing
    void line(const Point& from, const Point& to, const RenColour& colour, int thickness) const;
    void horizontalLine(const Point& start, int length, const RenColour& colour, int thickness) const;
    void verticalLine(const Point& start, int height, const RenColour& colour, int thickness) const;

    // Text drawing
    void drawText(int x, int y, std::string_view text, const Font& font, const TextOptions& options) const;
    void drawText(
        std::string_view text,
        const Point& startPos,
        const BmpFont& font,
        int maxWidth,
        Alignment alignment = AlignLeft) const;

    // Blit operations
    void blit(const RenSurface& source, const std::optional<Rect>& srcArea = {}, Point dest = {}, BlitMode mode = {}) const;
    void tileBlit(const RenSurface& source, const Rect& srcArea, const Rect& destArea) const;
    void stretchBlit(const RenSurface& source, const Rect& srcArea, const Rect& destArea, BlitMode mode = {}) const;
    void stretchBlit(const RenSurface& source, BlitMode mode = {}) const;
    void blitInRequestedSize(const RenSurface& source, Point dest, BlitMode mode = {}) const;

private:
    void beginOffscreen();
    void endOffscreen();

    int screenspaceW() const;
    int screenspaceH() const;

    RenSurface& target_;
    RenDevice* device_{};
};

} // namespace Ren
