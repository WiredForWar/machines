/*
 * P A I N T E R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _GUI_PAINTER_HPP
#define _GUI_PAINTER_HPP

#include "stdlib/string.hpp"

#include "gui/gui.hpp"

/* //////////////////////////////////////////////////////////////// */

class GuiBorderMetrics;
class GuiBorderDimensions;
class GuiFilledBorderColours;

// Interface for the GuiPainter Singleton
class IGuiPainter
{
public:
    virtual ~IGuiPainter() = 0;

    // Blit bitmap to backbuffer
    virtual void blit(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Coord& dest) const = 0;

    // Same as above only sourceArea is entire area of the source GuiBitmap
    virtual void blit(const GuiBitmap& source, const Gui::Coord& dest) const = 0;

    // Same as above only dest is defaulted to (0,0)
    virtual void blit(const GuiBitmap& source) const = 0;

    virtual void tile(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea) const = 0;

    virtual void stretch(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea) const = 0;

    virtual void stretch(const GuiBitmap& source, const Gui::Box& destArea) const = 0;

    virtual void blitInRequestedSize(const GuiBitmap& source, const Gui::Coord& dest) const = 0;

    ///////////////////////////////

    virtual void filledRectangle(const Gui::Box&, const Gui::Colour&) const = 0;
    virtual void hollowRectangle(const Gui::Box&, const Gui::Colour&, unsigned thickness) const = 0;

    virtual void bevel(const Gui::Box& b, unsigned thickness, const Gui::Colour& hiCol, const Gui::Colour& loCol) const
        = 0;

    ///////////////////////////////

    virtual void line(const Gui::Coord& c1, const Gui::Coord& c2, const Gui::Colour&, unsigned thickness) const = 0;

    virtual void horizontalLine(const Gui::Coord& c1, unsigned length, const Gui::Colour&, unsigned thickness) const
        = 0;

    virtual void verticalLine(const Gui::Coord& c1, unsigned height, const Gui::Colour&, unsigned thickness) const = 0;

    ///////////////////////////////

    virtual void
    drawText(const Gui::Coord& c, const std::string& text, const Gui::TextOptions& options, int pixelSize = -1) const
        = 0;

    ///////////////////////////////

    virtual void filledBorder(
        const Gui::Coord& absCoord,
        const GuiBorderDimensions&,
        const GuiFilledBorderColours&,
        const GuiBorderMetrics&) const
        = 0;
};

class GuiPainter : public IGuiPainter
// Singleton
{
public:
    static GuiPainter& instance();

    ///////////////////////////////

    // Blit bitmap to backbuffer
    void blit(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Coord& dest) const override;

    // Same as above only sourceArea is entire area of the source GuiBitmap
    void blit(const GuiBitmap& source, const Gui::Coord& dest) const override;

    // Same as above only dest is defaulted to (0,0)
    void blit(const GuiBitmap& source) const override;

    void tile(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea) const override;

    void stretch(const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea) const override;

    void stretch(const GuiBitmap& source, const Gui::Box& destArea) const override;

    void blitInRequestedSize(const GuiBitmap& source, const Gui::Coord& dest) const override;

    ///////////////////////////////

    void filledRectangle(const Gui::Box&, const Gui::Colour&) const override;
    void hollowRectangle(const Gui::Box&, const Gui::Colour&, unsigned thickness) const override;

    void
    bevel(const Gui::Box& b, unsigned thickness, const Gui::Colour& hiCol, const Gui::Colour& loCol) const override;

    ///////////////////////////////

    void line(const Gui::Coord& c1, const Gui::Coord& c2, const Gui::Colour&, unsigned thickness) const override;

    void horizontalLine(const Gui::Coord& c1, unsigned length, const Gui::Colour&, unsigned thickness) const override;

    void verticalLine(const Gui::Coord& c1, unsigned height, const Gui::Colour&, unsigned thickness) const override;

    ///////////////////////////////

    void drawText(const Gui::Coord& c, const std::string& text, const Gui::TextOptions& options, int pixelSize = -1)
        const override;

    ///////////////////////////////

    void filledBorder(
        const Gui::Coord& absCoord,
        const GuiBorderDimensions&,
        const GuiFilledBorderColours&,
        const GuiBorderMetrics&) const override;

private:
    GuiPainter(const GuiPainter&);
    GuiPainter& operator=(const GuiPainter&);
    bool operator==(const GuiPainter&) const;

    GuiPainter();
};

/* //////////////////////////////////////////////////////////////// */

#endif //  #ifndef _GUI_PAINTER_HPP
