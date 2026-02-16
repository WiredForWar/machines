/*
 * B L I T A B L E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/surface.hpp"

#include <string>
#include "base/diag.hpp"

#include "ctl/vector.hpp"
#include "system/pathname.hpp"
#include "render/Font.hpp"
#include "render/surfmgr.hpp"
#include "render/internal/surfbody.hpp"
#include "render/internal/surfmgri.hpp"
#include "render/internal/colpack.hpp"
#include "render/internal/vtxdata.hpp"
#include "render/device.hpp"
#include "render/internal/RenScopedImmediateCommands.hpp"

#include <SDL2/SDL_image.h>

#include "render/internal/IRenderBackend.hpp"

#include <stdio.h>

static int sDefaultFontSize{10};

//--------------------------------Creation & destruction--------------------------------
// static
RenSurface RenSurface::createAnonymousSurface(Size size, const RenSurface& surf)
{
    // This is pure delegation -- let the manager do the pre and post-conditions.
    return RenSurfaceManager::instance().createAnonymousSurface(size.width, size.height, surf);
}

// Static.
RenSurface RenSurface::createAnonymousVideoSurface(Size size, const RenSurface& pixelFmt)
{
    // The pre and post-conditions are in the surface manager.
    // PRE(!pixelFmt.isNull());
    // PRE(width > 0 and height > 0);

    return RenSurfaceManager::instance().createAnonymousVideoSurface(size.width, size.height, pixelFmt);

    // POST(!result.sharable() and !result.readOnly());
    // POST(name.length() == 0);
}

// static
RenSurface RenSurface::createAnonymousSurface(PerIstream& inStream, const RenSurface& surf)
{
    size_t w, h;

    PER_READ_RAW_OBJECT(inStream, w);
    PER_READ_RAW_OBJECT(inStream, h);

    // This is pure delegation -- let the manager do the pre and post-conditions.
    RenSurface retVal = RenSurfaceManager::instance().createAnonymousSurface(w, h, surf);

    retVal.read(inStream);

    return retVal;
}

// static
WEAK_SYMBOL RenSurface RenSurface::createSharedSurface(const std::string& bitmapName, const RenSurface& surf)
{
    // This is pure delegation -- let the manager do the pre and post-conditions.
    return RenSurfaceManager::instance().createSharedSurface(bitmapName, surf);
}

// static
RenSurface RenSurface::createFromInternal(RenISurfBody* body)
{
    return RenSurface(RenSurfaceManager::instance().impl().getBodyId(body));
}

// static
const RenSurface RenSurface::createFromInternal(const RenISurfBody* body)
{
    return RenSurface(RenSurfaceManager::instance().impl().getBodyId(body));
}

RenSurface::RenSurface(Ren::TexId id)
    : myId_(id)
{
    PRE(Ren::initialised());
    RenSurface::mgrIncrementRefCount(myId_);
    TEST_INVARIANT;
}

RenSurface::RenSurface()
    : myId_(Ren::NullTexId)
{
    PRE(Ren::initialised());
    RenSurface::mgrIncrementRefCount(myId_);
    TEST_INVARIANT;
    POST(isNull() && sharable() && readOnly());
}

RenSurface::RenSurface(const RenSurface& tex)
    : myId_(tex.myId_)
{
    RenSurface::mgrIncrementRefCount(myId_);
    TEST_INVARIANT;
}

RenSurface& RenSurface::operator=(const RenSurface& rhs)
{
    if (*this != rhs)
    {
        RenSurface::mgrDecrementRefCount(myId_);
        myId_ = rhs.myId_;
        RenSurface::mgrIncrementRefCount(myId_);
    }

    return *this;
}

RenSurface::~RenSurface()
{
    TEST_INVARIANT;
    RenSurface::mgrDecrementRefCount(myId_);
}

//------------------------------------Blitting---------------------------------------
void RenSurface::simpleBlit(
    const RenSurface& source, const std::optional<Rect>& srcArea, Point dest, Ren::BlitMode mode)
{
    PRE_INFO(*this);
    PRE(!readOnly());
    PRE(!isEmpty());
    PRE(!source.isEmpty());

    RENDER_STREAM("Simple blit before clipping:\n");
    if (srcArea.has_value())
        RENDER_STREAM("  from " << *srcArea << " of " << source << "\n");
    else
        RENDER_STREAM("  from all of " << source << "\n");
    RENDER_STREAM("  to (" << dest.x << "," << dest.y << ") of " << (*this) << "\n");

    const int srcW = source.width();
    const int srcH = source.height();
    const int intW = _STATIC_CAST(int, width());
    const int intH = _STATIC_CAST(int, height());

    if (srcArea.has_value())
    {
        if (srcArea->originX >= srcW || srcArea->originY >= srcH)
            return;

        // Check to see if the destination area is completely outside this surface.
        if (dest.x > intW || dest.y > intH || dest.x + srcArea->width <= 0 || dest.y + srcArea->height <= 0)
            return;
    }
    else
    {
        // Check to see if the destination area is completely outside this surface.
        if (dest.x > intW || dest.y > intH || dest.x + srcW <= 0 || dest.y + srcH <= 0)
            return;
    }

    // Clip the source rectangle so that the blit will lie in the destination.
    Rect tmp = srcArea.value_or(source.size());
    if (dest.x < 0)
    {
        tmp.originX -= dest.x;
        tmp.width += dest.x;
        dest.x = 0;
    }

    if (dest.y < 0)
    {
        tmp.originY -= dest.y;
        tmp.height += dest.y;
        dest.y = 0;
    }

    // Again, clip against the dest rectangle, this time for the maximum coords.
    const int rightMost = dest.x + tmp.width, bottomMost = dest.y + tmp.height;

    if (rightMost > intW)
        tmp.width -= (rightMost - intW);

    if (bottomMost > intH)
        tmp.height -= (bottomMost - intH);

    if (srcArea)
    {
        // Clip the source area so that it lies within the source surface.
        if (tmp.originX < 0)
        {
            tmp.width += tmp.originX;
            dest.x -= tmp.originX;
            tmp.originX = 0;
        }

        if (tmp.originY < 0)
        {
            tmp.height += tmp.originY;
            dest.y -= tmp.originY;
            tmp.originY = 0;
        }

        const int overshootRight = tmp.originX + tmp.width - srcW;
        if (overshootRight > 0)
        {
            tmp.width -= overshootRight;
        }

        const int overshootBottom = tmp.originY + tmp.height - srcH;
        if (overshootBottom > 0)
        {
            tmp.height -= overshootBottom;
        }
    }
    else
    {
        // Again, clip against the dest rectangle, this time for the maximum coords.
        const int rightMost = dest.x + tmp.width;
        const int bottomMost = dest.y + tmp.height;

        if (rightMost > intW)
        {
            tmp.width -= (rightMost - intW);
        }

        if (bottomMost > intH)
        {
            tmp.height -= (bottomMost - intH);
        }
    }

    if (tmp.width <= 0 || tmp.height <= 0)
        return;

    ASSERT(tmp.originX >= 0, "Clipping logic error.");
    ASSERT(tmp.originY >= 0, "Clipping logic error.");
    ASSERT(tmp.originX + tmp.width <= srcW, "Clipping logic error.");
    ASSERT(tmp.originY + tmp.height <= srcH, "Clipping logic error.");

    internals()->unclippedBlit(source.internals(), tmp, dest.x, dest.y, mode);
}

void RenSurface::tileBlit(const RenSurface& source, const Rect& srcArea, const Rect& destArea)
{
    PRE(!readOnly());
    PRE(srcArea.width <= destArea.width);
    PRE(srcArea.height <= destArea.height);
    PRE(srcArea.width <= source.width());
    PRE(srcArea.height <= source.height());
    PRE(destArea.width <= width());
    PRE(destArea.height <= height());
    PRE(destArea.originX + destArea.width <= width());
    PRE(destArea.originY + destArea.height <= height());

    // First copy however many whole repeats of the source are necessary.
    const int wholeXTiles = destArea.width / srcArea.width;
    const int wholeYTiles = destArea.height / srcArea.height;

    int destX = destArea.originX, destY = destArea.originY;

    for (int x = 0; x != wholeXTiles; ++x)
    {
        for (int y = 0; y != wholeYTiles; ++y)
        {
            internals()->unclippedBlit(source.internals(), srcArea, destX, destY);
            destY += srcArea.height;
        }

        destX += srcArea.width;
        destY = destArea.originY;
    }

    // Partial tiles may be needed at the right and bottom edges of the dest.
    const int extraX = destArea.width - (wholeXTiles * srcArea.width);
    const int extraY = destArea.height - (wholeYTiles * srcArea.height);
    ASSERT(extraX >= 0, logic_error());
    ASSERT(extraY >= 0, logic_error());

    // First, do any partial tiles down the right-hand edge.
    if (extraX > 0)
    {
        int destX = destArea.originX + wholeXTiles * srcArea.width;
        int destY = destArea.originY;

        Rect subArea = srcArea;
        subArea.width = extraX;

        for (int y = 0; y != wholeYTiles; ++y)
        {
            internals()->unclippedBlit(source.internals(), subArea, destX, destY);
            destY += srcArea.height;
        }
    }

    // Then, do any partial tiles along the bottom-hand edge.
    if (extraY > 0)
    {
        int destX = destArea.originX;
        int destY = destArea.originY + wholeYTiles * srcArea.height;

        Rect subArea = srcArea;
        subArea.height = extraY;

        for (int x = 0; x != wholeXTiles; ++x)
        {
            internals()->unclippedBlit(source.internals(), subArea, destX, destY);
            destX += srcArea.width;
        }
    }

    // Finally, fill in the bottom-right corner, if necessary.
    if (extraX > 0 && extraY > 0)
    {
        int destX = destArea.originX + wholeXTiles * srcArea.width;
        int destY = destArea.originY + wholeYTiles * srcArea.height;

        Rect subArea = srcArea;
        subArea.width = extraX;
        subArea.height = extraY;

        internals()->unclippedBlit(source.internals(), subArea, destX, destY);
    }
}

inline bool surfaceContainsRect(const RenSurface& surf, const Ren::Rect& rect)
{
    return rect.originX >= 0 && rect.originY >= 0 && rect.originX + rect.width <= surf.width()
        && rect.originY + rect.height <= surf.height();
}

void RenSurface::stretchBlit(const RenSurface& source, const Rect& srcArea, const Rect& destArea, Ren::BlitMode mode)
{
    PRE(!readOnly());

    // Unlike simple blit, clipping the areas for a stretch blit would be very
    // complicated (you'd have to muck about with ratios and so on).  So use
    // pre-conditions to ensure that the rectangles are within bounds.
    PRE(surfaceContainsRect(source, srcArea));
    PRE(surfaceContainsRect(*this, destArea));

    internals()->unclippedStretchBlit(source.internals(), srcArea, destArea, mode);
}

void RenSurface::stretchBlit(const RenSurface& source, Ren::BlitMode mode)
{
    stretchBlit(source, source.size(), size(), mode);
}

void RenSurface::blitInRequestedSize(const RenSurface& source, Point dest, Ren::BlitMode mode)
{
    if (source.requestedSize().isNull())
    {
        simpleBlit(source, {}, dest, mode);
    }
    else
    {
        stretchBlit(source, source.size(), { dest, source.requestedSize() }, mode);
    }
}

void RenSurface::copyFromRGBABuffer(const uint* buff)
{
    internals()->copyFromBuffer(buff);
}

//--------------------------------2D Drawing Primitives-----------------------------
void RenSurface::filledRectangle(const Rect& area, const RenColour& colour)
{
    PRE(!readOnly());

    if (internals())
        internals()->filledRectangle(area, packColour(colour.r(), colour.g(), colour.b(), colour.a()));
}

void RenSurface::hollowRectangle(const Ren::Rect& area, const RenColour& col, int thickness)
{
    PRE(!readOnly());
    PRE(thickness > 0);

    Ren::Rect orderedArea = area;

    if (orderedArea.width < 0)
    {
        orderedArea.originX += orderedArea.width;
        orderedArea.width = -orderedArea.width;
    }

    if (orderedArea.height < 0)
    {
        orderedArea.originY += orderedArea.height;
        orderedArea.height = -orderedArea.height;
    }

    const int x = orderedArea.originX;
    const int y = orderedArea.originY;
    const int w = orderedArea.width;
    const int h = orderedArea.height;
    const int t = thickness;

    // Top
    filledRectangle(Ren::Rect(x, y, w, t), col);
    // Bottom
    filledRectangle(Ren::Rect(x, y + h - t, w, t), col);
    // Left
    filledRectangle(Ren::Rect(x, y + t, t, h - 2 * t), col);
    // Right
    filledRectangle(Ren::Rect(x + w - t, y + t, t, h - 2 * t), col);
}

void RenSurface::getPixel(int x, int y, RenColour* colour) const
{
    PRE(colour);
    RenDevice* dev = RenDevice::current();

    float pixel[4] = { 0, 0, 0, 0 };
    if (internals() && internals()->isOffscreen())
    {
        // Bind the offscreen FBO, read back, then unbind.  Each step
        // uses its own immediate command buffer so the FBO bind is
        // actually executed before the direct readPixelsFloat call.
        dev->beginImmediateCommands();
        dev->renderToTextureMode(handle(), width(), height());
        dev->endImmediateCommands();

        dev->backend().readPixelsFloat(x, y, 1, 1, pixel);

        dev->beginImmediateCommands();
        dev->renderToTextureMode(Ren::NullTexId, 0, 0);
        dev->endImmediateCommands();
    }
    else
        dev->backend().readPixelsFloat(x, y, 1, 1, pixel);

    colour->r(pixel[0]);
    colour->g(pixel[1]);
    colour->b(pixel[2]);
    colour->a(pixel[3]);
}

void RenSurface::setPixel(int x, int y, const RenColour& colour)
{
    PRE(!readOnly());
    ASSERT_FAIL("Not implemented.");
}

void RenSurface::polyLine(const Points& pts, const RenColour& colour, int thickness)
{
    PRE(!readOnly());
    PRE(pts.size() > 1);
    PRE(thickness > 0);
    RenScopedImmediateCommands guard(RenDevice::current());

    static size_t nVertices = 30;
    static std::vector<RenIVertex> vtx = std::vector<RenIVertex>(nVertices);
    static bool initialised = false;

    if (nVertices < pts.size())
    {
        nVertices = pts.size() + 10;
        vtx = std::vector<RenIVertex>(nVertices);
        initialised = false;
    }

    if (! initialised)
    {
        initialised = true;

        for (size_t i = 0; i != nVertices; ++i)
        {
            vtx[i].z = 0.0;
            vtx[i].specular = 0;
            vtx[i].w = 0.1;
            vtx[i].tu = vtx[i].tv = 0.1;
        }
    }

    uint packedColour = packColour(colour.r(), colour.g(), colour.b(), colour.a());

    for (size_t i = 0; i < pts.size(); ++i)
    {
        vtx[i].x = static_cast<int>(pts[i].x());
        vtx[i].y = static_cast<int>(pts[i].y());
        vtx[i].color = packedColour;
    }

    RenDevice* dev = RenDevice::current();
    dev->recordCommand(Ren::Command::setLineWidth(static_cast<float>(thickness)));
    if (internals() && internals()->isOffscreen())
    {
        dev->renderToTextureMode(handle(), width(), height());
        dev->renderScreenspace(vtx.data(), pts.size(), Ren::PrimitiveTopology::LineStrip, width(), -height());
        dev->renderToTextureMode(Ren::NullTexId, 0, 0);
    }
    else
        dev->renderScreenspace(vtx.data(), pts.size(), Ren::PrimitiveTopology::LineStrip, width(), height());
    dev->recordCommand(Ren::Command::setLineWidth(1.0f));
}

int RenSurface::getDefaultFontSize()
{
    return sDefaultFontSize;
}

void RenSurface::setDefaultFontSize(int size)
{
    sDefaultFontSize = size;
}

void RenSurface::drawText(
    int x, int y, const std::string_view& text, const Ren::Font& font, const Ren::TextOptions& options)
{
    internals()->drawText(x, y, text, font, options);
}

//-----------------------------Simple properties & delegations-----------------------
bool RenSurface::isEmpty() const
{
    return width() == 0 || height() == 0;
}

bool RenSurface::isColourKeyingOn() const
{
    return (isNull()) ? false : internals()->keyingOn();
}

void RenSurface::enableColourKeying()
{
    if (!internals()->keyingOn())
    {
        internals()->keyingOn(true);
        internals()->setDDColourKey();
    }
}

void RenSurface::disableColourKeying()
{
    internals()->keyingOn(false);
}

const RenColour& RenSurface::colourKey() const
{
    return internals()->keyColour();
}

void RenSurface::colourKey(const RenColour& c)
{
    internals()->keyColour(c);
    internals()->setDDColourKey();
}

RenISurfBody* RenSurface::internals()
{
    RenISurfBody* b = RenSurfaceManager::instance().impl().getSurface(myId_);
    ASSERT(b, "NULL internal surface.");
    return b;
}

const RenISurfBody* RenSurface::internals() const
{
    const RenISurfBody* b = RenSurfaceManager::instance().impl().getSurface(myId_);
    ASSERT(b, "NULL internal surface.");
    return b;
}

void RenSurface::makeReadOnlySharable()
{
    internals()->makeReadOnlySharable();
}

void RenSurface::name(const std::string& n)
{
    internals()->name(n);
}

bool RenSurface::isNull() const
{
    return myId_ == Ren::NullTexId;
}

void RenSurface::reset()
{
    swap(RenSurface());
}

Ren::TexId RenSurface::handle() const
{
    return myId_;
}

RenSurface::Size RenSurface::requestedSize() const
{
    return internals()->requestedSize();
}

void RenSurface::setRequestedSize(Size size)
{
    return internals()->setRequestedSize(size);
}

// Several methods are delegated to RenISurfBody using these macros.
#define TEX_FORWARD_GET(RETTYPE, METHOD)                                                                               \
    RETTYPE RenSurface::METHOD() const                                                                                 \
    {                                                                                                                  \
        TEST_INVARIANT;                                                                                                \
        return RenSurfaceManager::instance().impl().getSurface(myId_)->METHOD();                                       \
    }

TEX_FORWARD_GET(const std::string&, name)
TEX_FORWARD_GET(bool, sharable)
TEX_FORWARD_GET(bool, readOnly)
TEX_FORWARD_GET(size_t, width)
TEX_FORWARD_GET(size_t, height)
TEX_FORWARD_GET(RenSurface::Size, size)

//---------------------------------Canonnical stuff--------------------------------
bool operator==(const RenSurface& s1, const RenSurface& s2)
{
    return s1.myId_ == s2.myId_;
}

bool operator!=(const RenSurface& s1, const RenSurface& s2)
{
    return s1.myId_ != s2.myId_;
}

void RenSurface::CLASS_INVARIANT
{
    INVARIANT(internals());
    INVARIANT(internals()->refCount() > 0);
}

std::ostream& operator<<(std::ostream& o, const RenSurface& t)
{
    if (t.isNull())
        o << "(no surface)";
    else
        o << *t.internals();

    return o;
}

void RenSurface::saveAsPng(const SysPathName& filename, const Rect& area) const
{
    TEST_INVARIANT;

    // Save the screen shot
    unsigned char* screenPixels = _NEW_ARRAY(unsigned char, width() * height() * 4);
    if (screenPixels)
    {
        // Read the pixels
        RenDevice* dev = RenDevice::current();
        if (internals() && internals()->isOffscreen())
        {
            dev->renderToTextureMode(handle(), width(), height());
            dev->backend().readPixelsUByte(0, 0, width(), height(), screenPixels);
            dev->renderToTextureMode(Ren::NullTexId, 0, 0);
        }
        else
            dev->backend().readPixelsUByte(0, 0, width(), height(), screenPixels);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
            screenPixels,
            width(),
            height(),
            32,
            width() * 4,
            0xff000000,
            0x00ff0000,
            0x0000ff00,
            0x000000ff);
#else
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
            screenPixels,
            width(),
            height(),
            32,
            width() * 4,
            0x000000ff,
            0x0000ff00,
            0x00ff0000,
            0xff000000);
#endif

        // Flip surface vertically because of OpenGL coordinates...
        // Code comes from https://halfgeek.org/wiki/Vertically_invert_a_surface_in_SDL
        Uint8* t;
        Uint8 *a, *b;
        Uint8* last;
        Uint16 pitch;

        /* get a place to store a line */
        pitch = surface->pitch;
        t = (Uint8*)malloc(pitch);

        if (t == nullptr)
        {
            // mem error
        }

        /* get first line; it's about to be trampled */
        memcpy(t, surface->pixels, pitch);

        /* now, shuffle the rest so it's almost correct */
        a = (Uint8*)surface->pixels;
        last = a + pitch * (surface->h - 1);
        b = last;

        while (a < b)
        {
            memcpy(a, b, pitch);
            a += pitch;
            memcpy(b, a, pitch);
            b -= pitch;
        }

        /* in this shuffled state, the bottom slice is too far down */
        memmove(b, b + pitch, last - b);

        /* now we can put back that first row--in the last place */
        memcpy(last, t, pitch);

        /* everything is in the right place; close up. */
        free(t);

        // Write the file
        IMG_SavePNG(surface, filename.pathname().c_str());

        // Free everything
        SDL_FreeSurface(surface);
        _DELETE_ARRAY(screenPixels);
    }

    TEST_INVARIANT;
}

