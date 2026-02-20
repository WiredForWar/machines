#pragma once

#include "render/render.hpp"

#include <memory>
#include <string>

#include <cstdint>

class RenSurface;

namespace Ren
{

class Font;

class FontImpl
{
public:
    FontImpl() = default;
    ~FontImpl();

    static const FontImpl* get(const Font& parent);
    static const FontImpl* get(const Font* parent);

    bool prepareTexture();

    int pixelSize = 0;

    int ascender() const { return ascender_; }
    int descender() const { return descender_; }
    int lineHeight() const { return lineHeight_ ? lineHeight_ : pixelSize; }
    int height() const { return (ascender_ || descender_) ? (ascender_ - descender_) : pixelSize; }

    std::string fontName;
    std::unique_ptr<RenSurface> atlasSurface{};
    Ren::TexId textureId{Ren::NullTexId};

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
    int ascender_ = 0;
    int descender_ = 0;
    int lineHeight_ = 0;
    CharData charData_[256]; // character information
};

} // namespace Ren
