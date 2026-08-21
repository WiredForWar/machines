#include "gui/Screenshots.hpp"

#include "render/Surface.hpp"

#include "spdlog/spdlog.h"

#include <filesystem>
#include <string_view>
#include <system_error>

#include <cstdio>

namespace Gui
{

namespace ScreenshotsImpl
{

namespace
{

constexpr std::string_view Directory = "screenshots";
constexpr std::string_view Extension = ".png";

// How many numbered names a prefix has before it runs out of them.
constexpr std::size_t NumberedNames = 100000;

// Long enough for anything descriptive, short enough that no filesystem will
// argue about it.
constexpr std::size_t LongestName = 64;

// Deliberately not std::isalnum, which answers for whatever locale is in force.
bool isAlphanumeric(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isNameCharacter(char c)
{
    return isAlphanumeric(c) || c == '_' || c == '.';
}

char toLowerAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool endsWithExtension(const std::string& fileName)
{
    if (fileName.size() < Extension.size())
        return false;

    const std::size_t start = fileName.size() - Extension.size();
    for (std::size_t index = 0; index != Extension.size(); ++index)
    {
        if (toLowerAscii(fileName[start + index]) != Extension[index])
            return false;
    }

    return true;
}

SysPathName pathIn(const std::string& fileName)
{
    return SysPathName(std::string(Directory) + "/" + fileName);
}

// Makes the directory screenshots go in, if it is not there yet. False if it
// could not be made, since there is then nowhere to write one.
bool prepareDirectory()
{
    std::error_code failure;
    std::filesystem::create_directory(Directory, failure);

    return !failure;
}

} // namespace

} // namespace ScreenshotsImpl

std::optional<std::string> screenshotNameComplaint(const std::string& fileName)
{
    using namespace ScreenshotsImpl;

    if (fileName.empty())
        return "A screenshot needs a name.";

    if (fileName.size() > LongestName)
        return "A screenshot name is at most " + std::to_string(LongestName) + " characters: " + fileName;

    if (!isAlphanumeric(fileName.front()))
        return "A screenshot name starts with a letter or a digit: " + fileName;

    for (char c : fileName)
    {
        if (!isNameCharacter(c))
        {
            return "A screenshot name takes letters, digits, underscores and dots only, so it cannot name a "
                   "directory to put one in: "
                + fileName;
        }
    }

    return std::nullopt;
}

SysPathName screenshotPath(const std::string& fileName)
{
    using namespace ScreenshotsImpl;

    return pathIn(endsWithExtension(fileName) ? fileName : fileName + std::string(Extension));
}

SysPathName nextScreenshotPath(const std::string& prefix)
{
    using namespace ScreenshotsImpl;

    for (std::size_t number = 0; number != NumberedNames; ++number)
    {
        char digits[8];
        std::snprintf(digits, sizeof(digits), "%04zu", number);

        const SysPathName path = pathIn(prefix + digits + std::string(Extension));
        if (! path.existsAsFile())
            return path;
    }

    return {};
}

bool saveScreenshot(const RenSurface& surface, const SysPathName& path, const Ren::Rect& area)
{
    using namespace ScreenshotsImpl;

    if (! path.set())
    {
        spdlog::warn("No screenshot was written: there was nowhere left to write one");
        return false;
    }

    if (!prepareDirectory() || !surface.saveAsPng(path, area))
    {
        spdlog::warn("The screenshot '{}' could not be written", path.pathname());
        return false;
    }

    spdlog::info("Wrote the screenshot '{}'", path.pathname());
    return true;
}

} // namespace Gui
