/*
 * S U R F M G R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_SURFMGR_HPP
#define _RENDER_SURFMGR_HPP

#include "base/base.hpp"

#include <string>

// class ostream;
class SysPathName;
class RenTexture;
class RenSurface;
class RenISurfaceManagerImpl;
template <class T> class ctl_vector;

namespace Ren
{

std::string resolveTextureFile(const std::string& path);

} // namespace Ren

class RenSurfaceManager
{
public:
    using PathNames = ctl_vector<SysPathName>;

    //  Singleton class
    static RenSurfaceManager& instance();
    virtual ~RenSurfaceManager();

    // Create a texture; load it from disk if it hasn't been loaded already.
    // The pixel format is determined by the contents of the bitmap file.
    // Returns a null value on failure.  It would be nice to state a more
    // definitive post-condition about the name, however, the path searching
    // algorithm makes the actual name quite unpredicable.
    // POST(retval.sharable() && retval.readOnly());
    // POST(implies(!retval.isNull(), name().length() > 0));

    //  Note that this must be passed a string, not a pathname. If a SysPathName is
    //  used then any base directory set will be appended to it. This then stops the
    //  search mechanism from working.
    RenTexture createTexture(const std::string& pathName);

    // Create a surface which doesn't correspond to a named bitmap.  This can't
    // be shared because it doesn't have a name.
    // POST(!retval.sharable() && !retval.readOnly()); POST(name().length() == 0);
    // POST(retval.width() == width && retval.height() == height);
    RenSurface createAnonymousSurface(size_t width, size_t height);

    // Create a non-texture surface in video memory.
    // PRE(width > 0 and height > 0);
    // POST(!result.sharable() and !result.readOnly());
    // POST(name.length() == 0);
    RenSurface createAnonymousVideoSurface(size_t width, size_t height);

    // In terms of locating and sharing the data, this works just like
    // createTexture.
    // PRE(name.length() > 0);
    // POST(retval.sharable() && retval.readOnly());
    // POST(implies(!retval.isNull(), name().length() > 0));
    RenSurface createSharedSurface(const std::string& name);

    //  Maintains a list of directories to search for any given texture
    const PathNames& searchList();
    void searchList(const PathNames& newSearchList);

    // When Alt-Tab is pressed, textures can get unloaded from a hardware
    // device.  Calling this method reloads all managed textures.
    bool restoreAll();

    // Statistics: what textures are loaded?  What textures were drawn
    // during the current frame?  Memory used may become a more complex
    // issue if textures can be swapped into or out of a graphics card.
    uint nTexturesLoaded() const; // total textures loaded
    uint memoryUsed() const; // memory used by the above
    void startFrame(); // reset hit counts before render

    void CLASS_INVARIANT;
    friend std::ostream& operator<<(std::ostream& o, const RenSurfaceManager& t);

    RenISurfaceManagerImpl& impl();
    const RenISurfaceManagerImpl& impl() const;

    // PRE(result);
    static bool extractBMPText(const SysPathName& pathName, std::string* result);

private:
    RenISurfaceManagerImpl* pImpl_;

    // Singleton => private.
    RenSurfaceManager();

    // Operations deliberately revoked
    RenSurfaceManager(const RenSurfaceManager&);
    RenSurfaceManager& operator=(const RenSurfaceManager&);
    bool operator==(const RenSurfaceManager&);
};

#endif

/* End SURFMGR.HPP ***************************************************/
