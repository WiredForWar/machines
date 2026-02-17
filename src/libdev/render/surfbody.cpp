/*
 * I N S U R F C E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "base/diag.hpp"

#include "system/pathname.hpp"
#include "render/Font.hpp"
#include "render/Painter.hpp"
#include "render/surface.hpp"
#include "render/TextOptions.hpp"
#include "render/texture.hpp"
#include "render/surfmgr.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "render/internal/displayi.hpp"
#include "render/internal/surfbody.hpp"
#include "render/internal/IRenderBackend.hpp"
#include "render/internal/FontImpl.hpp"
#include "device/timer.hpp"
#include <algorithm>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <SDL2/SDL_image.h>
#include "render/internal/vtxdata.hpp"
#include "render/internal/colpack.hpp"
#include "render/internal/RenScopedImmediateCommands.hpp"

#include "spdlog/spdlog.h"

namespace
{
Ren::IRenderBackend& requireBackend()
{
    RenDevice* device = RenDevice::current();
    ASSERT(device, "No active render device available");
    return device->backend();
}

Ren::IRenderBackend* tryBackend()
{
    RenDevice* device = RenDevice::current();
    if (!device)
        return nullptr;
    return &device->backend();
}
} // namespace

RenISurfBody::RenISurfBody()
    : displayType_(RenI::NOT_DISPLAY)
    , width_(0)
    , height_(0)
    , name_("") // NB: look at precondition on name set method.
    , sharedLeaf_(true)
    , readOnly_(true)
    , sharable_(true)
{
    POST(width() == 0 && height() == 0);
    POST(sharable() && readOnly());
    POST(name().length() == 0);
}

RenISurfBody::RenISurfBody(size_t rqWidth, size_t rqHeight, Residence residence)
    : displayType_(RenI::NOT_DISPLAY)
    , width_(0)
    , height_(0)
    , name_("") // NB: look at precondition on name set method.
    , sharedLeaf_(true)
{
    // Initialise UltProperties.
    keyingOn(false);
    keyColour(RenColour::magenta());

    allocateDDSurfaces(rqWidth, rqHeight, residence);

    POST(!sharable() && !readOnly());
    POST(width() == rqWidth && height() == rqHeight);
    POST(name().length() == 0);
}

// This is set up as non-sharable.  If two separate clients ask for display
// surfaces, then the surface manager will allocate two separate bodies.
// However, this ctor does not allocate surface memory, so there isn't a huge
// overhead.
RenISurfBody::RenISurfBody(const RenDevice* dev, RenI::DisplayType type)
    : displayType_(type)
    , device_(dev)
    , name_("") // NB: look at precondition on name set method.
    , sharedLeaf_(true)
{
    PRE(dev);
    PRE(type != RenI::NOT_DISPLAY); // i.e. back or front

    height_ = dev->windowHeight();
    width_ = dev->windowWidth();

    POST(!sharable() && !readOnly());
    POST(name().length() == 0);
    POST(width() > 0 && height() > 0);
}

bool RenISurfBody::allocateDDSurfaces(
    size_t rqWidth,
    size_t rqHeight,
    Residence residence)
{
    Ren::IRenderBackend& backend = requireBackend();

    nativeTexture2D_ = backend.createTexture2D();
    backend.textureSetMinMagFilter(nativeTexture2D_, Ren::TextureFilter::Linear, Ren::TextureFilter::Linear);
    backend.textureSetWrap(nativeTexture2D_, Ren::TextureWrap::Repeat, Ren::TextureWrap::Repeat);
    backend.textureStorage2D(
        nativeTexture2D_, static_cast<int>(rqWidth), static_cast<int>(rqHeight), Ren::TextureFormat::RGBA8_UNorm);

    width_ = rqWidth;
    height_ = rqHeight;
    return true;
}

SDL_Surface* RenISurfBody::readFromFile(const char *fileName)
{
    SDL_Surface* surface = IMG_Load(fileName);
    if (!surface)
    {
        spdlog::error("Failed to load texture from file (path: {}, error: {})", fileName, IMG_GetError());
    }
    return surface;
}

// virtual
RenISurfBody::~RenISurfBody()
{
    // Delete texture
    if (nativeTexture2D_.isValid())
    {
        if (auto* backend = tryBackend())
            backend->destroyTexture2D(nativeTexture2D_);
        nativeTexture2D_ = Ren::BackendTextureHandle{};
    }
}

// virtual
bool RenISurfBody::read(const std::string& bitmapName)
{
    return read(bitmapName, bitmapName);
}

// virtual
bool RenISurfBody::read(const std::string& filePath, const std::string& logicalName)
{
    PRE(filePath.length() > 0);
    PRE(logicalName.length() > 0);

    bool retval = false;
    SDL_Surface* surface = readFromFile(filePath.c_str());
    if (!surface)
        return false;

    if (allocateDDSurfaces(surface->w, surface->h, SYSTEM))
        retval = copyWithColourKeyEmulation(surface, RenColour::magenta());

    name(logicalName);
    SDL_FreeSurface(surface);

    return retval;
}

void RenISurfBody::setDDColourKey()
{
    //  PRE(surface_);

    if (keyingOn())
    {
    }
}

void RenISurfBody::unclippedBlit(const RenISurfBody* source, const Ren::Rect& srcArea, int destX, int destY)
{
    unclippedBlit(source, srcArea, destX, destY, Ren::BlitMode::AlphaBlend);
}

void RenISurfBody::unclippedBlit(
    const RenISurfBody* source,
    const Ren::Rect& srcArea,
    int destX,
    int destY,
    Ren::BlitMode mode)
{
    PRE_INFO(height());
    PRE_INFO(srcArea.height);
    PRE(source);
    PRE(srcArea.width <= width());
    PRE(srcArea.height <= height());
    PRE(destX < width());
    PRE(destY < height());
    PRE(destX + srcArea.width <= width());
    PRE(destY + srcArea.height <= height());

    // If a cursor is displayed, all blits to the display surfaces must be
    // bracketed by start-end frame calls.
    RenDevice* dev = RenDevice::current();
    RenScopedImmediateCommands guard(dev);
    PRE_DATA(const bool displayDest = displayType_ == RenI::FRONT || displayType_ == RenI::BACK);
    PRE(dev);
    PRE(dev->display());
    //  PRE(implies(displayDest && dev->display()->currentCursor(), dev->rendering()));

    Ren::Rect dstArea;
    dstArea.originX = destX;
    dstArea.originY = destY;
    dstArea.width = srcArea.width;
    dstArea.height = srcArea.height;

    //  if( (displayType_ != RenI::FRONT) && (displayType_ != RenI::BACK) )
    if (displayType_ == RenI::NOT_DISPLAY)
    {
        dev->renderToTextureMode(RenSurface::createFromInternal(this).handle(), width_, height_);
        dev->renderSurface(source, srcArea, dstArea, width_, height_, 0xFFFFFFFF, mode);
        dev->renderToTextureMode(Ren::NullTexId, 0, 0);
    }
    else
    {
        dev->renderSurface(source, srcArea, dstArea, 0, 0, 0xFFFFFFFF, mode);
    }
}

void RenISurfBody::unclippedStretchBlit(const RenISurfBody* source, const Ren::Rect& srcArea, const Ren::Rect& dstArea)
{
    unclippedStretchBlit(source, srcArea, dstArea, Ren::BlitMode::AlphaBlend);
}

void RenISurfBody::unclippedStretchBlit(
    const RenISurfBody* source,
    const Ren::Rect& srcArea,
    const Ren::Rect& dstArea,
    Ren::BlitMode mode)
{
    PRE(source);

    // If a cursor is displayed, all blits to the display surfaces must be
    // bracketed by start-end frame calls.
    RenDevice* dev = RenDevice::current();
    RenScopedImmediateCommands guard(dev);
    PRE_DATA(const bool displayDest = displayType_ == RenI::FRONT || displayType_ == RenI::BACK);
    PRE(dev);
    PRE(dev->display());
    // PRE(implies(displayDest && dev->display()->currentCursor(), dev->rendering()));

    if (displayType_ == RenI::NOT_DISPLAY)
    {
        dev->renderToTextureMode(RenSurface::createFromInternal(this).handle(), width_, height_);

        dev->setSmoothScaleEnabled(false);
        dev->renderSurface(source, srcArea, dstArea, width_, height_, 0xFFFFFFFF, mode);
        dev->setSmoothScaleEnabled(true);
        dev->renderToTextureMode(Ren::NullTexId, 0, 0);
    }
    else
    {
        dev->renderSurface(source, srcArea, dstArea, 0, 0, 0xFFFFFFFF, mode);
    }
}

void RenISurfBody::filledRectangle(const Ren::Rect& area, uint colour)
{
    RenDevice* device_ = RenDevice::current();

    PRE(device_);
    PRE(device_->display());
    RenScopedImmediateCommands guard(device_);
    Ren::Rect srcArea;
    srcArea.originX = srcArea.originY = 0;
    srcArea.width = srcArea.height = 1;
    RenISurfBody emptySurf;
    RenDevice* dev = _CONST_CAST(RenDevice*, device_);

    const bool backgroundColour = colour == 0xFFFF00FF;
    const Ren::BlitMode blitMode = backgroundColour ? Ren::BlitMode::ZeroZero : Ren::BlitMode::AlphaBlend;

    if (displayType_ == RenI::NOT_DISPLAY)
    {
        dev->renderToTextureMode(RenSurface::createFromInternal(this).handle(), width_, height_);
        dev->renderSurface(&emptySurf, srcArea, area, width_, height_, colour, blitMode);
        dev->renderToTextureMode(Ren::NullTexId, 0, 0);
    }
    else
    {
        dev->renderSurface(&emptySurf, srcArea, area, 0, 0, colour, blitMode);
    }
}

void RenISurfBody::drawText(
    int x, int y, const std::string_view& text, const Ren::Font& font, const Ren::TextOptions& options)
{
    RenScopedImmediateCommands guard(RenDevice::current());

    struct UnderlineSegment
    {
        int x1{};
        int x2{};
        int y{};
    };

    std::vector<UnderlineSegment> underlineSegments;

    std::vector<RenIVertex> vertices;
    {
        int passes = 1;
        if (options.hasShadow())
        {
            passes += 1;
        }
        if (options.hasOutline())
        {
            const int thickness = options.outlineThickness();
            int outlinePasses = 0;
            for (int ring = 1; ring <= thickness; ++ring)
            {
                outlinePasses += ring * 8;
            }
            passes += outlinePasses;
        }

        int expectedVerticesNumber = text.size() * 6 * passes;
        vertices.reserve(expectedVerticesNumber);
    }

    RenColour col = options.color();
    uint fontColor = packColour(col.r(), col.g(), col.b(), 1.0);

    uint secondColor = 0;

    if (options.hasShadow())
    {
        RenColour unpacked = options.shadowColor();
        secondColor = packColour(unpacked.r(), unpacked.g(), unpacked.b(), 1.0);
    }

    uint outlineColor = 0;
    if (options.hasOutline())
    {
        RenColour unpacked = options.outlineColor();
        outlineColor = packColour(unpacked.r(), unpacked.g(), unpacked.b(), 1.0);
    }

    const Ren::FontImpl& fontImpl = *Ren::FontImpl::get(&font);
    const Ren::FontImpl::CharData* charData = nullptr;

    y += fontImpl.ascender();

    if (options.alignment() & Ren::AlignRight)
    {
        int textWidth = 0;
        int lineTextWidth = 0;
        // Precalc the width
        int usedSpacing = 0;
        for (uint character : text)
        {
            if (character == '\n')
            {
                textWidth = std::max<int>(textWidth, lineTextWidth - usedSpacing);
                usedSpacing = 0;
                continue;
            }

            charData = fontImpl.getChar(character);
            // Ignore missing characters
            if (!charData)
                continue;

            /* Advance the cursor to the start of the next character */
            lineTextWidth += charData->ax + options.letterSpacing();
            usedSpacing = options.letterSpacing();
        }
        textWidth = std::max<int>(textWidth, lineTextWidth - usedSpacing);
        if (options.hasShadow())
        {
            textWidth += options.shadowX();
        }

        x -= textWidth;

        if (x < 0)
        {
            x = 0;
        }
    }

    const int originX = x;

    int lineStartX = originX;
    int lineEndX = originX;
    int lineBaselineY = y;

    x = originX;
    for (int i = 0; i < text.size(); ++i)
    {
        char character = text[i];
        if (character == '\n')
        {
            if (options.underline() && lineEndX != lineStartX)
            {
                underlineSegments.push_back({
                    lineStartX,
                    lineEndX,
                    lineBaselineY - fontImpl.descender() + 1,
                });
            }

            x = originX;
            y += fontImpl.lineHeight() + 2;

            lineStartX = originX;
            lineEndX = originX;
            lineBaselineY = y;
            continue;
        }

        charData = fontImpl.getChar(character);
        // Ignore missing characters
        if (!charData)
            continue;

        int x2 = x + charData->bl;
        int y2 = y - charData->bt;
        int w = charData->bw;
        int h = charData->bh;

        /* Advance the cursor to the start of the next character */
        x += charData->ax + options.letterSpacing();
        y += charData->ay;
        lineEndX = x;

        /* Skip glyphs that have no pixels */
        if (w <= 0 || h <= 0)
            continue;

        // Calculate some common coordinate values
        int x1 = x2 + w;
        int y1 = y2 + h;
        float tu1 = charData->tx;
        float tv1 = charData->ty;
        float tu2 = charData->tx2;
        float tv2 = charData->ty2;

        const auto addVertices
            = [&vertices](uint color, int x1, int x2, int y1, int y2, float tu1, float tu2, float tv1, float tv2) {
            RenIVertex vx;
            vx.color = color;
            vx.z = 0;
            vx.x = x2;
            vx.y = y2;
            vx.tu = tu1;
            vx.tv = tv1;
            vertices.push_back(vx);
            vx.x = x1;
            vx.y = y2;
            vx.tu = tu2;
            vx.tv = tv1;
            vertices.push_back(vx);
            vx.x = x2;
            vx.y = y1;
            vx.tu = tu1;
            vx.tv = tv2;
            vertices.push_back(vx);
            vx.x = x1;
            vx.y = y2;
            vx.tu = tu2;
            vx.tv = tv1;
            vertices.push_back(vx);
            vx.x = x2;
            vx.y = y1;
            vx.tu = tu1;
            vx.tv = tv2;
            vertices.push_back(vx);
            vx.x = x1;
            vx.y = y1;
            vx.tu = tu2;
            vx.tv = tv2;
            vertices.push_back(vx);
        };

        if (options.hasShadow())
        {
            addVertices(
                secondColor,
                x1 + options.shadowX(),
                x2 + options.shadowX(),
                y1 + options.shadowY(),
                y2 + options.shadowY(),
                tu1,
                tu2,
                tv1,
                tv2);
        }

        if (options.hasOutline())
        {
            const int thickness = options.outlineThickness();
            for (int ring = 1; ring <= thickness; ++ring)
            {
                for (int ox = -ring; ox <= ring; ++ox)
                {
                    const int absOx = (ox < 0) ? -ox : ox;
                    for (int oy = -ring; oy <= ring; ++oy)
                    {
                        const int absOy = (oy < 0) ? -oy : oy;
                        const int maxAbs = (absOx > absOy) ? absOx : absOy;
                        if (maxAbs != ring)
                            continue;
                        if (ox == 0 && oy == 0)
                            continue;

                        addVertices(outlineColor, x1 + ox, x2 + ox, y1 + oy, y2 + oy, tu1, tu2, tv1, tv2);
                    }
                }
            }
        }
        addVertices(fontColor, x1, x2, y1, y2, tu1, tu2, tv1, tv2);
    }
    ASSERT(!vertices.empty(), "drawText called with text that produced no glyphs");
    if (vertices.empty())
        return;

    RenDevice::current()->recordCommand(Ren::Command::setCullFace(false));
    RenDevice::current()->renderScreenspace(
        &vertices.front(),
        vertices.size(),
        Ren::PrimitiveTopology::Triangles,
        width_,
        height_,
        fontImpl.textureId);

    if (options.underline() && lineEndX != lineStartX)
    {
        underlineSegments.push_back({
            lineStartX,
            lineEndX,
            lineBaselineY - fontImpl.descender() + 1,
        });
    }

    if (!underlineSegments.empty())
    {
        RenSurface surface = RenSurface::createFromInternal(this);
        Ren::Painter painter(surface);

        for (const UnderlineSegment& seg : underlineSegments)
        {
            painter.line(Ren::Point(seg.x1, seg.y), Ren::Point(seg.x2, seg.y), options.color(), 1);
        }
    }

}

