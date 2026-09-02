/*
 * G U I . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _GUI_GUI_HPP
#define _GUI_GUI_HPP

#include "device/Mouse.hpp"

#include "mathex/AlignedBox2d.hpp"
#include "mathex/Point2d.hpp"
#include "mathex/Vec2.hpp"

#include "render/Alignment.hpp"
#include "render/Colour.hpp"
#include "render/Surface.hpp"

/* //////////////////////////////////////////////////////////////// */

// class ostream;

/************

class GuiColour
{
public:

    GuiColour( double r, double g, double b )
    : r_( r ), g_( g ), b_( b ) {}

    double r() const { return r_; }
    double g() const { return g_; }
    double b() const { return b_; }

    friend bool operator ==( const GuiColour&, const GuiColour& );
    friend std::ostream& operator <<( ostream&, const GuiColour& );

private:

    double  r_;
    double  g_;
    double  b_;
};

************/

using GuiColour = RenColour;

class GuiCoords;
// class GuiBitmap;
using GuiBitmap = RenSurface;

bool operator<(const GuiBitmap&, const GuiBitmap&);

namespace Ren
{

class BmpFont;
class Font;
class TextOptions;
struct BmpFontMetrics;

} // namespace Ren

using GuiBmpFont = Ren::BmpFont;

namespace Gui
{

bool initialised();

using Colour = GuiColour;
using XCoord = int;
using YCoord = int;
using Coord = MexPoint2d;
using Vec = MexVec2;
using Boundary = MexAlignedBox2d;
using Box = MexAlignedBox2d;
using Size = MexSize2d;
using namespace Ren::Align;
using Alignment = Ren::Alignment;
using TextOptions = Ren::TextOptions;

inline Size toSize(const Ren::Size& size)
{
    return Size(size.width, size.height);
}

// Deprecated
using WidthAndHeight = Size;

RenSurface& backBuffer();
// PRE( Gui::initialised(); )

///////////////////////////////

enum ButtonState
{
    PRESSED = DevMouse::PRESSED,
    RELEASED = DevMouse::RELEASED,
    NO_CHANGE = DevMouse::NO_CHANGE
};

enum class ScrollState
{
    NO_SCROLL,
    SCROLL_UP,
    SCROLL_DOWN
};

MATHEX_SCALAR uiScaleFactor();
void setUiScaleFactor(MATHEX_SCALAR scale);

///////////////////////////////

Gui::Box bitmapDimensions(const SysPathName& path);
// PRE( path.existsAsFile() );

Gui::Box translateBitmapDimensions(const SysPathName& path, const Gui::Coord& rel);
// PRE( path.existsAsFile() );

///////////////////////////////

GuiBitmap bitmap(const SysPathName&);

///////////////////////////////

const Gui::Colour& BLACK();
const Gui::Colour& WHITE();
const Gui::Colour& RED();
const Gui::Colour& GREEN();
const Gui::Colour& BLUE();
const Gui::Colour& YELLOW();
const Gui::Colour& MAGENTA();
const Gui::Colour& CYAN();
const Gui::Colour& LIGHTGREY();
const Gui::Colour& DARKGREY();
const Gui::Colour& GREY();

///////////////////////////////

void backBuffer(const RenSurface&);
// PRE( pNewBuffer != NULL );

///////////////////////////////

RenSurface& staticBackBuffer();

std::string getScaledImagePath(std::string path, float scale = uiScaleFactor());

/**
 * @brief Returns the original bitmap or its HiDPI alternative if available
 * @param path The path to the image file (with .bmp or without any extension)
 * @param scale The wanted scale factor
 * @return image as is or HiDPI bitmap
 */
GuiBitmap requestScaledImage(std::string path, float scale = uiScaleFactor());

/**
 * @brief Returns a bitmap scaled by uiScaleFactor() or a HiDPI alternatives
 * @param path to the image file (with .bmp or without any extension)
 * @param scale The wanted scale factor
 * @return scaled image or HiDPI bitmap
 */
GuiBitmap getScaledImage(std::string path, float scale = uiScaleFactor());

// Derive every image getScaledImage() has handed out again, into the surface the
// caller is already holding. A caller keeps the surface rather than the request,
// and the longest-lived ones ask only once per process, so this is what shows
// them a file that has changed underneath -- a mod switched on or off, or an art
// file replaced. Surfaces nothing holds any more are dropped rather than redrawn.
void rebuildScaledImages();

// Say what shape the art in an atlas is, so that text drawn in it can be placed by
// its baseline rather than by the line box around it. The metrics are those of the
// unscaled art. This has to be said before the font is first asked for, because an
// atlas is only read once.
void setFontMetrics(const SysPathName& fontPath, const Ren::BmpFontMetrics& metrics);

Ren::BmpFont getFont(const SysPathName& fontPath);

// Where the baseline of a line of text sits within a box of the given height,
// measured down from the top of the box.
//
// Text is placed from its baseline. The line box around it carries room for glyphs
// that reach higher and drop lower than the ones actually being drawn, so centring
// that box leaves the text looking low; what is centred here is the capitals, which
// is what the eye reads the middle of a line by. Ask for the same box height with
// the same font anywhere and the answer is the same, so two things drawn beside one
// another sit on one line.
int baselineIn(MATHEX_SCALAR boxHeight, int capHeight);
int baselineIn(MATHEX_SCALAR boxHeight, const Ren::BmpFont& font);
int baselineIn(MATHEX_SCALAR boxHeight, const Ren::Font& font);

// The top of the line box for text on that baseline, which is where drawing starts.
int textTopIn(MATHEX_SCALAR boxHeight, const Ren::BmpFont& font);
int textTopIn(MATHEX_SCALAR boxHeight, const Ren::Font& font);

// Free's up all the memory used by the cached fonts.
void releaseFontMemory();

}; // namespace Gui

/* //////////////////////////////////////////////////////////////// */

/****************************

class GuiBitmap
: public RenSurface
{
public:

    GuiBitmap();
    GuiBitmap( const SysPathName& path )
    {
        readFromFile( path, Gui::backBuffer() );
    }

    ~GuiBitmap() {};

};

****************************/

/* //////////////////////////////////////////////////////////////// */

#endif //  #ifndef _GUI_GUI_HPP
