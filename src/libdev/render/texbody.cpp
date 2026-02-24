/*
 * T E X B O D Y . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/internal/texbody.hpp"

#include <stdlib.h> // for abs
#include <iostream>
#include <iomanip>
#include "base/diag.hpp"
#include "render/device.hpp"
#include "render/colour.hpp"
#include "render/capable.hpp"
#include "render/internal/debug.hpp"
#include "render/internal/devicei.hpp"

#ifndef _INLINE
#include "render/internal/texbody.ipp"
#endif

#include <SDL2/SDL_image.h>

RenITexBody::RenITexBody()
    : RenISurfBody()
    ,
#ifndef NDEBUG
//  handleDev_(NULL),
#endif
    loaded_(false)
    , hitsPerFrame_(0)
    , alpha_(false)
    , bilinear_(false)
{
    TEST_INVARIANT;
}

RenITexBody::~RenITexBody()
{
    TEST_INVARIANT;
}

static bool isPowerOf2(int dimension)
{
    switch (dimension)
    {
        case 1:
            return true;
        case 2:
            return true;
        case 4:
            return true;
        case 8:
            return true;
        case 16:
            return true;
        case 32:
            return true;
        case 64:
            return true;
        case 128:
            return true;
        case 256:
            return true;
        case 512:
            return true;
        case 1024:
            return true;
        case 2048:
            return true;
        case 4096:
            return true;
        default:
            return false;
    }
}

// Implements a naming convention: textures which use colour keyed
// transparency have names ending in "_t" or "_bt".
static bool isTransparent(const SysPathName& pathname)
{
    const std::string& texName = pathname.filename();
    const std::string end = texName.substr(texName.length() - 6, 6);
    if (strcasecmp(end.c_str(), "_t.bmp") == 0)
        return true;

    const std::string end2 = texName.substr(texName.length() - 7, 7);
    return strcasecmp(end2.c_str(), "_bt.bmp") == 0;
}

// These two methods are only used for assert checking.
#ifndef NDEBUG
static bool isAlpha(const SysPathName& pathname)
{
    bool result = false;
    const std::string& texName = pathname.filename();
    const std::string end = texName.substr(texName.length() - 6, 6);
    if (strcasecmp(end.c_str(), "_a.bmp") == 0)
        result = true;
    const std::string end2 = texName.substr(texName.length() - 7, 7);
    if (strcasecmp(end2.c_str(), "_ba.bmp") == 0)
        result = true;
    return result;
}

static bool isColour(const SysPathName& pathname)
{
    bool result = false;
    const std::string& texName = pathname.filename();
    const std::string end = texName.substr(texName.length() - 6, 6);
    if (strcasecmp(end.c_str(), "_c.bmp") == 0)
        result = true;
    const std::string end2 = texName.substr(texName.length() - 7, 7);
    if (strcasecmp(end2.c_str(), "_bc.bmp") == 0)
        result = true;
    return result;
}
#endif

// Implements a naming convention: textures which use colour keyed
// transparency have names ending in "_b" or "_bt".
static bool bilinearRequired(const SysPathName& pathname)
{
    const std::string& texName = pathname.filename();
    const std::string end = texName.substr(texName.length() - 6, 6);
    if (strcasecmp(end.c_str(), "_b.bmp") == 0)
        return true;

    const std::string end2 = texName.substr(texName.length() - 7, 7);
    return strcasecmp(end2.c_str(), "_bt.bmp") == 0;
}

// Implements a naming convention: alpha maps have the same name as their
// associated texture map, except that the "_t" is replaced with "_a".
static SysPathName alphaMapName(const SysPathName& pathname)
{
    PRE(isTransparent(pathname));

    std::string retval = pathname.pathname();
    retval[retval.length() - 5] = 'a';
    return retval;
}

// Implements a naming convention: certain textures require a colour-only map
// *without* transparency colour-keying.  Given a normal, texture map name, the
// associated colour-only map has the same name, except that the "_t" is
// replaced with "_c".
static SysPathName colourMapName(const SysPathName& pathname)
{
    PRE(isTransparent(pathname));

    std::string retval = pathname.pathname();
    retval[retval.length() - 5] = 'c';
    return retval;
}

// static bool checkTextureSize(const SysPathName& fileName, const BITMAP& bitmap)
static bool checkTextureSize(const SysPathName& fileName, const SDL_Surface* surface)
{
    // ASSERT_INFO(bitmap.bmWidth);
    // ASSERT_INFO(bitmap.bmHeight);
    ASSERT_INFO(surface->w);
    ASSERT_INFO(surface->h);

    // There's a D3D capability query which tells you the maximum size of a
    // texture.  However, all 3Dfx systems appear to have this 256 limitation.
    // So, in practice, it applies to all textures all of the time.  To cope
    // the models must be built with this in mind, hence it's hardcoded.
    // TBD: we could maybe devise a better strategy for dealing with this, i.e.
    // automatically down-sizing some textures which are too large???
    const static uint16 maxTextureSize = 4096;

    if (surface->w > maxTextureSize || surface->h > maxTextureSize)
    {
        RENDER_STREAM("  Texture " << fileName << " must be smaller than " << maxTextureSize << "." << std::endl);
        RENDER_STREAM("  (Its size is " << surface->w << "x" << surface->h << ".)" << std::endl);

        ASSERT_FAIL("Texture size is greater than the maximum (4096x4096).");
        return false;
    }

    return true;
}

// virtual
bool RenITexBody::read(const std::string& nameAsString)
{
    return read(nameAsString, nameAsString);
}

bool RenITexBody::read(const std::string& filePath, const std::string& logicalName)
{
    PRE(! isAlpha(SysPathName(logicalName)));
    PRE(! isColour(SysPathName(logicalName)));
    PRE(Ren::initialised());
    TEST_INVARIANT;

    // Use the logical name (.bmp) for naming convention checks.
    const SysPathName requestedTextureName(logicalName);
    RENDER_STREAM("Trying to load texture " << requestedTextureName << ":" << std::endl);
    ASSERT_INFO(requestedTextureName);

    const bool transparent = isTransparent(requestedTextureName);
    const RenCapabilities& caps = RenDevice::current()->capabilities();
    bilinear_ = bilinearRequired(requestedTextureName) && caps.supportsBilinear();
    const bool colourKey = caps.supportsColourKey();

    // Start with the resolved file path for loading.
    std::string loadPath = filePath;
    const SysPathName colourName = (transparent) ? colourMapName(requestedTextureName) : requestedTextureName;

    // If this is a transparent texture and the device supports alpha texture
    // and a special colour-only map exists, then load that in preference to
    // the named file given as the arg to this function.
    if (transparent && caps.supportsTextureAlpha())
    {
        const std::string resolvedColourName = Ren::resolveTextureFile(colourName.pathname());
        if (SysPathName::existsAsFile(resolvedColourName))
        {
            // However, if the texture is *not* bilinear blended and colour-keying
            // *is* supported, load the colour-keyed version because it will
            // typically be more efficient and avoids alpha sorting problems.  This
            // should be the correct choice for architectures without colour-keying
            //  (PowerVR) and for architectures with both alpha and keying (3Dfx).
            if (bilinear_ || !colourKey)
            {
                loadPath = resolvedColourName;
                RENDER_STREAM("  Colour-only map exists, loading " << resolvedColourName << " instead." << std::endl);
            }
        }
    }

    SDL_Surface* surface = readFromFile(loadPath.c_str());
    if (!surface)
        return false;

    if (!checkTextureSize(requestedTextureName, surface))
        return false;

    // Use a separate alpha map file if it exists.
    bool tryToLoadAlpha = transparent && caps.supportsTextureAlpha();

    // But prefer colour-keying if it's supported and bilinear is not required.
    if (colourKey && !bilinear_)
        tryToLoadAlpha = false;

    SDL_Surface* surfaceAlpha = nullptr;
    if (tryToLoadAlpha)
    {
        const SysPathName alphaName = alphaMapName(requestedTextureName);
        const std::string resolvedAlphaName = Ren::resolveTextureFile(alphaName.pathname());
        RENDER_STREAM("  Looking for alpha map file " << alphaName);

        if (!SysPathName::existsAsFile(resolvedAlphaName))
        {
            RENDER_STREAM(" (not found)" << std::endl);
        }
        else
        {
            RENDER_STREAM(" (found)" << std::endl);

            surfaceAlpha = readFromFile(resolvedAlphaName.c_str());
            if (!surfaceAlpha)
                return false;

            ASSERT(surfaceAlpha->w == surface->w, "Wrong alpha map size.");
            ASSERT(surfaceAlpha->h == surface->h, "Wrong alpha map size.");
        }
    }

    //  alpha_ = transparent && alphaHandle;
    alpha_ = transparent && surfaceAlpha;

    // If the HAL's colour model is set, that implies that 3D hardware is
    // present, therefore create the main surface in video memory.
    RenISurfBody::Residence residence = SYSTEM;

    if (caps.hardware())
        residence = TEXTURE;

    if (!allocateDDSurfaces(surface->w, surface->h, residence))
    {
        // DeleteObject(handle);
        SDL_FreeSurface(surface);
        // DeleteObject(alphaHandle);
        return false;
    }

    // Store the logical name for sharing/lookup purposes.
    name(logicalName);

    // copy the bitmap to our surface
    bool retval;
    if (alpha_)
    {
        retval = copyWithAlpha(surface, surfaceAlpha, true);
        SDL_FreeSurface(surfaceAlpha);
    }
    //  else if (transparent && !colourKey)
    else
    {
        //      ASSERT(alphaChannelFound, logic_error());
        // retval = copyWithColourKeyEmulation(handle, RenColour::magenta());
        retval = copyWithColourKeyEmulation(surface, RenColour::magenta(), true);
        alpha_ = transparent;
    }

    loaded_ = true;
    TEST_INVARIANT;

    SDL_FreeSurface(surface);
    RENDER_STREAM(std::endl);
    return retval;
}

bool RenITexBody::hasAlphaTransparency() const
{
    return alpha_;
}

bool RenITexBody::usesBilinear() const
{
    return bilinear_;
}

uint RenITexBody::hitsPerFrame() const
{
    TEST_INVARIANT;
    return hitsPerFrame_;
}

void RenITexBody::incHits()
{
    TEST_INVARIANT;
    ++hitsPerFrame_;
}

void RenITexBody::resetHits()
{
    TEST_INVARIANT;
    hitsPerFrame_ = 0;
}
void RenITexBody::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);

    // If the texture loaded correctly, these should all be non zero.
    //  INVARIANT(implies(loaded_, width_    > 0));
    //  INVARIANT(implies(loaded_, height_   > 0));
    //  INVARIANT(implies(loaded_, bitDepth_ > 0));
}

// virtual
RenITexBody* RenITexBody::castToTexBody()
{
    return this;
}

// virtual
const RenITexBody* RenITexBody::castToTexBody() const
{
    return this;
}

// virtual
void RenITexBody::print(std::ostream& o) const
{
    o << "texture " << name() << " (" << width() << "x" << height() << ")";
}

/* End TEXBODY.CPP **************************************************/
