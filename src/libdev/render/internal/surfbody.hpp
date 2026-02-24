/*
 * I N S U R F C E . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_INSURFCE_HPP
#define _RENDER_INSURFCE_HPP

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "render/render.hpp"
#include "render/colour.hpp"
#include "utility/property.hpp"
#include "render/internal/IRenderBackend.hpp"
#include "render/internal/internal.hpp"

#include <cstdint>

struct SDL_Surface;

// class ostream;
class SysPathname;
class RenITexBody;
class RenIFont;
class RenDevice;

namespace Ren::OpenGL
{
class RenderBackendGL;
}

namespace Ren
{

class Font;
class TextOptions;

} // namespace Ren

// The internal data representation for RenSurface.
class RenISurfBody
{
public:
    // The various ctors have different intended clients.
    // This creates a default, null surface and is used by the manager.
    // POST(width() == 0 && height == 0);
    // POST(sharable() && readOnly()); POST(name().length() == 0);
    RenISurfBody();

    virtual ~RenISurfBody();

    // This is a concrete base class.  The only intended derived class is RenITexBody.
    virtual bool read(const std::string& name);
    // Load from filePath on disk but store logicalName for sharing/lookup purposes.
    virtual bool read(const std::string& filePath, const std::string& logicalName);
    virtual RenITexBody* castToTexBody();
    virtual const RenITexBody* castToTexBody() const;
    virtual void print(std::ostream&) const;

    UtlProperty<RenColour> keyColour; // default is purple
    UtlProperty<bool> keyingOn; // default is false

    size_t width() const;
    size_t height() const;
    Ren::Size size() const;
    size_t memoryUsed() const;
    bool isEmpty() const; // width == 0 || height == 0

    // These are methods which would be private internal methods of RenSurface,
    // however, they are placed here to reduce dependancies.
    void setDDColourKey();
    void filledRectangle(const Ren::Rect& area, uint colour);
    void drawText(
        int x, int y, const std::string_view& text, const Ren::Font& font, const Ren::TextOptions& options);

    // When Alt-Tab is pressed, textures can get unloaded from a hardware
    // device.  Calling this method reloads this image.
    bool restoreToVRAM() const;
    bool loadIntoVRAM() const;
    bool recreateVRAMSurface();

    // HDC getDC();
    // HDC DC() const;
    void releaseDC();

    uint refCount() const;
    void incRefCount();
    void decRefCount();

    Ren::Size requestedSize() const { return requestedSize_; }
    void setRequestedSize(Ren::Size value) { requestedSize_ = value; }

    // You cannot change the name of a surface which already has a name.
    // PRE(name().length() == 0);
    void name(const std::string&);
    const std::string& name() const;

    // returns the name of the shared resource (leafName or full pathName)
    const std::string& sharedName() const;

    // is the file leaf name to be shared or the full pathname
    void shareLeafName(bool);

    // Sharing and read/write control.
    bool sharable() const; // Is this English?
    bool readOnly() const;

    // You cannot do the opposite: change a read-only surface to be writable.
    // You can't change this property on a shared surface, however, if it's
    // already read-only, then it doesn't make any difference.
    // PRE(readOnly() || !sharable());
    // POST(sharable() && readOnly());
    void makeReadOnlySharable();

    void unclippedBlit(const RenISurfBody* source, const Ren::Rect& srcArea, int destX, int destY);
    void unclippedStretchBlit(const RenISurfBody* source, const Ren::Rect& srcArea, const Ren::Rect& destArea);
    void unclippedBlit(const RenISurfBody* source, const Ren::Rect& srcArea, int destX, int destY, Ren::BlitMode mode);
    void unclippedStretchBlit(
        const RenISurfBody* source,
        const Ren::Rect& srcArea,
        const Ren::Rect& destArea,
        Ren::BlitMode mode);

    // Initialise this surface using a second bitmap as an alpha map.
    bool copyWithAlpha(SDL_Surface* surface, SDL_Surface* surfaceAlpha, bool createMipmaps = false);

    // Copy an image from a GDI Bitmap to a DirectDraw surface.  Whilst copying,
    // if any texel matches the given colour key, the alpha component of the
    // output texture will be appropriately set to transparent.
    bool copyWithColourKeyEmulation(SDL_Surface* surface, const RenColour& keyColour, bool createMipmaps = false);
    // Copy image from RGBA buffer of size width * height
    bool copyFromBuffer(const uint* pixelsBuffer);

    // This does a case-insensitive comparison on the names.  The manager
    // needs this test to implement it's factory like behaviour.
    bool matches(const std::string& name) const;

    bool isFront() const { return displayType_ == RenI::FRONT; }
    bool isOffscreen() const { return displayType_ == RenI::NOT_DISPLAY; }

protected:
    // Used by the surface manager to create surfaces is specific places. The = 2 is a
    // safety measure as the parameter used to be a bool.
    enum Residence
    {
        TEXTURE = 2,
        SYSTEM,
        VIDEO
    };

    bool allocateDDSurfaces(size_t width, size_t height, Residence);

    static SDL_Surface *readFromFile(const char *fileName);

private:
    // Only the surface manager can create internal surface objects.
    friend class RenSurfaceManager;
    friend class RenISurfaceManagerImpl;
    friend class Ren::OpenGL::RenderBackendGL;

    // Allocates DirectDraw memory for textures and non-texture bitmaps. This
    // creates a writable surface.  The client must change it to read-only, if
    // appropriate, e.g., all textures are read-only.
    // POST(!sharable() && !readOnly());  POST(name().length() == 0);
    // POST(width() == w && height() == h); POST(pixelFormat.isValid());
    RenISurfBody(size_t w, size_t h, Residence);

    // This creates a surface corresponding to a display backbuffer.  Although
    // the display may be shared by multiple clients, it is not sharable in the
    // sense that the surface manager will find a named copy of the surface.  In
    // order to aquire a display surface, you must go via a RenDisplay.
    // PRE(dev); PRE(t != NOT_DISPLAY);
    // POST(!sharable() && !readOnly()); POST(name().length() == 0);
    RenISurfBody(const RenDevice* dev, RenI::DisplayType t);

    // Truly private, as opposed to available to friends.
    void updateDescr();

    RenI::DisplayType displayType_;
    const RenDevice* device_{};
    Ren::BackendTextureHandle nativeTexture2D_{};
    uint width_, height_;

    uint refCount_{};
    std::string name_, leafName_;
    Ren::Size requestedSize_{};
    bool sharedLeaf_;
    bool loaded_{};
    bool readOnly_{};
    bool sharable_{};

    RenISurfBody(const RenISurfBody&);
    RenISurfBody& operator=(const RenISurfBody&);
};

std::ostream& operator<<(std::ostream& o, const RenISurfBody& t);

#endif

/* End INSURFCE.HPP *************************************************/
