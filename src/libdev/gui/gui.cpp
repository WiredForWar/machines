#include "gui/gui.hpp"
#include "gui/Displayable.hpp"
#include "render/BmpFont.hpp"
#include "render/Font.hpp"
#include "render/Painter.hpp"
#include "render/SurfaceManager.hpp"
#include "system/PathName.hpp"
#include "system/VFS.hpp"

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

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
        std::string pngPath;
        if (hasBmpExtention)
        {
            pngPath = path;
            const auto from = pngPath.end() - s_BmpSuffixSize;
            pngPath.replace(from, pngPath.end(), s_PngTextureSuffix);
        }
        else
        {
            pngPath = path + s_PngTextureSuffix;
            path += s_BmpTextureSuffix;
        }

        // Prefer png over bmp
        pngPath = System::findFile(pngPath);
        if (SysPathName::existsAsFile(pngPath))
            return pngPath;

        return System::findFile(path);
    }

    s_ScaledTextureSuffix[1] = '0' + static_cast<int>(scale);
    if (hasBmpExtention)
    {
        const auto from = path.end() - s_BmpSuffixSize;
        path.replace(from, path.end(), s_ScaledTextureSuffix);
    }
    else
    {
        path += s_ScaledTextureSuffix;
    }

    return path.empty() ? path : System::findFile(path);
}

GuiBitmap Gui::requestScaledImage(std::string path, float scale)
{
    std::string imagePath = getScaledImagePath(path, scale);

    if (scale == 1)
        return Gui::bitmap(imagePath);

    if (SysPathName::existsAsFile(imagePath))
        return Gui::bitmap(imagePath);

    GuiBitmap result = Gui::bitmap(getScaledImagePath(path, 1));
    result.setRequestedSize(result.size() * scale);
    return result;
}

namespace
{

// A surface getScaledImage() derived by stretching, and what it was derived
// from. A caller keeps the surface and not the request behind it -- often in a
// function-local static, which runs once for the life of the process -- so the
// only way to show it a file that has changed is to put new pixels into the
// surface it is already holding.
struct DerivedImage
{
    RenSurface surface{};
    std::string path{};
    float scale{};
};

std::vector<DerivedImage>& derivedImages()
{
    static std::vector<DerivedImage> images;
    return images;
}

// What a request comes back at: the size that was asked for when the image has
// to be stretched to reach it, and the file's own size when a file of the right
// size was found.
Ren::Size scaledSize(const GuiBitmap& source)
{
    return source.requestedSize().isNull() ? source.size() : source.requestedSize();
}

void drawScaled(RenSurface& target, const GuiBitmap& source)
{
    // Workaround artefacts in transparent pixels:
    Ren::Painter painter(target);
    painter.clearRectangle(target.size());

    // Keying is what lets a stretch keep its transparent parts. A file that was
    // already the right size is copied as it stands, keying and all, because
    // handing that file straight back is what this used to do.
    // Said either way round, so that a surface derived a second time from a
    // different file does not keep the state the first one gave it.
    if (!source.requestedSize().isNull() || source.isColourKeyingOn())
        target.enableColourKeying();
    else
        target.disableColourKeying();

    painter.stretchBlit(source, Ren::BlitMode::Replace);
}

// The image already derived for this request, or nothing if there is none. The
// pointer is into the list, so it is only good until the list is next touched.
DerivedImage* findDerivedImage(const std::string& path, float scale)
{
    std::vector<DerivedImage>& images = derivedImages();
    for (std::vector<DerivedImage>::iterator it = images.begin(); it != images.end(); ++it)
    {
        if (it->scale == scale && it->path == path)
            return &*it;
    }

    return nullptr;
}

// Forget the images this list is the last holder of. Nothing draws them, so
// nothing would see them redrawn, and the surface would otherwise last as long
// as the process. Some callers ask for an image inside the function that draws
// it, once a frame for as long as something is on screen, so this has to happen
// as images are added and not only when they are rebuilt.
void forgetUnheldImages()
{
    const RenSurfaceManager& manager = RenSurfaceManager::instance();
    std::vector<DerivedImage>& images = derivedImages();

    images.erase(
        std::remove_if(
            images.begin(),
            images.end(),
            [&manager](const DerivedImage& image) { return manager.refCount(image.surface) <= 1; }),
        images.end());
}

} // namespace

