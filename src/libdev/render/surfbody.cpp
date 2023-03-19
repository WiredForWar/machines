/*
 * I N S U R F C E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/internal/ren_pch.hpp" // NB: pre-compiled header must come 1st

#include "base/diag.hpp"

#include "system/pathname.hpp"
#include "render/Font.hpp"
#include "render/texture.hpp"
#include "render/surfmgr.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "render/internal/displayi.hpp"
#include "render/internal/surfbody.hpp"
#include "render/internal/FontImpl.hpp"
#include "device/timer.hpp"
#include <algorithm>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <SDL2/SDL_image.h>
#include "render/internal/vtxdata.hpp"
#include "render/internal/colpack.hpp"

RenISurfBody::RenISurfBody()
    : displayType_(RenI::NOT_DISPLAY)
    , refCount_(0)
    , textureID_(0)
    , width_(0)
    , height_(0)
    , loaded_(false)
    , readOnly_(true)
    , sharable_(true)
    , currentHeight_(0)
    , device_(nullptr)
    , name_("")
    , // NB: look at precondition on name set method.
    sharedLeaf_(true)
{
    POST(width() == 0 && height() == 0);
    POST(sharable() && readOnly());
    POST(name().length() == 0);
}

RenISurfBody::RenISurfBody(size_t rqWidth, size_t rqHeight, const RenIPixelFormat& format, Residence residence)
    : displayType_(RenI::NOT_DISPLAY)
    , refCount_(0)
    , textureID_(0)
    , width_(0)
    , height_(0)
    , loaded_(false)
    , readOnly_(false)
    , sharable_(false)
    , currentHeight_(0)
    , device_(nullptr)
    , name_("")
    , // NB: look at precondition on name set method.
    sharedLeaf_(true)
{
    // Initialise UltProperties.
    keyingOn(false);
    keyColour(RenColour::magenta());

    allocateDDSurfaces(rqWidth, rqHeight, format, residence);

    POST(!sharable() && !readOnly());
    POST(width() == rqWidth && height() == rqHeight);
    //  POST(pixelFormat_.isValid());           // TBD: write operator==
    POST(name().length() == 0);
}

RenISurfBody::RenISurfBody(const RenIPixelFormat& format)
    : displayType_(RenI::NOT_DISPLAY)
    , pixelFormat_(format)
    , refCount_(0)
    , textureID_(0)
    , width_(0)
    , height_(0)
    , loaded_(false)
    , readOnly_(false)
    , sharable_(false)
    , currentHeight_(0)
    , device_(nullptr)
    , name_("")
    , // NB: look at precondition on name set method.
    sharedLeaf_(true)
{
    // Initialise UltProperties.
    keyingOn(false);
    keyColour(RenColour::magenta());

    POST(width() == 0 && height() == 0);
    POST(!sharable() && !readOnly());
    //  POST(pixelFormat_.isValid());           // TBD: write operator==
    POST(name().length() == 0);
}

// This is set up as non-sharable.  If two separate clients ask for display
// surfaces, then the surface manager will allocate two separate bodies.
// However, this ctor does not allocate surface memory, so there isn't a huge
// overhead.
RenISurfBody::RenISurfBody(const RenDevice* dev, RenI::DisplayType type)
    : displayType_(type)
    , device_(dev)
    , refCount_(0)
    , loaded_(false)
    , readOnly_(false)
    , sharable_(false)
    , currentHeight_(0)
    , name_("")
    , // NB: look at precondition on name set method.
    sharedLeaf_(true)
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
    const RenIPixelFormat& format,
    Residence residence)
{
    glGenTextures(1, &textureID_);
    glBindTexture(GL_TEXTURE_2D, textureID_);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(
        GL_TEXTURE_2D, // target
        0, // level, 0 = base, no minimap,
        GL_RGBA, // internalformat
        rqWidth, // width
        rqHeight, // height
        0, // border, always 0 in OpenGL ES
        GL_RGBA, // format
        GL_UNSIGNED_BYTE, // type, /GL_UNSIGNED_BYTE
        (GLvoid*)nullptr);

    width_ = rqWidth;
    height_ = rqHeight;
    return true;
}

// virtual
RenISurfBody::~RenISurfBody()
{
    // Delete texture
    if (textureID_ != NULL)
    {
        glDeleteTextures(1, &textureID_);
        textureID_ = NULL;
    }
}

// virtual
bool RenISurfBody::read(const std::string& bitmapName)
{
    PRE(bitmapName.length() > 0);
    //  PRE(pixelFormat_.isValid());    // Use the ctor which initialises the format.

    bool retval = false;
    SDL_Surface* surface = IMG_Load(bitmapName.c_str());
    if (!surface)
    {
        RENDER_STREAM("Failed to load surface from file " << SDL_GetError() << std::endl);
        return false;
    }
    else
    {
        if (allocateDDSurfaces(surface->w, surface->h, pixelFormat_, SYSTEM))
            retval = copyWithColourKeyEmulation(surface, RenColour::magenta());

        name(bitmapName);
        SDL_FreeSurface(surface);
    }

    return retval;
}

void RenISurfBody::setDDColourKey()
{
    //  PRE(surface_);

    if (keyingOn())
    {
    }
}

static char* formatMsg()
{
    char* msgBuf = nullptr;

    return msgBuf;
}

void RenISurfBody::unclippedBlit(const RenISurfBody* source, const Ren::Rect& srcArea, int destX, int destY)
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
        dev->renderToTextureMode(textureID_, width_, height_);
        dev->renderSurface(source, srcArea, dstArea, width_, height_);
        dev->renderToTextureMode(0, 0, 0);
    }
    else
    {
        dev->renderSurface(source, srcArea, dstArea);
    }
}

void RenISurfBody::unclippedStretchBlit(const RenISurfBody* source, const Ren::Rect& srcArea, const Ren::Rect& dstArea)
{
    PRE(source);

    // If a cursor is displayed, all blits to the display surfaces must be
    // bracketed by start-end frame calls.
    RenDevice* dev = RenDevice::current();
    PRE_DATA(const bool displayDest = displayType_ == RenI::FRONT || displayType_ == RenI::BACK);
    PRE(dev);
    PRE(dev->display());
    // PRE(implies(displayDest && dev->display()->currentCursor(), dev->rendering()));

    if (displayType_ == RenI::NOT_DISPLAY)
    {
        dev->renderToTextureMode(textureID_, width_, height_);
        dev->renderSurface(source, srcArea, dstArea, width_, height_);
        dev->renderToTextureMode(0, 0, 0);
    }
    else
    {
        dev->renderSurface(source, srcArea, dstArea);
    }
}

void RenISurfBody::filledRectangle(const Ren::Rect& area, uint colour)
{
    RenDevice* device_ = RenDevice::current();

    PRE(device_);
    PRE(device_->display());
    Ren::Rect srcArea;
    srcArea.originX = srcArea.originY = 0;
    srcArea.width = srcArea.height = 1;
    RenISurfBody emptySurf;
    RenDevice* dev = _CONST_CAST(RenDevice*, device_);

    if (displayType_ == RenI::NOT_DISPLAY)
    {
        dev->renderToTextureMode(textureID_, width_, height_);
        if (colour == 0xFFFF00FF) // Handle background colour
        {
            GLint blendSrc, blendDst;
            glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
            glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
            glBlendFunc(GL_ZERO, GL_ZERO);
            dev->renderSurface(&emptySurf, srcArea, area, width_, height_, colour);
            glBlendFunc(blendSrc, blendDst);
        }
        else
            dev->renderSurface(&emptySurf, srcArea, area, width_, height_, colour);
        dev->renderToTextureMode(0, 0, 0);
    }
    else
    {
        dev->renderSurface(&emptySurf, srcArea, area, 0, 0, colour);
    }
}

// static
const std::string& RenISurfBody::fontName()
{
    return Render::Font::getDefaultFontName();
}

// static
size_t RenISurfBody::defaultHeight()
{
    return RenSurface::getDefaultFontHeight();
}

size_t RenISurfBody::useFontHeight(size_t pixelHeight)
{
    // Decrease to fit in game scale
    pixelHeight -= 2;

    if (currentHeight_ == 0)
        currentHeight_ = pixelHeight;

    // Set this whatever results are
    currentHeight_ = pixelHeight;

    pCurrentFont_ = Render::Font::getFont(fontName(), pixelHeight);

    return pixelHeight;
}

size_t RenISurfBody::currentFontHeight() const
{
    if (pCurrentFont_)
        return pCurrentFont_->pixelSize();
    else
        return _CONST_CAST(RenISurfBody*, this)->useFontHeight(defaultHeight());
}

void RenISurfBody::drawText(int x, int y, const std::string& text, const RenColour& col)
{
    if (currentHeight_ == 0)
        useFontHeight(defaultHeight());

    if (pCurrentFont_)
    {
        int originX = x;
        std::vector<RenIVertex> vertices;
        vertices.reserve(text.size() * 6);
        uint fontColor = packColour(col.r(), col.g(), col.b(), 1.0);
        y += currentHeight_;
        const Render::FontImpl& font = *Render::FontImpl::get(pCurrentFont_);
        const Render::FontImpl::CharData* charData = nullptr;

        for (int i = 0; i < text.size(); ++i)
        {
            uint character = text[i];
            if (character == '\n')
            {
                x = originX;
                y += currentHeight_ + 2;
                continue;
            }

            charData = font.getChar(character);
            // Ignore missing characters
            if (!charData)
                continue;

            float x2 = x + charData->bl;
            float y2 = y - charData->bt;
            float w = charData->bw;
            float h = charData->bh;

            /* Advance the cursor to the start of the next character */
            x += charData->ax;
            y += charData->ay;

            /* Skip glyphs that have no pixels */
            if (w <= 0 || h <= 0)
                continue;

            RenIVertex vx;
            vx.color = fontColor;
            vx.z = 0;
            // Calculate some common coordinate values
            float x1 = x2 + w, y1 = y2 + h;
            float tu1 = charData->tx, tv1 = charData->ty;
            float tu2 = charData->tx2, tv2 = charData->ty2;
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
        }
        glDisable(GL_CULL_FACE);
        RenDevice::current()
            ->renderScreenspace(&vertices.front(), vertices.size(), GL_TRIANGLES, width_, height_, font.textureId);
        glEnable(GL_CULL_FACE);
    }
}

