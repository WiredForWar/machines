#ifndef RENDER_FONT_HPP
#define RENDER_FONT_HPP

#include <string>

namespace Render
{

class FontImpl;

class Font
// Canonical form revoked
{
public:
    Font(const std::string& fontName, int pixelSize);
    virtual ~Font();

    bool isValid() const;
    int pixelSize() const;

    static const Font* getFont(const std::string& fontName, int pixelSize);

    static const std::string& getDefaultFontName();

protected:
    friend class Render::FontImpl;
    FontImpl* pImpl_ = nullptr;
};

} // Render namespace

#endif // RENDER_FONT_HPP
