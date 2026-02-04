#include "gui/gui.hpp"
#include "gui/displaya.hpp"
#include "system/pathname.hpp"
#include "system/vfs.hpp"

#include <cassert>
#include <stdio.h>

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

static thread_local char s_ScaledTextureSuffix[] = "_2x.png";
static constexpr char s_PngTextureSuffix[] = ".png";
static constexpr char s_BmpTextureSuffix[] = ".bmp";
constexpr auto s_BmpSuffixSize = sizeof(s_BmpTextureSuffix) - 1;

std::string Gui::getScaledImagePath(std::string path, float scale)
{
    const bool hasBmpExtention = path.size() > s_BmpSuffixSize
        && path.substr(path.size() - s_BmpSuffixSize, s_BmpSuffixSize) == s_BmpTextureSuffix;

    if (scale == 1)
    {
        if (hasBmpExtention)
            return path;

        return path + s_BmpTextureSuffix;
    }

    s_ScaledTextureSuffix[1] = '0' + static_cast<int>(scale);
    if (hasBmpExtention)
    {
        const auto from = path.end() - s_BmpSuffixSize;
        path.replace(from, path.end(), s_ScaledTextureSuffix);
        return path;
    }

    return path + s_ScaledTextureSuffix;
}

GuiBitmap Gui::requestScaledImage(std::string path, float scale)
{
    const bool hasBmpExtention = path.size() > s_BmpSuffixSize
        && path.substr(path.size() - s_BmpSuffixSize, s_BmpSuffixSize) == s_BmpTextureSuffix;

    if (scale == 1)
    {
        std::string pngImagePath;
        if (hasBmpExtention)
        {
            pngImagePath = path;
            const auto from = pngImagePath.end() - s_BmpSuffixSize;
            pngImagePath.replace(from, pngImagePath.end(), s_PngTextureSuffix);
        }
        else
        {
            pngImagePath = path + s_PngTextureSuffix;
            path += s_BmpTextureSuffix;
        }

        pngImagePath = System::findFile(pngImagePath);

        // Prefer (try first) png images
        if (SysPathName::existsAsFile(pngImagePath))
        {
            return Gui::bitmap(pngImagePath);
        }

        path = System::findFile(path);
        return Gui::bitmap(path);
    }

    s_ScaledTextureSuffix[1] = '0' + static_cast<int>(scale);
    std::string imagePath = path;
    if (hasBmpExtention)
    {
        const auto from = imagePath.end() - s_BmpSuffixSize;
        imagePath.replace(from, imagePath.end(), s_ScaledTextureSuffix);
    }
    else
    {
        imagePath += s_ScaledTextureSuffix;
    }

    imagePath = System::findFile(imagePath);
    if (SysPathName::existsAsFile(imagePath))
    {
        return Gui::bitmap(imagePath);
    }

    imagePath = System::findFile(hasBmpExtention ? path : path + s_BmpTextureSuffix);
    GuiBitmap result = Gui::bitmap(imagePath);
    result.setRequestedSize(result.size() * scale);
    return result;
}

GuiBitmap Gui::getScaledImage(std::string path, float scale)
{
    GuiBitmap image = Gui::requestScaledImage(path, scale);
    if (image.requestedSize().isNull())
        return image;

    RenSurface scaledSurface = RenSurface::createAnonymousSurface(image.requestedSize(), image);

    // Workaround artefacts in transparent pixels:
    scaledSurface.filledRectangle(image.requestedSize(), Gui::MAGENTA());
    scaledSurface.enableColourKeying();

    scaledSurface.stretchBlit(image, Ren::BlitMode::Replace);

    return scaledSurface;
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
    std::string pathName(path.pathname());

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

SysPathName Gui::getNextAvailablePngFileName(const std::string& startFilename)
{
    constexpr std::size_t MaxScreenshots = 100000;

    for(std::size_t number = 0; number < MaxScreenshots; ++number)
    {
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%04zu", number);
        SysPathName pathName(startFilename + buffer + ".png");
        if (!pathName.existsAsFile())
            return pathName;
    }

    return {};
}

/* //////////////////////////////////////////////////////////////// */
static double s_uiScaleFactor = 0;

MATHEX_SCALAR Gui::uiScaleFactor()
{
    assert(s_uiScaleFactor > 0);
    return s_uiScaleFactor;
}

void Gui::setUiScaleFactor(MATHEX_SCALAR scale)
{
    s_ScaledTextureSuffix[1] = '0' + static_cast<int>(scale);
    s_uiScaleFactor = scale;
}

/* End **************************************************************/
