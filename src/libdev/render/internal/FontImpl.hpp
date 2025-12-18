#pragma once

#include <GL/glew.h>

#include <string>

#include <cstdint>

namespace Render
{

class Font;

class FontImpl
{
public:
    FontImpl() = default;

    static const FontImpl* get(const Font& parent);
    static const FontImpl* get(const Font* parent);

    bool prepareTexture();

    int pixelSize = 0;

    std::string fontName;
    GLuint textureId = 0; // texture atlas object

    unsigned int w = 0; // width of texture in pixels
    unsigned int h = 0; // height of texture in pixels

    struct CharData
    {
        float ax{}; // advance.x
        float ay{}; // advance.y

        int bw{}; // bitmap.width;
        int bh{}; // bitmap.height;

        int bl{}; // bitmap_left;
        int bt{}; // bitmap_top;

        float tx{}; // x offset of glyph in texture coordinates
        float ty{}; // y offset of glyph in texture coordinates
        float tx2{};
        float ty2{};
    };

    const CharData* getChar(int32_t c) const;

protected:
    CharData charData_[256]; // character information
};

} // Render namespace