void RenSurface::ellipse(const Rect& area, const RenColour& penColour, const RenColour& brushColour)
{
    PRE(!readOnly());
    RenScopedImmediateCommands guard(RenDevice::current());

    int x, y, RX, RY;
    uint packedColour = packColour(brushColour.r(), brushColour.g(), brushColour.b(), brushColour.a());
    RX = area.width / 2;
    RY = area.height / 2;
    x = area.originX + RX;
    y = area.originY + RY;
    std::vector<RenIVertex> vertices;
    vertices.reserve(10);

    float i, inc, endAngle;
    endAngle = 3.1415 * 2;
    inc = endAngle / 10;
    i = 0;
    while (i <= endAngle)
    {
        RenIVertex vtx;
        vtx.x = ((RX * cos(i) + x));
        vtx.y = ((RY * sin(i) + y));
        vtx.color = packedColour;
        vertices.push_back(vtx);
        i += inc;
    }

    RenDevice::current()->recordCommand(Ren::Command::setCullFace(false));
    RenDevice* dev = RenDevice::current();
    if (internals() && internals()->isOffscreen())
    {
        dev->renderToTextureMode(handle(), width(), height());
        dev->renderScreenspace(vertices.data(), vertices.size(), Ren::PrimitiveTopology::TriangleFan, width(), -height());
        dev->renderToTextureMode(Ren::NullTexId, 0, 0);
    }
    else
        dev->renderScreenspace(vertices.data(), vertices.size(), Ren::PrimitiveTopology::TriangleFan, width(), height());
    RenDevice::current()->recordCommand(Ren::Command::setCullFace(true));
}

