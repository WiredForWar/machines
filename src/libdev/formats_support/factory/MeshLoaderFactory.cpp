#include "formats_support/factory/MeshLoaderFactory.hpp"

#ifdef HAS_GLTF_LOADER
#include "formats_support/gltf/GltfMeshLoader.hpp"
#endif

#ifdef HAS_XFILE_LOADER
#include "formats_support/xfile/XFileMeshLoader.hpp"
#endif

#ifdef HAS_AGT_LOADER
#include "formats_support/agt/AgtMeshLoader.hpp"
#endif

namespace FormatSupport {

std::vector<std::unique_ptr<IMeshLoader>> createMeshLoaders()
{
    std::vector<std::unique_ptr<IMeshLoader>> loaders;

    // Order matters: first registered = highest priority.
    // glTF is preferred over .x when both are available.
#ifdef HAS_GLTF_LOADER
    loaders.push_back(std::make_unique<GltfMeshLoader>());
#endif

#ifdef HAS_XFILE_LOADER
    loaders.push_back(std::make_unique<XFileMeshLoader>());
#endif

#ifdef HAS_AGT_LOADER
    loaders.push_back(std::make_unique<AgtMeshLoader>());
#endif

    return loaders;
}

} // namespace FormatSupport
