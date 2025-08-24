#include "vfs.hpp"

#include "system/pathname.hpp"

#include <vector>

namespace System
{

static std::vector<std::string> overrideLocations;

void registerFsOverride(std::string extraPath)
{
    overrideLocations.emplace_back("overrides/" + extraPath + "/");
}

std::string findFile(std::string path)
{
    for (const std::string& location : overrideLocations)
    {
        std::string lookup = location + path;
        if (SysPathName::existsAsFile(lookup))
            return lookup;
    }

    return path;
}

} // namespace System
