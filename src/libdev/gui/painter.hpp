/*
 * P A I N T E R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _GUI_PAINTER_HPP
#define _GUI_PAINTER_HPP

#include "gui/gui.hpp"
#include "render/Painter.hpp"

/* //////////////////////////////////////////////////////////////// */

class GuiBorderMetrics;
class GuiBorderDimensions;
class GuiFilledBorderColours;

class GuiPainter : public Ren::Painter
{
public:
    static GuiPainter& instance();

    ///////////////////////////////

    using Ren::Painter::blit;
    using Ren::Painter::tileBlit;
    using Ren::Painter::stretchBlit;
    using Ren::Painter::blitInRequestedSize;
    using Ren::Painter::filledRectangle;
    using Ren::Painter::hollowRectangle;
    using Ren::Painter::ellipse;
    using Ren::Painter::line;
    using Ren::Painter::horizontalLine;
    using Ren::Painter::verticalLine;
    using Ren::Painter::drawText;

    ///////////////////////////////

    // Blit overloads accepting Gui types
    void blit(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Coord& dest) const;
    void blit(const GuiBitmap& source, const Gui::Coord& dest) const;
    void tile(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea) const;
    void stretch(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea) const;
    void stretch(const GuiBitmap& source, const Gui::Box& destArea) const;
    void blitInRequestedSize(const GuiBitmap& source, const Gui::Coord& dest) const;

    void filledRectangle(const Gui::Box&, const Gui::Colour&) const;
    void hollowRectangle(const Gui::Box&, const Gui::Colour&, unsigned thickness) const;

    void
    bevel(const Gui::Box& b, unsigned thickness, const Gui::Colour& hiCol, const Gui::Colour& loCol) const;

    ///////////////////////////////

    void line(const Gui::Coord& c1, const Gui::Coord& c2, const Gui::Colour&, unsigned thickness) const;

    void horizontalLine(const Gui::Coord& c1, unsigned length, const Gui::Colour&, unsigned thickness) const;

    void verticalLine(const Gui::Coord& c1, unsigned height, const Gui::Colour&, unsigned thickness) const;

    void drawText(
        const Gui::Coord& c,
        const std::string_view& text,
        const Gui::TextOptions& options,
        const Ren::Font& font) const;

    void drawText(
        const Gui::Box& rect,
        const std::string_view& text,
        const Gui::TextOptions& options,
        const Ren::Font& font) const;

    ///////////////////////////////

    void filledBorder(
        const Gui::Coord& absCoord,
        const GuiBorderDimensions&,
        const GuiFilledBorderColours&,
        const GuiBorderMetrics&) const;

private:
    GuiPainter(const GuiPainter&);
    GuiPainter& operator=(const GuiPainter&);
    bool operator==(const GuiPainter&) const;

    GuiPainter();
};

/* //////////////////////////////////////////////////////////////// */

#endif //  #ifndef _GUI_PAINTER_HPP
