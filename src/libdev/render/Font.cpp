#include "internal/FontImpl.hpp"

#include "render/Font.hpp"
#include "system/pathname.hpp"

#include <algorithm>
#include <ostream>
#include <vector>

#include <GL/glew.h>

// FONTS
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Render
{

// Maximum font texture width
constexpr int MAXWIDTH = 1024;

/* Create texture atlasses for font sizes */
void FontImpl::prepareTexture()
{
    FT_Library ft;
    FT_Face face;
    /* Initialize the FreeType2 library */
    FT_Error result = FT_Init_FreeType(&ft);
    if (result)
    {
        std::cerr << "Could not init freetype library!"
                  << " FT_Error: " << result << std::endl;
        return;
    }

    /* Load a font */
    SysPathName fontFile("gui/" + (fontName + ".ttf"));
    result = FT_New_Face(ft, fontFile.pathname().c_str(), 0, &face);
    if (result)
    {
        std::cerr << "Could not open font " << fontFile << "."
                  << " FT_Error: " << result << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, pixelSize);
    FT_GlyphSlot g = face->glyph;

    unsigned int roww = 0;
    unsigned int rowh = 0;

    memset(charData_, 0, sizeof(charData_));

    /* Find minimum size for a texture holding all visible ASCII characters */
    for (int i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            std::cerr << "Loading font character " << i << " failed!" << std::endl;
            continue;
        }
        if (roww + g->bitmap.width + 1 >= MAXWIDTH)
        {
            w = std::max(w, roww);
            h += rowh;
            roww = 0;
            rowh = 0;
        }
        roww += g->bitmap.width + 1;
        rowh = std::max(rowh, g->bitmap.rows);
    }

    w = std::max(w, roww);
    h += rowh;

    /* Create a texture that will be used to hold all ASCII glyphs */
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    /* We require 1 byte alignment when uploading texture data */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Clamping to edges is important to prevent artifacts when scaling */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Linear filtering usually looks best for text */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Paste all glyph bitmaps into the texture, remembering the offset */
    int ox = 0;
    int oy = 0;
    GLuint* rgbaBitmap = _NEW_ARRAY(GLuint, pixelSize * pixelSize);
    rowh = 0;

    for (int i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            std::cerr << "Loading font character " << i << " failed!" << std::endl;
            continue;
        }

        if (ox + g->bitmap.width + 1 >= MAXWIDTH)
        {
            oy += rowh;
            rowh = 0;
            ox = 0;
        }
        for (int j = 0; j < g->bitmap.width * g->bitmap.rows; ++j)
        {
            rgbaBitmap[j] = (g->bitmap.buffer[j] << 24) | 0x00FFFFFF;
        }
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            ox,
            oy,
            g->bitmap.width,
            g->bitmap.rows,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            rgbaBitmap);

        CharData& data = charData_[i];
        data.ax = g->advance.x >> 6;
        data.ay = g->advance.y >> 6;

        data.bw = g->bitmap.width;
        data.bh = g->bitmap.rows;

        data.bl = g->bitmap_left;
        data.bt = g->bitmap_top;

        data.tx = ox / (float)w;
        data.ty = oy / (float)h;
        data.tx2 = data.tx + data.bw / (float)w;
        data.ty2 = data.ty + data.bh / (float)h;

        rowh = std::max(rowh, g->bitmap.rows);
        ox += g->bitmap.width + 1;
    }
    _DELETE_ARRAY(rgbaBitmap);
}

static std::vector<Font> s_fonts;

const FontImpl* FontImpl::get(const Font* parent)
{
    return parent->pImpl_;
}

const FontImpl::CharData* FontImpl::getChar(char c) const
{
    int index = c;
    return &charData_[index];
}

/**
 * Code taken from modern opengl tutorials.
 * The struct holds a texture atlas that contains the visible US-ASCII characters
 * of a certain font rendered with a certain character height.
 * It also contains an array that contains all the information necessary to
 * generate the appropriate vertex and texture coordinates for each character.
 *
 * After the constructor is run, you don't need to use any FreeType functions anymore.
 */
Font::Font(const std::string& fontName, int pixelSize)
{
    pImpl_ = new FontImpl();
    pImpl_->fontName = fontName;
    pImpl_->pixelSize = pixelSize;
    pImpl_->prepareTexture();
}

Font::~Font()
{
    // glDeleteTextures(1, &tex);
}

bool Font::isValid() const
{
    return pImpl_ && pImpl_->textureId;
}

int Font::pixelSize() const
{
    return pImpl_ ? pImpl_->pixelSize : 0;
}

const Font* Font::getFont(int pixelSize)
{
    return getFont(getDefaultFontName(), pixelSize);
}

const Render::Font* Font::getFont(const std::string& fontName, int pixelSize)
{
    const auto it = std::find_if(s_fonts.cbegin(), s_fonts.cend(), [&](const Font& fontImpl) {
        if (fontImpl.pixelSize() == pixelSize)
            return true;
        return false;
    });

    if (it == s_fonts.cend())
    {
        s_fonts.emplace_back(Font(fontName, pixelSize));
        return &s_fonts.back();
    }
    else
    {
        return &*it;
    }
}

const std::string& Font::getDefaultFontName()
{
    static bool first = true;
    static std::string fn = "Arial";

    if (first)
    {
        first = false;

        const char* envVar = getenv("CB_RENDER_FONT");
        if (envVar)
        {
            char* copy = strdup(envVar);
            fn = strtok(copy, ":");
            free(copy);
        }
    }

    return fn;
}

} // Render namespace
