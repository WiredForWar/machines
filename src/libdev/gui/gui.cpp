#include <iostream>
#include <stdio.h>
#include "stdlib/string.hpp"
#include "gui/gui.hpp"
#include "gui/displaya.hpp"
#include "system/pathname.hpp"

/* //////////////////////////////////////////////////////////////// */

// static
RenSurface& Gui::staticBackBuffer()
{
    static RenSurface backBuffer_;
    return backBuffer_;
}

static bool s_GuiInitialised = false;

/* //////////////////////////////////////////////////////////////// */

bool Gui::initialised()
{
    return s_GuiInitialised;
}

RenSurface& Gui::backBuffer()
{
    PRE(Gui::initialised());
    return staticBackBuffer();
}

void Gui::backBuffer(const RenSurface& pNewBuffer)
{
    s_GuiInitialised = true;
    staticBackBuffer() = pNewBuffer;
}

GuiBitmap Gui::requestScaledImage(std::string path, float scale)
{
    const bool hasBmpExtention = path.size() > 4 && path.substr(path.size() - 4, 4) == ".bmp";
    if (scale == 1)
    {
        if (!hasBmpExtention)
        {
            path += ".bmp";
        }
        return Gui::bitmap(path);
    }

           // TODO: Fix later :eyes:
    std::string scaledImagePath = path;
    if (hasBmpExtention)
    {
        const auto from = scaledImagePath.end() - 4;
        scaledImagePath.replace(from, scaledImagePath.end(), "_2x.png");
    }
    else
    {
        scaledImagePath += "_2x.png";
    }

    if (SysPathName::existsAsFile(scaledImagePath))
    {
        return Gui::bitmap(scaledImagePath);
    }

    GuiBitmap result = Gui::bitmap(hasBmpExtention ? path : path + ".bmp");
    result.setRequestedSize(result.size() * scale);
    return result;
}

/* //////////////////////////////////////////////////////////////// */

bool operator==(const GuiColour& a, const GuiColour& b)
{
    return a.r() == b.r() && a.g() == b.g() && a.b() == b.b();
}

/* //////////////////////////////////////////////////////////////// */

const Gui::Colour& Gui::BLACK()
{
    static Gui::Colour col_(0, 0, 0);
    return col_;
}

const Gui::Colour& Gui::WHITE()
{
    static Gui::Colour col_(1, 1, 1);
    return col_;
}

const Gui::Colour& Gui::RED()
{
    static Gui::Colour col_(1, 0, 0);
    return col_;
}

const Gui::Colour& Gui::GREEN()
{
    static Gui::Colour col_(0, 1, 0);
    return col_;
}

const Gui::Colour& Gui::BLUE()
{
    static Gui::Colour col_(0, 0, 1);
    return col_;
}

const Gui::Colour& Gui::YELLOW()
{
    static Gui::Colour col_(1, 1, 0);
    return col_;
}

const Gui::Colour& Gui::MAGENTA()
{
    static Gui::Colour col_(1, 0, 1);
    return col_;
}

const Gui::Colour& Gui::CYAN()
{
    static Gui::Colour col_(0, 1, 1);
    return col_;
}

const Gui::Colour& Gui::LIGHTGREY()
{
    static Gui::Colour col_(0.8, 0.8, 0.8);
    return col_;
}

const Gui::Colour& Gui::DARKGREY()
{
    static Gui::Colour col_(0.3, 0.3, 0.3);
    return col_;
}

const Gui::Colour& Gui::GREY()
{
    static Gui::Colour col_(0.5, 0.5, 0.5);
    return col_;
}

/* //////////////////////////////////////////////////////////////// */

// static
GuiBitmap Gui::bitmap(const SysPathName& path)
{
    // LO's assets don't keep case. :(
    string pathName(path.pathname());

    if (path.containsCapitals() && ! path.existsAsFile())
    {
        std::transform(pathName.begin(), pathName.end(), pathName.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
    }
    ASSERT_FILE_EXISTS(pathName.c_str());

    return GuiBitmap::createSharedSurface(pathName, Gui::backBuffer());
}

// static
Gui::Box Gui::bitmapDimensions(const SysPathName& path)
{
    PRE(path.existsAsFile());
    GuiBitmap bmp = GuiBitmap::createSharedSurface(path.pathname(), Gui::backBuffer());
    return Gui::Box(0, 0, bmp.width(), bmp.height());
}

// static
Gui::Box Gui::translateBitmapDimensions(const SysPathName& path, const Gui::Coord& rel)
{
    return GuiDisplayable::translateBox(bitmapDimensions(path), rel);
}

// static
void Gui::writeScreenAsFile(const char* startFilename)
{
    const RenSurface& surface = Gui::backBuffer();

    // Create next filename...

    bool gotBmpPathName = false;
    SysPathName bmpPathName;
    size_t count = 0;

    while (! gotBmpPathName)
    {
        char buffer[20];

        sprintf(buffer, "%04ld", count);

        // bmpPathName = string( startFilename ) + buffer + ".png";
        bmpPathName = SysPathName(string(startFilename) + buffer + ".png");

        if (! bmpPathName.existsAsFile())
            gotBmpPathName = true;

        ++count;
    }

    surface.saveAsPng(bmpPathName);
}

/* //////////////////////////////////////////////////////////////// */

/* End **************************************************************/
