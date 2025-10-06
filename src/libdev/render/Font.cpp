#include "internal/FontImpl.hpp"

#include "render/Font.hpp"
#include "render/FontsManager.hpp"

#include "spdlog/spdlog.h"

#include <GL/glew.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

// FONTS
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Render
{

// Maximum font texture width
constexpr int MAXWIDTH = 1024;

static std::unique_ptr<FontsManager> fontsManager;

void initFonts()
{
    spdlog::info("Initializing FreeType library...");
    fontsManager.reset(new FontsManager);
}

void cleanUpFonts()
{
    spdlog::info("Deinitializing FreeType library...");
    fontsManager.reset();
}

/* Create texture atlasses for font sizes */
bool FontImpl::prepareTexture()
{
    if (!fontsManager)
        return false;

    FT_Face face = fontsManager->getFace(fontName);
    if (!face)
        return false;

    FT_Set_Pixel_Sizes(face, 0, pixelSize);
    FT_GlyphSlot g = face->glyph;

    unsigned int roww = 0;
    unsigned int rowh = 0;

    // Include as many characters as can fit 1 byte to give a chance for more (pre UTF-8 of cource) translations
    // E.g. German translation which needs characters up to 252.
    constexpr uint32_t maxCharacter = std::numeric_limits<uint8_t>::max();

    memset(charData_, 0, sizeof(charData_));

    /* Find minimum size for a texture holding all required characters */
    for (uint32_t i = 32; i <= maxCharacter; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            spdlog::warn("Loading font character {} failed!", i);
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

    /* Create a texture that will be used to hold all required glyphs */
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
    std::vector<GLuint> rgbaBitmap;
    rgbaBitmap.resize(pixelSize * pixelSize);
    rowh = 0;

    for (uint32_t i = 32; i <= maxCharacter; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
            continue;

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
            rgbaBitmap.data());

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

    return true;
}

static std::vector<Font> s_fonts;

const FontImpl* FontImpl::get(const Font* parent)
{
    return parent->pImpl_.get();
}

const FontImpl::CharData* FontImpl::getChar(char c) const
{
    uint8_t index = c;
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
    pImpl_ = std::make_unique<FontImpl>();
    pImpl_->fontName = fontName;
    pImpl_->pixelSize = pixelSize;
    pImpl_->prepareTexture();
}

Font::Font(Font &&other)
    : pImpl_(std::move(other.pImpl_))
{
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
        Font newFont(fontName, pixelSize);
        if (newFont.isValid())
        {
            s_fonts.emplace_back(std::move(newFont));
            return &s_fonts.back();
        }
        else
        {
            spdlog::critical("Unavailable font is a fatal error. Aborting.");
            exit(1);
        }
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
