#ifndef RENDER_FONT_HPP
#define RENDER_FONT_HPP

#include <memory>
#include <string>
#include <string_view>

#include "render/render.hpp"

namespace Ren
{

class FontImpl;
class TextOptions;

void initFonts();
void cleanUpFonts();
void reloadFonts();

class Font
// Canonical form revoked
{
public:
    Font(const std::string& fontName, int pixelSize);
    Font(Font&& other);

    virtual ~Font();

    bool isValid() const;
    void reload();
    int pixelSize() const;
    int height() const;

    int charWidth(uint32_t character) const;

    Size boundingSize(const std::string_view& text, const TextOptions &options) const;
    Size boundingSize(const std::string_view& text) const;

    int horizontalAdvance(const std::string_view& text, const TextOptions &options) const;
    int horizontalAdvance(const std::string_view& text) const;

    static const Font* getFont(int pixelSize);
    static const Font* getFont(const std::string& fontName, int pixelSize);

    static const std::string& getDefaultFontName();

protected:
    friend class Ren::FontImpl;
    std::unique_ptr<FontImpl> pImpl_;
};

} // namespace Ren

#endif // RENDER_FONT_HPP
