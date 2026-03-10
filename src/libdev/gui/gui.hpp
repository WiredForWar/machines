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

// Finds the next available filename for a png file.
// A startFilename is the first few characters of the filename
// the function will add a four digit number to startFilename
// E.g. writeScreenAsFile("XXXX") would return names "XXXX0000.png"
// followed by "XXXX0001.png", etc.
SysPathName getNextAvailablePngFileName(const std::string& startFilename);

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

Ren::BmpFont getFont(const SysPathName& fontPath);

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