GuiBitmap Gui::getScaledImage(std::string path, float scale)
{
    forgetUnheldImages();

    // One surface per request, so that asking twice costs one image rather than
    // two. The exact-size case used to share the file's own surface between its
    // callers and this keeps that; the stretched case used to derive a fresh
    // surface per call, and this shares those too.
    if (const DerivedImage* found = findDerivedImage(path, scale))
        return found->surface;

    const GuiBitmap image = Gui::requestScaledImage(path, scale);

    // A surface of this function's own, even where the file found was already
    // the size asked for and could have been handed back as it stands. A file's
    // surface is named after that file, so a caller holding one is holding the
    // file, and there is no way to tell it that the answer to its request is a
    // different file now -- which is what switching a mod off is. A surface
    // derived here belongs to the request, and the request can be asked again.
    RenSurface scaledSurface = RenSurface::createAnonymousSurface(scaledSize(image));
    drawScaled(scaledSurface, image);

    derivedImages().push_back({ scaledSurface, std::move(path), scale });

    return scaledSurface;
}

void Gui::rebuildScaledImages()
{
    forgetUnheldImages();

    for (DerivedImage& image : derivedImages())
    {
        // Ask as if for the first time: the file may have changed, gained a
        // scaled companion or lost one. What comes back is drawn at the size the
        // surface already has, which is the size everything laid out around it
        // was told to expect.
        drawScaled(image.surface, Gui::requestScaledImage(image.path, image.scale));
    }
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

    return GuiBitmap::createSharedSurface(pathName);
}

// static
Gui::Box Gui::bitmapDimensions(const SysPathName& path)
{
    PRE(path.existsAsFile());
    GuiBitmap bmp = GuiBitmap::createSharedSurface(path.pathname());
    return Gui::Box(0, 0, bmp.width(), bmp.height());
}

// static
Gui::Box Gui::translateBitmapDimensions(const SysPathName& path, const Gui::Coord& rel)
{
    return GuiDisplayable::translateBox(bitmapDimensions(path), rel);
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

/* //////////////////////////////////////////////////////////////// */

static std::map<std::string, Ren::BmpFontMetrics>& getFontMetrics()
{
    static std::map<std::string, Ren::BmpFontMetrics> metrics;
    return metrics;
}

void Gui::setFontMetrics(const SysPathName& fontPath, const Ren::BmpFontMetrics& metrics)
{
    getFontMetrics()[fontPath.pathname()] = metrics;
}

static std::vector<Ren::BmpFont>& getFontCache()
{
    static std::vector<Ren::BmpFont> fonts;
    static bool firstTime = true;
    if (firstTime)
    {
        firstTime = false;
        fonts.reserve(10);
    }
    return fonts;
}

Ren::BmpFont Gui::getFont(const SysPathName& fontPath)
{
    auto& fonts = getFontCache();
    size_t spacing = 1 * Gui::uiScaleFactor();
    size_t spaceCharWidth = 5 * Gui::uiScaleFactor();

    for (auto& cached : fonts)
    {
        if (cached.fontPath() == fontPath)
        {
            Ren::BmpFont font(cached);
            font.fontType(Ren::BmpFont::PROPORTIONAL);
            font.spaceCharWidth(spaceCharWidth);
            font.spacing(spacing);
            return font;
        }
    }

    const auto& metrics = getFontMetrics();
    const auto found = metrics.find(fontPath.pathname());
    const Ren::BmpFontMetrics& fontMetrics = (found != metrics.end()) ? found->second : Ren::BmpFontMetrics{};

    Ren::BmpFont newFont(fontPath, static_cast<std::size_t>(Gui::uiScaleFactor()), fontMetrics);
    newFont.fontType(Ren::BmpFont::PROPORTIONAL);
    newFont.spaceCharWidth(spaceCharWidth);
    newFont.spacing(spacing);
    fonts.push_back(newFont);
    return newFont;
}

int Gui::baselineIn(MATHEX_SCALAR boxHeight, int capHeight)
{
    // Sit the capitals in the middle; the baseline is the line they stand on.
    return static_cast<int>((boxHeight - capHeight) / 2) + capHeight;
}

int Gui::baselineIn(MATHEX_SCALAR boxHeight, const Ren::BmpFont& font)
{
    return baselineIn(boxHeight, font.capHeight());
}

int Gui::baselineIn(MATHEX_SCALAR boxHeight, const Ren::Font& font)
{
    return baselineIn(boxHeight, font.capHeight());
}

int Gui::textTopIn(MATHEX_SCALAR boxHeight, const Ren::BmpFont& font)
{
    return baselineIn(boxHeight, font) - font.ascender();
}

int Gui::textTopIn(MATHEX_SCALAR boxHeight, const Ren::Font& font)
{
    return baselineIn(boxHeight, font) - font.ascender();
}

void Gui::releaseFontMemory()
{
    getFontCache().clear();
}

/* End **************************************************************/
