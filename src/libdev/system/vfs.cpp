#include "vfs.hpp"

#include "system/pathname.hpp"

#include <vector>

namespace System
{

namespace
{
    std::vector<std::string>& OverrideLocations()
    {
        static std::vector<std::string> locations;
        return locations;
    }
} // namespace

bool registerFsOverride(std::string extraPath)
{
    if (!SysPathName(extraPath).existsAsDirectory())
        return false;

    OverrideLocations().emplace_back(extraPath + "/");
    return true;
}

void clearFsOverrides()
{
    OverrideLocations().clear();
}

std::string findFile(std::string path)
{
    for (const std::string& location : OverrideLocations())
    {
        std::string lookup = location + path;
        if (SysPathName::existsAsFile(lookup))
            return lookup;
    }

    return path;
}

} // namespace System
