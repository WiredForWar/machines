/*
 * G U I . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _GUI_GUI_HPP
#define _GUI_GUI_HPP

#include "device/mouse.hpp"

#include "mathex/abox2d.hpp"
#include "mathex/point2d.hpp"
#include "mathex/vec2.hpp"

#include "render/colour.hpp"
#include "render/surface.hpp"

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
    friend ostream& operator <<( ostream&, const GuiColour& );

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

// Saves the screen as a bitmap file. startFilename is the first few
// characters of the bitmap filename, the function will add a four digit
// number to startFilename ( e.g. writeScreenAsFile( "XXXX" ) would store
// a file with name e.g "XXXX0000.png" followed by "XXXX0001.png" etc ).
void writeScreenAsFile(const char* startFilename);

RenSurface& staticBackBuffer();

};

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
