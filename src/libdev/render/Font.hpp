#ifndef RENDER_FONT_HPP
#define RENDER_FONT_HPP

#include <memory>
#include <string>

namespace Render
{

class FontImpl;
class TextOptions;

void initFonts();
void cleanUpFonts();

class Font
// Canonical form revoked
{
public:
    Font(const std::string& fontName, int pixelSize);
    Font(Font&& other);

    virtual ~Font();

    bool isValid() const;
    int pixelSize() const;
    int height() const;

    int charWidth(uint32_t character) const;

    int horizontalAdvance(const std::string& text, const TextOptions &options) const;
    int horizontalAdvance(const std::string& text) const;

    static const Font* getFont(int pixelSize);
    static const Font* getFont(const std::string& fontName, int pixelSize);

    static const std::string& getDefaultFontName();

protected:
    friend class Render::FontImpl;
    std::unique_ptr<FontImpl> pImpl_;
};

} // Render namespace

#endif // RENDER_FONT_HPP
