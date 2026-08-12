/*
 * P A I N T E R . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#include "gui/GuiPainter.hpp"

#include "gui/Border.hpp"

#include "ctl/Vector.hpp"
#include "render/Font.hpp"
#include "render/Surface.hpp"

/* //////////////////////////////////////////////////////////////// */


GuiPainter& GuiPainter::instance()
{
    static GuiPainter instance_;
    return instance_;
}

/* //////////////////////////////////////////////////////////////// */

GuiPainter::GuiPainter()
    : Ren::Painter(Gui::staticBackBuffer())
{
}

//////////////////////////////////////////////////////////////////////

RenSurface::Rect map_GuiBox_to_RenSurfaceRect(const Gui::Box& b)
{
    return RenSurface::Rect(b.minCorner().x(), b.minCorner().y(), b.width(), b.height());
}

void GuiPainter::filledRectangle(const Gui::Box& b, const Gui::Colour& col) const
{
    Ren::Painter::filledRectangle(map_GuiBox_to_RenSurfaceRect(b), col);
}

//////////////////////////////////////////////////////////////////////

void GuiPainter::hollowRectangle(const Gui::Box& b, const Gui::Colour& col, unsigned thickness) const
{
    Ren::Painter::hollowRectangle(map_GuiBox_to_RenSurfaceRect(b), col, thickness);
}

/* //////////////////////////////////////////////////////////////// */

void GuiPainter::line(const Gui::Coord& c1, const Gui::Coord& c2, const Gui::Colour& col, unsigned thickness) const
{
    Ren::Painter::line(Ren::Point(c1.x(), c1.y()), Ren::Point(c2.x(), c2.y()), col, thickness);
}

void GuiPainter::horizontalLine(const Gui::Coord& c1, unsigned length, const Gui::Colour& col, unsigned thickness) const
{
    Ren::Painter::horizontalLine(Ren::Point(c1.x(), c1.y()), length, col, thickness);
}

void GuiPainter::verticalLine(const Gui::Coord& c1, unsigned height, const Gui::Colour& col, unsigned thickness) const
{
    Ren::Painter::verticalLine(Ren::Point(c1.x(), c1.y()), height, col, thickness);
}

/* //////////////////////////////////////////////////////////////// */

void GuiPainter::bevel(const Gui::Box& b, unsigned thickness, const Gui::Colour& hiCol, const Gui::Colour& loCol) const
{
    const Gui::Coord& c = b.minCorner();
    Gui::XCoord x1 = c.x();
    Gui::YCoord y1 = c.y();
    Gui::XCoord width = b.width();
    Gui::YCoord height = b.height();

    // Each edge is a line of its own rather than a rectangle, because a rectangle
    // one pixel across is not one that hollowRectangle can draw.
    for (unsigned i = 0; i < thickness && width > 0 && height > 0; ++i, ++x1, ++y1, width -= 2, height -= 2)
    {
        // Lit from the top left, so the near edges catch the light and the far ones
        // fall away. The corners go to whichever edge reaches them first.
        filledRectangle(Gui::Box(Gui::Coord(x1, y1), width, 1), hiCol);
        filledRectangle(Gui::Box(Gui::Coord(x1, y1), 1, height), hiCol);
        filledRectangle(Gui::Box(Gui::Coord(x1 + width - 1, y1 + 1), 1, height - 1), loCol);
        filledRectangle(Gui::Box(Gui::Coord(x1 + 1, y1 + height - 1), width - 1, 1), loCol);
    }
}

//////////////////////////////////////////////////////////////////////

void GuiPainter::drawText(
    const Gui::Coord& c,
    const std::string_view& text,
    const Gui::TextOptions& options,
    const Ren::Font& font) const
{
    Ren::Painter::drawText(c.x(), c.y(), text, font, options);
}

void GuiPainter::drawText(
    const Gui::Box& rect, const std::string_view& text, const Gui::TextOptions& options, const Ren::Font& font) const
{
    std::size_t textWidth = font.horizontalAdvance(text, options);
    std::size_t textHeight = font.height();
    std::size_t textX = rect.minCorner().x() + (rect.width() - textWidth) / 2.0;
    std::size_t textY = rect.minCorner().y() + (rect.height() - textHeight) / 2.0;

    drawText(Gui::Coord(textX, textY), text, options, font);
}

