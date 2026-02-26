/*
 * P A I N T E R . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/Painter.hpp"

#include "render/BmpFont.hpp"
#include "render/colour.hpp"
#include "render/surface.hpp"
#include "render/Font.hpp"
#include "render/TextOptions.hpp"

#include "ctl/vector.hpp"
#include "mathex/point2d.hpp"

namespace Ren {

Painter::Painter(RenSurface& target)
    : target_(target)
{
}

void Painter::filledRectangle(const Rect& area, const RenColour& colour) const
{
    target_.filledRectangle(area, colour);
}

void Painter::clearRectangle(const Rect& area) const
{
    filledRectangle(area, RenColour::magenta());
}

void Painter::hollowRectangle(const Rect& area, const RenColour& colour, int thickness) const
{
    target_.hollowRectangle(area, colour, thickness);
}

void Painter::ellipse(const Rect& area, const RenColour& outline, const RenColour& fill) const
{
    target_.ellipse(area, outline, fill);
}

void Painter::line(const Point& from, const Point& to, const RenColour& colour, int thickness) const
{
    RenSurface::Points pts;
    pts.reserve(2);
    pts.push_back(MexPoint2d(from.x, from.y));
    pts.push_back(MexPoint2d(to.x, to.y));
    target_.polyLine(pts, colour, thickness);
}

void Painter::horizontalLine(const Point& start, int length, const RenColour& colour, int thickness) const
{
    if (thickness > 1)
    {
        const Point adjusted { start.x, start.y - thickness / 2 };
        line(adjusted, Point { adjusted.x + length, adjusted.y }, colour, thickness);
    }
    else
    {
        line(start, Point { start.x + length, start.y }, colour, thickness);
    }
}

void Painter::verticalLine(const Point& start, int height, const RenColour& colour, int thickness) const
{
    line(start, Point { start.x, start.y + height }, colour, thickness);
}

void Painter::drawText(std::string_view text, const Point& startPos, const Font& font, const TextOptions& options) const
{
    drawText(startPos.x, startPos.y, text, font, options);
}

void Painter::drawText(int x, int y, std::string_view text, const Font& font, const TextOptions& options) const
{
    target_.drawText(x, y, text, font, options);
}

void Painter::drawText(
    std::string_view text, const Point& startPos, const BmpFont& font, int maxWidth, Alignment alignment) const
{
    const bool rightJustify = alignment & AlignRight;

    Point absPos = startPos;

    if (rightJustify)
    {
        int endAbsPos = absPos.x - maxWidth;

        for (int i = text.length(); i > 0;)
        {
            --i;

            if (text[i] == ' ')
            {
                absPos.x -= font.spaceCharWidth() + font.spacing();
            }
            else if (font.charWidth(text[i]))
            {
                if (font.fontType() == BmpFont::PROPORTIONAL)
                    absPos.x -= font.charWidth(text[i]) + font.spacing();
                else
                    absPos.x -= font.maxCharWidth() + font.spacing();

                if (absPos.x >= endAbsPos)
                {
                    blit(
                        font.fontBitmap(),
                        Rect(font.charOffset((unsigned char)text[i]), 0, font.charWidth(text[i]) + 1, font.height()),
                        absPos);
                }
                else
                {
                    i = 0;
                }
            }
        }
    }
    else
    {
        int endAbsPos = absPos.x + maxWidth;

        for (int i = 0; i < text.length(); ++i)
        {
            if (text[i] == ' ')
            {
                absPos.x += font.spaceCharWidth() + font.spacing();
            }
            else if (font.charWidth(text[i]))
            {
                if (absPos.x + font.charWidth(text[i]) <= endAbsPos)
                {
                    blit(
                        font.fontBitmap(),
                        Rect(
                            font.charOffset((unsigned char)text[i]), 0, font.charWidth(text[i]) + 1, font.height()),
                        absPos);

                    if (font.fontType() == BmpFont::PROPORTIONAL)
                        absPos.x += font.charWidth(text[i]) + font.spacing();
                    else
                        absPos.x += font.maxCharWidth() + font.spacing();
                }
                else
                {
                    i = text.length();
                }
            }
        }
    }

    if (font.underline())
    {
        line(
            Point(startPos.x, startPos.y + font.height() + 1),
            Point(absPos.x, startPos.y + font.height() + 1),
            font.underlineColour(),
            1);
    }
}

void Painter::blit(
    const RenSurface& source, const std::optional<Rect>& srcArea, Point dest, BlitMode mode) const
{
    target_.simpleBlit(source, srcArea, dest, mode);
}

void Painter::tileBlit(const RenSurface& source, const Rect& srcArea, const Rect& destArea) const
{
    target_.tileBlit(source, srcArea, destArea);
}

void Painter::stretchBlit(
    const RenSurface& source, const Rect& srcArea, const Rect& destArea, BlitMode mode) const
{
    target_.stretchBlit(source, srcArea, destArea, mode);
}

void Painter::stretchBlit(const RenSurface& source, BlitMode mode) const
{
    target_.stretchBlit(source, mode);
}

void Painter::blitInRequestedSize(const RenSurface& source, Point dest, BlitMode mode) const
{
    target_.blitInRequestedSize(source, dest, mode);
}

} // namespace Ren
