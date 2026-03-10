#include "system/VFS.hpp"

#include "system/PathName.hpp"

#include <algorithm>
#include <filesystem>
#include <ranges>
#include <vector>

namespace System
{

static std::vector<std::string> overrideLocations;

void addFsOverride(std::string extraPath)
{
    overrideLocations.emplace_back(extraPath + "/");
}

std::string findFile(std::string_view path)
{
    for (std::string_view location : overrideLocations)
    {
        std::string lookup;
        lookup.reserve(location.size() + path.size());
        lookup.append(location);
        lookup.append(path);

        if (SysPathName::existsAsFile(lookup))
            return lookup;
    }

    return std::string(path);
}

std::vector<std::string> listAvailableMods()
{
    constexpr char ModsDirName[] = "mods";
    std::filesystem::create_directory(ModsDirName);

    std::vector<std::string> result;

    try
    {
        // Iterate over each entry in the directory
        for (const auto& entry : std::filesystem::directory_iterator(ModsDirName))
        {
            if (entry.is_directory())
            {
                result.push_back(entry.path().string());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        return {};
    }

    return result;
}

std::vector<std::string> listMods()
{
    std::vector<std::string> mods = listAvailableMods();
    std::sort(mods.begin(), mods.end());
    return mods;
}

std::vector<std::string> getFileOverrides(std::string path)
{
    std::vector<std::string> result;

    if (SysPathName::existsAsFile(path))
        result.emplace_back(path);

    for (const std::string& location : std::ranges::reverse_view(overrideLocations))
    {
        std::string lookup = location + path;
        if (SysPathName::existsAsFile(lookup))
            result.emplace_back(lookup);
    }

    return result;
}

} // namespace System