void RenISurfBody::releaseDC()
{
    // Delete texture
    if (nativeTexture2D_.isValid())
    {
        if (auto* backend = tryBackend())
        {
            backend->destroyTexture2D(nativeTexture2D_);
        }
        nativeTexture2D_ = Ren::BackendTextureHandle{};
    }
}

static void computeScaleAndShift(unsigned long bitMask, int& shift, int& scale)
{
    unsigned long m = bitMask;
    int s;
    for (s = 0; !(m & 1); s++)
        m >>= 1;

    shift = s;
    scale = 255 / (bitMask >> shift);
}

bool RenISurfBody::copyWithAlpha(SDL_Surface* surface, SDL_Surface* surfaceAlpha, bool createMipmaps)
{
    SDL_Surface* surfaceDst = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_Surface* surfaceTmp = SDL_ConvertSurfaceFormat(surfaceAlpha, SDL_PIXELFORMAT_BGRA8888, 0);

    Uint32* pixelsDst = (Uint32*)surfaceDst->pixels;
    Uint32* pixelsSrc = (Uint32*)surfaceTmp->pixels;
    for (int y = 0; y < surfaceDst->h; y++)
    {
        for (int x = 0; x < surfaceDst->w; x++)
        {
            Uint32 index = y * surfaceDst->w + x;
            Uint32 pixel = pixelsDst[index] & 0x00FFFFFF;
            pixelsDst[index] = pixel | (pixelsSrc[index] & 0xFF000000);
        }
    }
    Ren::IRenderBackend& backend = requireBackend();
    backend.textureSubImage2D(
        nativeTexture2D_, 0, 0, surfaceDst->w, surfaceDst->h, Ren::TextureFormat::RGBA8_UNorm, surfaceDst->pixels);

    if (createMipmaps && surfaceDst->w > 128 && surfaceDst->h > 128)
    {
        backend.textureGenerateMipmap(nativeTexture2D_);
        backend.textureSetMinMagFilter(
            nativeTexture2D_, Ren::TextureFilter::LinearMipmapLinear, Ren::TextureFilter::LinearMipmapLinear);
    }
    else
    {
        backend.textureSetMinMagFilter(nativeTexture2D_, Ren::TextureFilter::Linear, Ren::TextureFilter::Linear);
    }

    width_ = surface->w;
    height_ = surface->h;
    SDL_FreeSurface(surfaceDst);
    SDL_FreeSurface(surfaceTmp);
    return true;
}

