/*
 * B L I T A B L E . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_BLITABLE_HPP
#define _RENDER_BLITABLE_HPP

#include "base/base.hpp"
#include "mathex/Point2d.hpp"
#include "render/render.hpp"

#include <optional>
#include <vector>

class SysPathName;
class RenColour;
class RenISurfBody;
template <class T> class ctl_vector;
// template <class T> class basic_string;
// typedef basic_string< char > string;

namespace Ren
{

class Font;
class TextOptions;

} // namespace Ren

// A class representing 2D images composed of pixels.  This interface can
// represent visible display surfaces, textures or arbitrary images created
// by clients.  The images can exist in main memory or video memory.
// RenDisplay and RenTexture both have methods which create RenSurfaces.
//
// In general, there are preconditions which ensure that any write operation,
// i.e., blit, fill, line, etc, does not write outside the destination's area.
// Plus, writes cannot be applied to read-only surfaces: PRE(!readOnly());
// They are not explicitly listed.
class RenSurface
{
public:
    using Point = Ren::Point;
    using Size = Ren::Size;
    using Rect = Ren::Rect;

    // Creates a null, not very useful surface.
    // POST(isNull() && sharable() && readOnly());
    RenSurface();

    // Surface creation is controlled by the manager.  However, you may copy
    // surface objects to your heart's content.
    RenSurface(const RenSurface&);
    RenSurface& operator=(const RenSurface&);

    void swap(RenSurface&& other) { std::swap(other.myId_, myId_); }

    // Create a surface of the given size.
    // PRE(width > 0 && height > 0);
    // POST(!retval.sharable() && !retval.readOnly()); POST(name.length() == 0);
    static RenSurface createAnonymousSurface(Size size);

    // Create a non-texture surface in video memory.
    // PRE(width > 0 and height > 0);
    // POST(!retval.sharable() && !retval.readOnly()); POST(name.length() == 0);
    static RenSurface createAnonymousVideoSurface(Size size);

    // Create a surface initialised with data stored in the persistent stream.
    // POST(!retval.sharable() && !retval.readOnly()); POST(name.length() == 0);
    static RenSurface createAnonymousSurface(PerIstream&);

    // Create a shared surface initialised with data from the given named bitmap.
    // Uses the surface manager's search path to locate the named BMP file.
    // POST(retval.sharable() && retval.readOnly());
    static RenSurface createSharedSurface(const std::string& bitmapName);

    // Create a new surface whose area subsets the given one's area.  The result
    // shares the same representation & data, so the input must be sharable.
    // Not implemented yet.
    // PRE(surf.sharable()); PRE(!surf.isNull());
    // PRE(subArea.width <= surf.width() && subArea.height <= surf.height());
    // POST(retval.width() == subArea.width && retval.height() == subArea.height);
    // POST(retval.sharable());
    static RenSurface createSurface(const RenSurface& surf, const Rect& subArea);

    virtual ~RenSurface();

    // Use to indicate bad return values etc.
    bool isNull() const;
    void reset();

    // Blit from the given surface to this one:
    //   simple = one copy, no repeats and no stretching
    //   tiled  = repeated w/o stretching to entirely cover the destination
    // The shorter version of simpleBlit copies the entire source.
    // PRE(source.pixelFormat() == pixelFormat());      applies to all blits
    void simpleBlit(
        const RenSurface& source, const std::optional<Rect>& srcArea = {}, Point dest = {}, Ren::BlitMode mode = {});
    void tileBlit(const RenSurface& source, const Rect& srcArea, const Rect& destArea);
    void stretchBlit(const RenSurface& source, const Rect& srcArea, const Rect& destArea, Ren::BlitMode mode = {});
    void stretchBlit(const RenSurface& source, Ren::BlitMode mode = {});
    void blitInRequestedSize(const RenSurface& source, Point dest, Ren::BlitMode mode = {});
    void copyFromRGBABuffer(const uint* buff);

    void getPixel(int x, int y, RenColour*) const;

    // An area of a surface brought back into main memory, four bytes per pixel,
    // with the rows running from the top down.
    struct Pixels
    {
        int width{};
        int height{};
        std::vector<unsigned char> rgba{};
    };

    // Reads back the given area, or the whole surface when the area is empty.
    // The area is measured from the top left corner, and is brought inside the
    // surface, so a caller need not know how much surface there is. Empty if
    // nothing of the area is on the surface.
    Pixels readPixels(const Rect& area = Rect()) const;

    // The given area as the bytes of a PNG file, for a caller that wants to send
    // or store the image rather than write it out here. Empty if the area holds
    // no pixels or the encoding failed.
    std::vector<unsigned char> encodePng(const Rect& area = Rect()) const;

    // Writes the given area of the surface out, or the whole of it when the area is
    // empty. The area is measured from the top left corner.
    void saveAsPng(const SysPathName&, const Rect& area = Rect()) const;

    // Returns the actual size used, in case an exact match doesn't exist.
    static int getDefaultFontSize();
    static void setDefaultFontSize(int size);

    using Points = ctl_vector<MexPoint2d>;

    // Is source colour keying enabled for this image?
    bool isColourKeyingOn() const;
    void enableColourKeying();
    void disableColourKeying(); // default state is off

    // The colour key to use when this object is used as a source in a blit.
    const RenColour& colourKey() const;
    void colourKey(const RenColour&); // default == RenColour::magenta

    // You cannot change the name of a surface which already has a name.
    // PRE(name().length() == 0);
    void name(const std::string&);
    const std::string& name() const;
    Ren::TexId handle() const;

    Size requestedSize() const;
    void setRequestedSize(Size size);

    // Sharing and read/write control.
    bool sharable() const; // Is this English?
    bool readOnly() const;

    // You cannot do the opposite: change a read-only surface to be writable.
    // You can't change this property on a shared surface, however, if it's
    // already read-only, then it doesn't make any difference.
    // PRE(readOnly() || !sharable());
    // POST(sharable() && readOnly());
    void makeReadOnlySharable();

    size_t width() const;
    size_t height() const;
    Size size() const;
    bool isEmpty() const; // i.e., width() == 0 || height() == 0
    bool isOffscreen() const;

    // Allows a body to get a handle corresponding to itself.
    // PRE(body);
    static RenSurface createFromInternal(RenISurfBody* body);
    static const RenSurface createFromInternal(const RenISurfBody* body);

    // Read pixels from persistence stream. Assumes that the data in the persistence stream
    // corresponds to the same width and height as this RenSurface ( width and height information
    // should have already been removed from the persistence stream at this point). To avoid
    // having to create a RenSurface before calling this function you can just use the
    // createAnonymousSurface function passing in the PerIstream, this takes care of setting up
    // a surface of the correct width and height.
    // PRE( not readOnly() and not sharable() );
    void read(PerIstream& inStream);

    // Write pixels to persistence stream. Writes out the width and height first followed by
    // pixel colour information.
    void write(PerOstream& outStream);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const RenSurface& t);

    friend bool operator==(const RenSurface&, const RenSurface&);
    friend bool operator!=(const RenSurface&, const RenSurface&);

protected:
    Ren::TexId myId() const { return myId_; }

private:
    // Create a blitable from an the internal data representation.
    // RenTexture can do this to create blitables corresponding to textures.
    // RenDisplay can likewise create blitables corresponding to its buffers.
    // This object becomes responsible for deleting the internals.
    // TBD: replace this functionality in light of RenSurfaceManager.
    //  friend class RenDisplay;
    //  friend class RenTexture;
    //  RenSurface(RenISurfBody* i);        // PRE(i);

    // Compile Seam Wrappers for the RenSurfaceManager calls
    static void mgrIncrementRefCount(Ren::TexId texId);
    static void mgrDecrementRefCount(Ren::TexId texId);

    // Only the manager class can allocate Ids and create surfaces.
    // (A texture can create a surface corresponding to its own id.)
    friend class RenSurfaceManager;
    friend class RenISurfaceManagerImpl;
    friend class RenTexture;
    RenSurface(Ren::TexId);

    Ren::TexId myId_;

    RenISurfBody* internals();
    const RenISurfBody* internals() const;
};

#endif

/* End BLITABLE.HPP *************************************************/