//////////////////////////////////////////////////////////////////////

void GuiPainter::blit(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Coord& dest) const
{
    Ren::Painter::blit(source, map_GuiBox_to_RenSurfaceRect(sourceArea), Ren::Point(dest.x(), dest.y()));
}

void GuiPainter::blit(const GuiBitmap& source, const Gui::Coord& dest) const
{
    Ren::Painter::blit(source, {}, Ren::Point(dest.x(), dest.y()));
}

void GuiPainter::tile(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea) const
{
    Ren::Painter::tileBlit(
        source,
        map_GuiBox_to_RenSurfaceRect(sourceArea),
        map_GuiBox_to_RenSurfaceRect(destArea));
}

void GuiPainter::stretch(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea) const
{
    Ren::Painter::stretchBlit(
        source,
        map_GuiBox_to_RenSurfaceRect(sourceArea),
        map_GuiBox_to_RenSurfaceRect(destArea));
}

void GuiPainter::stretch(const GuiBitmap& source, const Gui::Box& destArea) const
{
    Ren::Painter::stretchBlit(source, source.size(), map_GuiBox_to_RenSurfaceRect(destArea));
}

void GuiPainter::blitInRequestedSize(const GuiBitmap& source, const Gui::Coord& dest) const
{
    Ren::Painter::blitInRequestedSize(source, Ren::Point(dest.x(), dest.y()));
}

//////////////////////////////////////////////////////////////////////

void GuiPainter::filledBorder(
    const Gui::Coord& absCoord,
    const GuiBorderDimensions& dim,
    const GuiFilledBorderColours& colours,
    const GuiBorderMetrics& m) const
{
    const int frameThickness = m.frameThickness();
    const int highlightThickness = m.highlightThickness();
    const int shadowThickness = m.shadowThickness();

    hollowRectangle(
        Gui::Box(absCoord, dim.exteriorWidth(), dim.exteriorHeight()),
        colours.frameColour(),
        frameThickness);

    // What the frame leaves for the bevel to be drawn in.
    const int left = absCoord.x() + frameThickness;
    const int top = absCoord.y() + frameThickness;
    const int width = dim.exteriorWidth() - (2 * frameThickness);
    const int height = dim.exteriorHeight() - (2 * frameThickness);

    // Each side of the bevel is a rectangle rather than a line, because a line of a
    // given width is put where it rasterises, which is not where it was asked for.
    auto band = [this](int x, int y, int bandWidth, int bandHeight, const Gui::Colour& colour) {
        if (bandWidth > 0 && bandHeight > 0)
            filledRectangle(Gui::Box(Gui::Coord(x, y), bandWidth, bandHeight), colour);
    };

    // Lit from the top left, so the far sides fall away and the near ones catch the
    // light. The near sides run the whole way, so the two corners where they meet the
    // far ones go to the light.
    band(left, top + height - shadowThickness, width, shadowThickness, colours.shadowColour());
    band(left + width - shadowThickness, top, shadowThickness, height, colours.shadowColour());

    band(left, top, width, highlightThickness, colours.highlightColour());
    band(left, top, highlightThickness, height, colours.highlightColour());
}

/*********************************************************************

void GuiPainter::filledBorder( const Gui::Coord& absCoord,
                                const GuiBorderDimensions& dim,
                                const GuiFilledBorderColours& colours )
{

    unsigned w = dim.exteriorWidth() - 1;
    unsigned h = dim.exteriorHeight() - 1;

    const GuiBorderMetrics& metrics = dim.metrics();

    horizontalLineGW( colours.outlineColour(), absCoord.x() + 1, absCoord.y(), w );
    horizontalLineGW( colours.outlineColour(), absCoord.x() + 1, absCoord.y() + h + 1, w );

    verticalLineGW( colours.outlineColour(), absCoord().x(), absCoord().y() + 1, h );
    verticalLineGW( colours.outlineColour(), absCoord().x() + w + 1, absCoord().y() + 1, h );

    bevelGW( colours.highlightColour(),
                colours.shadowColour(),
                absCoord().x() + 2,
                absCoord().y() + 1, 1,
                width() - 2, height() - 2 );
}

*********************************************************************/

//////////////////////////////////////////////////////////////////////

/* End PAINTER.CPP **************************************************/
