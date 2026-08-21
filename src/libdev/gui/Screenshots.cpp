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

// How many numbered names a prefix has before it runs out of them.
constexpr std::size_t NumberedNames = 100000;

// Makes the screenshot directory if it is not there yet. False if it could not
// be made, since there is then nowhere to write.
bool prepareDirectory()
{
    std::error_code failure;
    std::filesystem::create_directory(Directory, failure);

    return ! failure;
}

SysPathName pathIn(const std::string& fileName)
{
    return SysPathName(std::string(Directory) + "/" + fileName);
}

} // namespace

} // namespace ScreenshotsImpl

SysPathName screenshotPath(const std::string& fileName)
{
    using namespace ScreenshotsImpl;

    if (! prepareDirectory())
        return {};

    return pathIn(fileName);
}

SysPathName nextScreenshotPath(const std::string& prefix)
{
    using namespace ScreenshotsImpl;

    if (! prepareDirectory())
        return {};

    for (std::size_t number = 0; number != NumberedNames; ++number)
    {
        char digits[8];
        std::snprintf(digits, sizeof(digits), "%04zu", number);

        const SysPathName path = pathIn(prefix + digits + ".png");
        if (! path.existsAsFile())
            return path;
    }

    return {};
}

bool saveScreenshot(const RenSurface& surface, const SysPathName& path, const Ren::Rect& area)
{
    if (! path.set())
    {
        spdlog::warn("No screenshot was written: there was nowhere to write one");
        return false;
    }

    if (! surface.saveAsPng(path, area))
    {
        spdlog::warn("The screenshot '{}' could not be written", path.pathname());
        return false;
    }

    spdlog::info("Wrote the screenshot '{}'", path.pathname());
    return true;
}

} // namespace Gui
