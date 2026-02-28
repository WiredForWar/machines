#include "formats_support/factory/MeshLoaderFactory.hpp"

namespace FormatSupport {

std::vector<std::unique_ptr<IMeshLoader>> createMeshLoaders()
{
    std::vector<std::unique_ptr<IMeshLoader>> loaders;

    return loaders;
}

} // namespace FormatSupport
