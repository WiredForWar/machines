#include "formats_support/factory/MeshLoaderFactory.hpp"

#ifdef HAS_AGT_LOADER
#include "formats_support/agt/AgtMeshLoader.hpp"
#endif

#ifdef HAS_XFILE_LOADER
#include "formats_support/xfile/XFileMeshLoader.hpp"
#endif

namespace FormatSupport {

std::vector<std::unique_ptr<IMeshLoader>> createMeshLoaders()
{
    std::vector<std::unique_ptr<IMeshLoader>> loaders;

#ifdef HAS_XFILE_LOADER
    loaders.push_back(std::make_unique<XFileMeshLoader>());
#endif

#ifdef HAS_AGT_LOADER
    loaders.push_back(std::make_unique<AgtMeshLoader>());
#endif

    return loaders;
}

} // namespace FormatSupport