bool RenISurfBody::copyWithColourKeyEmulation(SDL_Surface* surface, const RenColour& keyColour, bool createMipmaps)
{
    // Convert to RGBA and set alpha 0 for key magenta colour
    SDL_Surface* surfaceTmp = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_SetColorKey(surfaceTmp, SDL_TRUE, SDL_MapRGB(surfaceTmp->format, 0xFF, 0x0, 0xFF));

    SDL_Surface* surfaceDst = SDL_CreateRGBSurface(
        SDL_SWSURFACE,
        surface->w,
        surface->h,
        32,
        0x000000ff,
        0x0000ff00,
        0x00ff0000,
        0xff000000);
    SDL_BlitSurface(surfaceTmp, nullptr, surfaceDst, nullptr);

    Ren::IRenderBackend& backend = requireBackend();
    backend.textureSubImage2D(
        nativeTexture2D_, 0, 0, surfaceDst->w, surfaceDst->h, Ren::TextureFormat::RGBA8_UNorm, surfaceDst->pixels);
    if (createMipmaps && surfaceDst->w > 128 && surfaceDst->h > 128)
    {
        backend.textureGenerateMipmap(nativeTexture2D_);
        backend.textureSetMinMagFilter(
            nativeTexture2D_, Ren::TextureFilter::LinearMipmapLinear, Ren::TextureFilter::LinearMipmapLinear);
    }
    else
    {
        backend.textureSetMinMagFilter(nativeTexture2D_, Ren::TextureFilter::Linear, Ren::TextureFilter::Linear);
    }

    width_ = surface->w;
    height_ = surface->h;

    SDL_FreeSurface(surfaceTmp);
    SDL_FreeSurface(surfaceDst);
    return true;
}

