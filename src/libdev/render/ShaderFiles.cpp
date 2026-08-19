#include "render/ShaderFiles.hpp"

#include "system/VFS.hpp"

#include "spdlog/spdlog.h"

#include <fstream>
#include <sstream>

namespace Ren
{

namespace
{

std::string_view extensionFor(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::Vertex:
        return ".vxgls";
    case ShaderStage::Fragment:
        return ".fggls";
    }

    return {};
}

} // namespace

std::optional<std::string> ShaderFiles::source(ShaderSet set, ShaderStage stage, std::string_view name) const
{
    std::string path(shaderDirectory(set));
    path += name;
    path += extensionFor(stage);

    const std::string resolved = System::findFile(path);

    std::ifstream stream(resolved, std::ios::in);
    if (!stream.is_open())
        return std::nullopt;

    spdlog::debug("Loading shader {}", resolved);

    std::ostringstream contents;
    contents << stream.rdbuf();

    return contents.str();
}

} // namespace Ren
