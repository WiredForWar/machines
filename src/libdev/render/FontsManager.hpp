#pragma once

#include <map>
#include <string>

// FONTS
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Ren
{

class FontsManager
{
public:
    FontsManager();
    ~FontsManager();

    FT_Face getFace(const std::string& fontName);

private:
    bool init();

    FT_Library library_{};
    std::map<std::string, FT_Face> faces_;
};

} // namespace Ren