bool RenISurfBody::copyFromBuffer(const uint* pixelsBuffer)
{
    Ren::IRenderBackend& backend = requireBackend();
    backend.textureSubImage2D(
        nativeTexture2D_, 0, 0, static_cast<int>(width_), static_cast<int>(height_), Ren::TextureFormat::RGBA8_UNorm, pixelsBuffer);
    return true;
}

bool RenISurfBody::restoreToVRAM() const
{

    return true;
}

bool RenISurfBody::loadIntoVRAM() const
{
    return false;
}

bool RenISurfBody::recreateVRAMSurface()
{
    return true;
}

bool RenISurfBody::isEmpty() const
{
    return width() == 0 || height() == 0;
}

size_t RenISurfBody::memoryUsed() const
{
    // Assume RGBA8 (4 bytes per pixel) for all surfaces.
    return width() * height() * 4;
}

void RenISurfBody::incRefCount()
{
    ++refCount_;
}

void RenISurfBody::decRefCount()
{
    --refCount_;
}

uint RenISurfBody::refCount() const
{
    return refCount_;
}

size_t RenISurfBody::width() const
{
    // return descr_.dwWidth;
    return width_;
}

size_t RenISurfBody::height() const
{
    // return descr_.dwHeight;
    return height_;
}

Ren::Size RenISurfBody::size() const
{
    return Ren::Size(width_, height_);
}

