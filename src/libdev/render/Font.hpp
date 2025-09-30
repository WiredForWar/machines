#ifndef RENDER_FONT_HPP
#define RENDER_FONT_HPP

#include <memory>
#include <string>

namespace Render
{

class FontImpl;

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

    static const Font* getFont(int pixelSize);
    static const Font* getFont(const std::string& fontName, int pixelSize);

    static const std::string& getDefaultFontName();

protected:
    friend class Render::FontImpl;
    std::unique_ptr<FontImpl> pImpl_;
};

} // Render namespace

#endif // RENDER_FONT_HPP
