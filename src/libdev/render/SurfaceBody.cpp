/*
 * I N S U R F C E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "base/Diag.hpp"

#include "system/PathName.hpp"
#include "render/Font.hpp"
#include "render/Painter.hpp"
#include "render/Surface.hpp"
#include "render/TextOptions.hpp"
#include "render/Texture.hpp"
#include "render/SurfaceManager.hpp"
#include "render/Device.hpp"
#include "render/Display.hpp"
#include "render/internal/DisplayImpl.hpp"
#include "render/internal/SurfaceBody.hpp"
#include "render/internal/IRenderBackend.hpp"
#include "render/internal/FontImpl.hpp"
#include "device/Timer.hpp"
#include "render/internal/DisplayImpl.hpp"
#include <algorithm>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <SDL3_image/SDL_image.h>
#include "render/internal/VertexData.hpp"
#include "render/internal/ColourPack.hpp"
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
        spdlog::error("Failed to load texture from file (path: {}, error: {})", fileName, SDL_GetError());
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

void RenISurfBody::releaseNativeTexture()
{
    // Just clear the handle — the old backend will destroy the GL object when it shuts down.
    nativeTexture2D_ = Ren::BackendTextureHandle{};
}

// virtual
void RenISurfBody::reuploadFromDisk()
{
    if (name_.empty())
        return;

    // Re-read the file and re-upload to the current backend.
    SDL_Surface* surface = readFromFile(name_.c_str());
    if (!surface)
        return;

    if (allocateDDSurfaces(surface->w, surface->h, SYSTEM))
        copyWithColourKeyEmulation(surface, RenColour::magenta());

    SDL_DestroySurface(surface);
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
    SDL_DestroySurface(surface);

    return retval;
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

bool RenISurfBody::copyWithAlpha(SDL_Surface* surface, SDL_Surface* surfaceAlpha, bool createMipmaps)
{
    SDL_Surface* surfaceDst = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ABGR8888);
    SDL_Surface* surfaceTmp = SDL_ConvertSurface(surfaceAlpha, SDL_PIXELFORMAT_BGRA8888);

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
    SDL_DestroySurface(surfaceDst);
    SDL_DestroySurface(surfaceTmp);
    return true;
}

bool RenISurfBody::copyWithColourKeyEmulation(SDL_Surface* surface, const RenColour& keyColour, bool createMipmaps)
{
    // Convert to RGBA and set alpha 0 for key magenta colour
    SDL_Surface* surfaceTmp = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA8888);
    SDL_SetSurfaceColorKey(surfaceTmp, true, SDL_MapSurfaceRGB(surfaceTmp, 0xFF, 0x0, 0xFF));

    SDL_Surface* surfaceDst = SDL_CreateSurface(surface->w, surface->h, SDL_PIXELFORMAT_RGBA32);
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

    SDL_DestroySurface(surfaceTmp);
    SDL_DestroySurface(surfaceDst);
    return true;
}

bool RenISurfBody::copyFromBuffer(const uint* pixelsBuffer)
{
    Ren::IRenderBackend& backend = requireBackend();
    backend.textureSubImage2D(
        nativeTexture2D_, 0, 0, static_cast<int>(width_), static_cast<int>(height_), Ren::TextureFormat::RGBA8_UNorm, pixelsBuffer);
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


/* End INSURFCE.CPP *************************************************/
