#ifndef GUI_TTF_FONT_IMPL_HPP
#define GUI_TTF_FONT_IMPL_HPP

#include "render/Font.hpp"

#include <GL/glew.h>

namespace Render
{

class Font;

class FontImpl
{
public:
    FontImpl() = default;

    static const FontImpl* get(const Font* parent);

    void prepareTexture();

    int pixelSize = 0;

    std::string fontName;
    GLuint textureId = 0; // texture atlas object

    unsigned int w = 0; // width of texture in pixels
    unsigned int h = 0; // height of texture in pixels

    struct CharData
    {
        float ax; // advance.x
        float ay; // advance.y

        int bw; // bitmap.width;
        int bh; // bitmap.height;

        int bl; // bitmap_left;
        int bt; // bitmap_top;

        float tx; // x offset of glyph in texture coordinates
        float ty; // y offset of glyph in texture coordinates
        float tx2, ty2;
    };

    const CharData* getChar(char c) const;

protected:
    CharData charData_[128]; // character information
};

} // Render namespace

#endif // GUI_TTF_FONT_IMPL_HPP