// These read/write functions are used for fog of war in savegame and store alpha only
void RenSurface::read(PerIstream& inStream)
{
    PRE(! readOnly() && ! sharable());

    size_t w = width();
    size_t h = height();

    size_t lineLength = w;

    uint* image = _NEW_ARRAY(uint, w * h);
    char* imageLine = _NEW_ARRAY(char, lineLength);

    {
        uint pixel = 0;
        for (size_t y = 0; y < h; ++y)
        {
            PER_READ_RAW_DATA(inStream, imageLine, w);

            for (size_t x = 0; x < w; ++x)
            {
                //              colRef = RGB( image[ ( x * 3 ) + 2 ], image[ ( x * 3 ) + 1 ], image[ x * 3 ] );
                //              SetPixel(hdc, x, y, colRef );
                image[pixel++] = imageLine[x] << 24;
            }
        }
        internals()->copyFromBuffer(image);
    }

    _DELETE_ARRAY(image);
    _DELETE_ARRAY(imageLine);
}

void RenSurface::write(PerOstream& outStream)
{
    // Ensure all pending render commands are submitted before reading
    // back pixel data — callers may have issued blits into the frame
    // command buffer that haven't been executed yet.
    RenDevice* dev = RenDevice::current();
    dev->flushCommandBuffer();

    const size_t w = width();
    const size_t h = height();

    PER_WRITE_RAW_OBJECT(outStream, w);
    PER_WRITE_RAW_OBJECT(outStream, h);

    // Read the entire surface in one GPU call instead of per-pixel.
    const size_t pixelCount = w * h;
    auto* rgba = _NEW_ARRAY(unsigned char, pixelCount * 4);

    if (internals() && internals()->isOffscreen())
    {
        dev->beginImmediateCommands();
        dev->renderToTextureMode(handle(), w, h);
        dev->endImmediateCommands();

        dev->backend().readPixelsUByte(0, 0, w, h, rgba);

        dev->beginImmediateCommands();
        dev->renderToTextureMode(Ren::NullTexId, 0, 0);
        dev->endImmediateCommands();
    }
    else
    {
        dev->backend().readPixelsUByte(0, 0, w, h, rgba);
    }

    auto* row = _NEW_ARRAY(char, w);
    for (size_t y = 0; y < h; ++y)
    {
        const size_t srcRow = y * w * 4;
        for (size_t x = 0; x < w; ++x)
            row[x] = static_cast<char>(rgba[srcRow + x * 4 + 3]);

        PER_WRITE_RAW_DATA(outStream, row, w);
    }

    _DELETE_ARRAY(row);
    _DELETE_ARRAY(rgba);
}

// static
WEAK_SYMBOL void RenSurface::mgrIncrementRefCount(Ren::TexId texId)
{
    RenSurfaceManager::instance().impl().incRefCount(texId);
}
// static
WEAK_SYMBOL void RenSurface::mgrDecrementRefCount(Ren::TexId texId)
{
    RenSurfaceManager::instance().impl().decRefCount(texId);
}

/* End BLITABLE.CPP *************************************************/
