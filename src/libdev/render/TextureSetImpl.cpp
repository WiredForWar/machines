/*
 * T E X S E T I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "render/internal/TextureSetImpl.hpp"
#include "base/Diag.hpp"
#include "base/IProgressReporter.hpp"
#include "system/PathName.hpp"
#include "system/FileEnumerator.hpp"
#include "system/FileData.hpp"
#include "render/SurfaceManager.hpp"

#include <algorithm>
#include <set>
#include <string>

#define TEXSET_STREAM(x) RENDER_STREAM(x)
#define TEXSET_INDENT(x) RENDER_INDENT(x)

RenTextureSetImpl::RenTextureSetImpl()
    : isLoaded_(false)
{
    TEST_INVARIANT;
}

RenTextureSetImpl::RenTextureSetImpl(const SysPathName& directory)
    : isLoaded_(false)
{
    load(directory);

    TEST_INVARIANT;
}

RenTextureSetImpl::RenTextureSetImpl(const SysPathName& directory, IProgressReporter* pReporter)
    : isLoaded_(false)
{
    PRE(pReporter);

    load(directory, pReporter);

    TEST_INVARIANT;
}

RenTextureSetImpl::~RenTextureSetImpl()
{
    TEST_INVARIANT;
    TEXSET_STREAM("Unloading textures " << std::endl);
    TEXSET_INDENT(2);
    TEXSET_STREAM(RenSurfaceManager::instance());

    textures_.erase(textures_.begin(), textures_.end());

    TEXSET_STREAM("Unloaded" << std::endl);
    TEXSET_STREAM(RenSurfaceManager::instance());
    TEXSET_INDENT(-2);
}

// Check for alpha/colour map naming conventions, supporting both .bmp and .png extensions.
static bool endsWithCaseInsensitive(const std::string& str, const char* suffix, size_t suffixLen)
{
    if (str.size() < suffixLen)
        return false;
    return strcasecmp(str.c_str() + str.size() - suffixLen, suffix) == 0;
}

static bool isAlphaMap(const SysPathName& pathname)
{
    const std::string& texName = pathname.filename();
    return endsWithCaseInsensitive(texName, "_a.bmp", 6)
        || endsWithCaseInsensitive(texName, "_a.png", 6)
        || endsWithCaseInsensitive(texName, "_ba.bmp", 7)
        || endsWithCaseInsensitive(texName, "_ba.png", 7);
}

static bool isColourMap(const SysPathName& pathname)
{
    return endsWithCaseInsensitive(pathname.filename(), "_c.bmp", 6)
        || endsWithCaseInsensitive(pathname.filename(), "_c.png", 6)
        || endsWithCaseInsensitive(pathname.filename(), "_bc.bmp", 7)
        || endsWithCaseInsensitive(pathname.filename(), "_bc.png", 7);
}

// Return the stem of a filename (without the last extension).
static std::string filenameStem(const std::string& filename)
{
    auto dot = filename.rfind('.');
    if (dot == std::string::npos)
        return filename;
    return filename.substr(0, dot);
}

void RenTextureSetImpl::load(const SysPathName& directory, IProgressReporter* pReporter)
{
    PRE(! isLoaded_);

    TEXSET_STREAM("Preloading textures from directory " << directory << std::endl);
    TEXSET_INDENT(2);
    TEXSET_STREAM(RenSurfaceManager::instance());

    // Scan for .bmp files (the original game assets)
    SysFileEnumerator bmpFinder(directory, "*.bmp");
    bmpFinder.examineSubdirectories(true);
    bmpFinder.find();

    // Also scan for .png files (mod replacements or new assets)
    SysFileEnumerator pngFinder(directory, "*.png");
    pngFinder.examineSubdirectories(true);
    pngFinder.find();

    const SysFileEnumerator::FileDatas& bmpFiles = bmpFinder.files();
    const SysFileEnumerator::FileDatas& pngFiles = pngFinder.files();

    TEXSET_STREAM("Found " << bmpFiles.size() << " .bmp and " << pngFiles.size() << " .png files" << std::endl);

    // Collect all .bmp stems so we skip .png files that already have a .bmp counterpart
    // (the .bmp will be resolved to .png by findTextureFile if appropriate).
    std::set<std::string> bmpStems;
    for (auto it = bmpFiles.begin(); it != bmpFiles.end(); ++it)
    {
        std::string stem = filenameStem((*it).pathName().filename());
        std::transform(stem.begin(), stem.end(), stem.begin(), ::tolower);
        bmpStems.insert(std::move(stem));
    }

    // Merge: all .bmp files + .png files that don't duplicate a .bmp
    std::vector<SysPathName> filesToLoad;
    filesToLoad.reserve(bmpFiles.size() + pngFiles.size());

    for (auto it = bmpFiles.begin(); it != bmpFiles.end(); ++it)
        filesToLoad.push_back((*it).pathName());

    for (auto it = pngFiles.begin(); it != pngFiles.end(); ++it)
    {
        std::string stem = filenameStem((*it).pathName().filename());
        std::transform(stem.begin(), stem.end(), stem.begin(), ::tolower);
        if (bmpStems.find(stem) == bmpStems.end())
            filesToLoad.push_back((*it).pathName());
    }

    textures_.reserve(filesToLoad.size());

    size_t filesRead = 0;
    size_t numFiles = filesToLoad.size();
    size_t reportWhenFilesRead = 1;

    for (auto it = filesToLoad.begin(); it != filesToLoad.end(); ++it)
    {
        if (pReporter)
        {
            ++filesRead;
            if (filesRead == reportWhenFilesRead)
            {
                size_t inc = pReporter->report(filesRead, numFiles);
                if (inc == 0)
                    inc = 1;
                reportWhenFilesRead += inc;
            }
        }

        const SysPathName& fileName = *it;
        // Check if the file has alpha or colour map naming convention —
        // RenITexBody::read will sort out which bitmap effectively needs to be loaded
        if (! isAlphaMap(fileName) && ! isColourMap(fileName))
        {
            TEXSET_STREAM(" (" << textures_.size() << ") preloading texture " << fileName.pathname() << std::endl);
            // load a texture and save the texture handle in textures_
            // Note: This call does not make use of the directory search
            // mechanism since (*i) refers to an absolute pathname (see
            // RenISurfaceManagerImpl::createSurfOrTex)
            RenTexture texture = RenSurfaceManager::instance().createTexture(fileName.pathname());
            textures_.push_back(texture);
        }
    }

    TEXSET_STREAM("Loaded " << textures_.size() << " texture files" << std::endl);
    isLoaded_ = true;

    TEXSET_STREAM(RenSurfaceManager::instance());
    TEXSET_INDENT(-2);
    TEXSET_STREAM("Done preloading textures from directory " << directory << std::endl);
}

void RenTextureSetImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const RenTextureSetImpl& t)
{

    o << "RenTextureSetImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "RenTextureSetImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End TEXSETI.CPP **************************************************/
