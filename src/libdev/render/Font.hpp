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

    // What a line of this font is made of. The ascender is the rise above the
    // baseline and the descender the drop below it, negative, so that the height is
    // the two apart. The cap height is how far a capital letter reaches, which is
    // what to centre on when the text has nothing hanging below the baseline.
    int height() const;
    int ascender() const;
    int descender() const;
    int capHeight() const;

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
