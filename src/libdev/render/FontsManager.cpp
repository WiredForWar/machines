#include "FontsManager.hpp"

#include "system/pathname.hpp"

#include "spdlog/spdlog.h"

namespace Render
{

FontsManager::FontsManager()
{
    init();
}

FontsManager::~FontsManager()
{
    for (const auto& v : faces_)
        FT_Done_Face(v.second);

    FT_Done_FreeType(library_);
}

bool FontsManager::init()
{
    FT_Error result = FT_Init_FreeType(&library_);
    if (result)
    {
        spdlog::critical("Could not init freetype library! FT_Error: {}", result);
        return false;
    }

    {
        struct Version
        {
            int Major {};
            int Minor {};
            int Patch {};

            bool operator==(const Version& another) const = default;
        };

        Version runtimeV;
        FT_Library_Version(library_, &runtimeV.Major, &runtimeV.Minor, &runtimeV.Patch);

        constexpr Version compiledWithVersion { FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH };
        if (runtimeV == compiledWithVersion)
            spdlog::info("FreeType version: {}.{}.{}", runtimeV.Major, runtimeV.Minor, runtimeV.Patch);
        else
            spdlog::info(
                "FreeType version: {}.{}.{} (compiled with {}.{}.{})",
                runtimeV.Major,
                runtimeV.Minor,
                runtimeV.Patch,
                compiledWithVersion.Major,
                compiledWithVersion.Minor,
                compiledWithVersion.Patch);
    }

    return true;
}

FT_Face FontsManager::getFace(const std::string& fontName)
{
    const auto faceIt = faces_.find(fontName);
    if (faceIt != faces_.end())
        return faceIt->second;

    FT_Face face{};
    std::string fontFile("gui/" + fontName + ".ttf");
    if (!SysPathName::existsAsFile(fontFile))
        fontFile = "gui/fonts/" + fontName + ".ttf";

    FT_Error initResult_ = FT_New_Face(library_, fontFile.c_str(), 0, &face);
    if (initResult_)
    {
        spdlog::critical("Could not open font {}. FT_Error: {}", fontFile, initResult_);
        return {};
    }

    faces_[fontName] = face;
    return face;
}

} // namespace Render