const std::string& RenISurfBody::sharedName() const
{
    if (sharedLeaf_)
        return leafName_;
    else
        return name_;
}

void RenISurfBody::shareLeafName(bool shared)
{
    sharedLeaf_ = shared;
}

const std::string& RenISurfBody::name() const
{
    return name_;
}

void RenISurfBody::name(const std::string& n)
{
    PRE(name().length() == 0);
    name_ = n;

    // Note : original code below is inefficient ( probably more portable though ).
    // const SysPathName pathName = n;
    const SysPathName pathName(n);
    ASSERT(pathName.components().size() > 0, "");
    leafName_ = pathName.components().back();

    // Above, inefficient code, restored due to change in SysPathName breaking following code...
    // If the given string is a pathname, set leafName_ to be the last component.
    // char* leafName = strrchr( name_.c_str(), '/' );
    // if ( leafName )
    //{
    //  leafName_ = ++leafName;
    //}
    // else
    //{
    //  leafName_ = name_;
    //}
}

bool RenISurfBody::sharable() const
{
    return sharable_;
}

bool RenISurfBody::readOnly() const
{
    return readOnly_;
}

void RenISurfBody::makeReadOnlySharable()
{
    // You can't change this property on a shared surface, however, if it's
    // already read-only, then it doesn't make any difference.
    PRE(readOnly() || !sharable());

    readOnly_ = true;
    sharable_ = true;

    POST(sharable() && readOnly());
}

// virtual
RenITexBody* RenISurfBody::castToTexBody()
{
    return nullptr;
}

// virtual
const RenITexBody* RenISurfBody::castToTexBody() const
{
    return nullptr;
}

// virtual
void RenISurfBody::print(std::ostream& o) const
{
    o << "surface ";

    if (name().length() > 0)
        o << name() << " ";

    switch (displayType_)
    {
        case RenI::BACK:
            o << "back-buffer ";
            break;
        case RenI::FRONT:
            o << "front-buffer ";
            break;
    }

    o << "(" << width() << "x" << height() << ")";
}

bool RenISurfBody::matches(const std::string& name) const
{
    return strcasecmp(sharedName().c_str(), name.c_str()) == 0;
}

std::ostream& operator<<(std::ostream& o, const RenISurfBody& t)
{
    t.print(o);
    return o;
}

void RenISurfBody::updateDescr()
{
}

/* End INSURFCE.CPP *************************************************/
