#include "machgui/SaveGames.hpp"

#include <filesystem>
#include <string>
#include <string_view>

#include <cstdio>

namespace MachGui
{

namespace SaveGamesImpl
{

namespace
{

constexpr std::string_view Directory = "savegame";

} // namespace

} // namespace SaveGamesImpl

void prepareSaveGameDirectory()
{
    using namespace SaveGamesImpl;

    std::filesystem::create_directory(Directory);
}

SysPathName nextSaveGamePath()
{
    using namespace SaveGamesImpl;

    for (std::size_t number = 0;; ++number)
    {
        char digits[20];
        std::snprintf(digits, sizeof(digits), "%04zu", number);

        SysPathName path(std::string(Directory) + "/save" + digits + ".sav");
        if (! path.existsAsFile())
            return path;
    }
}

} // namespace MachGui
