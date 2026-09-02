/*
 * S U R F M G R . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/SurfaceManager.hpp"

#include "render/internal/SurfaceManagerImpl.hpp"

#include "base/Diag.hpp"
#include "render/Texture.hpp"
#include "render/Device.hpp"
#include "render/internal/TextureBody.hpp"
#include "render/internal/SurfaceBody.hpp"
#include "render/internal/DeviceImpl.hpp"

#include "system/PathName.hpp"
#include "system/VFS.hpp"

#include <stdio.h>
#include <climits>
#include <algorithm>

static constexpr char s_PngTextureSuffix[] = ".png";
static constexpr char s_BmpTextureSuffix[] = ".bmp";
static constexpr auto s_BmpSuffixSize = sizeof(s_BmpTextureSuffix) - 1;

namespace Ren
{

std::string resolveTextureFile(const std::string& path)
{
    const bool hasBmpExtension = path.size() > s_BmpSuffixSize
        && path.substr(path.size() - s_BmpSuffixSize, s_BmpSuffixSize) == s_BmpTextureSuffix;

    if (hasBmpExtension)
    {
        std::string pngPath = path;
        const auto from = pngPath.end() - s_BmpSuffixSize;
        pngPath.replace(from, pngPath.end(), s_PngTextureSuffix);

        pngPath = System::findFile(pngPath);
        if (SysPathName::existsAsFile(pngPath))
            return pngPath;
    }

    return System::findFile(path);
}

} // namespace Ren

////////////////////////////////////////////////////////////

#define CB_REN_SURFACE_MANAGER_DEPIMPL                                                                                 \
    CB_DEPIMPL(ctl_vector<RenISurfBody*>, entries_);                                                                   \
    CB_DEPIMPL(RenISurfaceManagerImpl::NameMap, nameMap_);                                                             \
    CB_DEPIMPL(PathNames, directorySearchList_);                                                                       \
    CB_DEPIMPL(const Ren::TexId, firstValidId_);                                                                       \
    CB_DEPIMPL(const RenDevice*, handleDevice_)

#define CB_PEER_PTR_DEPIMPL(objectPtr, name) objectPtr->pImpl_->name
#define CB_PEER_REF_DEPIMPL(objectRef, name) objectRef.pImpl_->name

////////////////////////////////////////////////////////////

// static
RenSurfaceManager::RenSurfaceManager()
    : pImpl_(new RenISurfaceManagerImpl())
{
    PRE(Ren::initialised());
    TEST_INVARIANT;
}

RenSurfaceManager::~RenSurfaceManager()
{
    TEST_INVARIANT;
    delete pImpl_;
}

// As an anonymous surface, this cannot be shared, so we don't search for a
// matching name in the existing surfaces.
RenSurface RenSurfaceManager::createAnonymousSurface(size_t width, size_t height)
{
    CB_REN_SURFACE_MANAGER_DEPIMPL;

    PRE(width > 0 && height > 0);
    TEST_INVARIANT;

    if (width == 0 || height == 0)
        return RenSurface();

    RenISurfBody* newSurf = new RenISurfBody(width, height, RenISurfBody::SYSTEM);

    Ren::TexId newId = pImpl_->addAnonymousEntry(newSurf);

    RenSurface retval(newId);

    TEST_INVARIANT;
    POST(!retval.sharable() && !retval.readOnly());
    POST(retval.name().length() == 0);
    POST(retval.width() == width && retval.height() == height);
    return retval;
}

RenSurface RenSurfaceManager::createAnonymousVideoSurface(size_t width, size_t height)
{
    PRE(width > 0 && height > 0);

    CB_REN_SURFACE_MANAGER_DEPIMPL;

    TEST_INVARIANT;

    if (width == 0 || height == 0)
        return RenSurface();

    RenISurfBody* newSurf = new RenISurfBody(width, height, RenISurfBody::VIDEO);

    Ren::TexId newId = pImpl_->addAnonymousEntry(newSurf);

    RenSurface result(newId);

    POST(!result.sharable() && !result.readOnly());
    POST(result.name().length() == 0);

    return result;
}

RenSurface RenSurfaceManager::createSharedSurface(const std::string& name)
{
    CB_REN_SURFACE_MANAGER_DEPIMPL;

    TEST_INVARIANT;

    const Ren::TexId newId = pImpl_->createSurfOrTex(name, false);

    RenSurface retval(newId);
    POST(retval.sharable() && retval.readOnly());
    POST(implies(!retval.isNull(), retval.name().length() > 0));
    TEST_INVARIANT;
    return retval;
}

RenTexture RenSurfaceManager::createTexture(const std::string& pathName)
{
    CB_REN_SURFACE_MANAGER_DEPIMPL;

    TEST_INVARIANT;

    std::string pathNameLower(pathName);
    std::transform(pathNameLower.begin(), pathNameLower.end(), pathNameLower.begin(), ::tolower);
    const Ren::TexId newId = pImpl_->createSurfOrTex(pathNameLower, true);

    RenTexture retval(newId);
    POST(retval.sharable() && retval.readOnly());
    POST(implies(!retval.isNull(), retval.name().length() > 0));
    TEST_INVARIANT;
    return retval;
}

uint RenSurfaceManager::refCount(const RenSurface& surface) const
{
    if (surface.isNull())
        return 0;

    const RenISurfBody* body = pImpl_->getSurfaceBody(surface.handle());
    return body ? body->refCount() : 0;
}

// There can be null slots, so this isn't necessarily entries_.size().
uint RenSurfaceManager::nTexturesLoaded() const
{
    CB_REN_SURFACE_MANAGER_DEPIMPL;

    TEST_INVARIANT;

    uint count = 0;
    for (Ren::TexId id = firstValidId_; id != entries_.size(); ++id)
    {
        RenISurfBody* entry = entries_[id];
        if (entry && entry->castToTexBody())
            ++count;
    }

    return count;
}

uint RenSurfaceManager::memoryUsed() const
{
    CB_REN_SURFACE_MANAGER_DEPIMPL;

    TEST_INVARIANT;

    uint mem = 0;
    for (Ren::TexId id = firstValidId_; id != entries_.size(); ++id)
    {
        RenISurfBody* entry = entries_[id];
        if (entry)
            mem += entry->memoryUsed();
    }

    return mem;
}

void RenSurfaceManager::startFrame()
{
    CB_REN_SURFACE_MANAGER_DEPIMPL;

    TEST_INVARIANT;

    if (pImpl_->handleSets_ > 0)
    {
        const double percent = 100.0 * pImpl_->stateChanges_ / pImpl_->handleSets_;
        RENDER_STREAM("Texture handle requests=" << pImpl_->handleSets_ << "\n");
        RENDER_STREAM("Actual state changes   =" << pImpl_->stateChanges_ << " (" << percent << "%)\n");
    }

    pImpl_->handleSets_ = pImpl_->stateChanges_ = 0;

    // Mark this as not used at all.
    pImpl_->lastUsedId_ = UINT_MAX;

    TEST_INVARIANT;
}

const RenSurfaceManager::PathNames& RenSurfaceManager::searchList()
{
    CB_REN_SURFACE_MANAGER_DEPIMPL;

    TEST_INVARIANT;
    return directorySearchList_;
}

void RenSurfaceManager::searchList(const PathNames& newSearchList)
{
    CB_REN_SURFACE_MANAGER_DEPIMPL;

    TEST_INVARIANT;
    directorySearchList_ = newSearchList;
}

// static
RenSurfaceManager& RenSurfaceManager::instance()
{
    static RenSurfaceManager instance_;
    return instance_;
}

std::ostream& operator<<(std::ostream& o, const RenSurfaceManager& t)
{
    o << "RenSurfaceManager." << std::endl;
    t.pImpl_->write(o);
    o << "Total memory used " << t.memoryUsed() << std::endl;

    return o;
}

void RenSurfaceManager::CLASS_INVARIANT
{
    INVARIANT(this);
    INVARIANT(pImpl_);
}

RenISurfaceManagerImpl& RenSurfaceManager::impl()
{
    PRE(pImpl_);
    return *pImpl_;
}

const RenISurfaceManagerImpl& RenSurfaceManager::impl() const
{
    PRE(pImpl_);
    return *pImpl_;
}


/* End SURFMGR.CPP ***************************************************/