void RenISurfBody::textDimensions(const std::string& text, Ren::Rect* dimensions) const
{
    PRE(dimensions);

    if (currentHeight_ == 0)
        _CONST_CAST(RenISurfBody*, this)->useFontHeight(defaultHeight());

    ASSERT(currentHeight_ > 0, "Failed to create default font.");
    const Render::Font* pFont = pCurrentFont_;

    ASSERT(pFont && pFont->isValid(), "Failed to get valid font.");

    std::cerr << "RenISurfBody::textDimensions(): NOT IMPLEMENTED" << std::endl;
}

void RenISurfBody::releaseDC()
{
    // Delete texture
    if (textureID_ != NULL)
    {
        glDeleteTextures(1, &textureID_);
        textureID_ = NULL;
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
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        surfaceDst->w,
        surfaceDst->h,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        surfaceDst->pixels);
    if (createMipmaps && surfaceDst->w > 128 && surfaceDst->h > 128)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    width_ = surface->w;
    height_ = surface->h;

    // unbind
    glBindTexture(GL_TEXTURE_2D, NULL);
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

    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        surfaceDst->w,
        surfaceDst->h,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        surfaceDst->pixels);
    if (createMipmaps && surfaceDst->w > 128 && surfaceDst->h > 128)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    width_ = surface->w;
    height_ = surface->h;

    // unbind
    glBindTexture(GL_TEXTURE_2D, NULL);

    SDL_FreeSurface(surfaceTmp);
    SDL_FreeSurface(surfaceDst);
    return true;
}

bool RenISurfBody::copyFromBuffer(const uint* pixelsBuffer)
{
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, pixelsBuffer);

    // unbind
    glBindTexture(GL_TEXTURE_2D, NULL);
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
    // Conceivably, this might not be a 100% accurate figure depending on how
    // the bits are packed.  (The texture could even use compressed storage.)
    return (width() * height() * bitDepth()) / 8;
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

const RenIPixelFormat& RenISurfBody::pixelFormat() const
{
    return pixelFormat_;
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

size_t RenISurfBody::bitDepth() const
{
    return pixelFormat().totalDepth();
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
void RenISurfBody::print(ostream& o) const
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

    o << "(" << width() << "x" << height() << "x" << bitDepth() << ")";
}

bool RenISurfBody::matches(const std::string& name) const
{
    return strcasecmp(sharedName().c_str(), name.c_str()) == 0;
}

ostream& operator<<(ostream& o, const RenISurfBody& t)
{
    t.print(o);
    return o;
}

void RenISurfBody::updateDescr()
{
}

/* End INSURFCE.CPP *************************************************/
