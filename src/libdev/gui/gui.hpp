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

// namespace Gui
struct Gui
{
    static bool initialised();

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

    static RenSurface& backBuffer();
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

    static Gui::Box bitmapDimensions(const SysPathName& path);
    // PRE( path.existsAsFile() );

    static Gui::Box translateBitmapDimensions(const SysPathName& path, const Gui::Coord& rel);
    // PRE( path.existsAsFile() );

    ///////////////////////////////

    static GuiBitmap bitmap(const SysPathName&);

    ///////////////////////////////

    static const Gui::Colour& BLACK();
    static const Gui::Colour& WHITE();
    static const Gui::Colour& RED();
    static const Gui::Colour& GREEN();
    static const Gui::Colour& BLUE();
    static const Gui::Colour& YELLOW();
    static const Gui::Colour& MAGENTA();
    static const Gui::Colour& CYAN();
    static const Gui::Colour& LIGHTGREY();
    static const Gui::Colour& DARKGREY();
    static const Gui::Colour& GREY();

    ///////////////////////////////

    static void backBuffer(const RenSurface&);
    // PRE( pNewBuffer != NULL );

    ///////////////////////////////

    // Saves the screen as a bitmap file. startFilename is the first few
    // characters of the bitmap filename, the function will add a four digit
    // number to startFilename ( e.g. writeScreenAsFile( "XXXX" ) would store
    // a file with name e.g "XXXX0000.png" followed by "XXXX0001.png" etc ).
    static void writeScreenAsFile(const char* startFilename);

private:
    static RenSurface& staticBackBuffer();
    static bool initialised_;
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
