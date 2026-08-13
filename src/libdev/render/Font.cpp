#include "internal/FontImpl.hpp"

#include "render/Font.hpp"
#include "render/FontsManager.hpp"
#include "render/TextOptions.hpp"

#include "render/Device.hpp"
#include "render/Surface.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

// FONTS
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Ren
{

// Maximum font texture width
constexpr int MAXWIDTH = 1024;

static std::unique_ptr<FontsManager> fontsManager;
static std::vector<std::unique_ptr<Font>> s_fonts;

void initFonts()
{
    spdlog::info("Initializing FreeType library...");
    fontsManager.reset(new FontsManager);
}

void cleanUpFonts()
{
    spdlog::info("Deinitializing FreeType library...");
    s_fonts.clear();
    fontsManager.reset();
}

void reloadFonts()
{
    spdlog::info("Reloading font atlases...");
    for (auto& font : s_fonts)
    {
        if (font)
            font->reload();
    }
}

FontImpl::~FontImpl() = default;

/* Create texture atlasses for font sizes */
bool FontImpl::prepareTexture()
{
    if (!fontsManager)
        return false;

    RenDevice* const dev = RenDevice::current();
    if (!dev)
        return false;

    FT_Face face = fontsManager->getFace(fontName);
    if (!face)
        return false;

    FT_Set_Pixel_Sizes(face, 0, pixelSize);

    ascender_ = static_cast<int>(face->size->metrics.ascender >> 6);
    descender_ = static_cast<int>(face->size->metrics.descender >> 6);
    lineHeight_ = static_cast<int>(face->size->metrics.height >> 6);
    FT_GlyphSlot g = face->glyph;

    unsigned int roww = 0;
    unsigned int rowh = 0;

    w = 0;
    h = 0;

    // Include as many characters as can fit 1 byte to give a chance for more (pre UTF-8 of cource) translations
    // E.g. German translation which needs characters up to 252.
    constexpr uint32_t maxCharacter = std::numeric_limits<uint8_t>::max();

    std::fill(std::begin(charData_), std::end(charData_), CharData{});

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

    atlasSurface = std::make_unique<RenSurface>(RenSurface::createAnonymousSurface(Ren::Size(w, h)));
    textureId = atlasSurface->handle();

    /* Paste all glyph bitmaps into the texture, remembering the offset */
    int ox = 0;
    int oy = 0;
    rowh = 0;

    std::vector<uint> atlasPixels;
    atlasPixels.resize(static_cast<std::size_t>(w) * static_cast<std::size_t>(h));

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

        for (int y = 0; y < static_cast<int>(g->bitmap.rows); ++y)
        {
            for (int x = 0; x < static_cast<int>(g->bitmap.width); ++x)
            {
                const std::size_t dstX = static_cast<std::size_t>(ox + x);
                const std::size_t dstY = static_cast<std::size_t>(oy + y);
                const std::size_t dstIdx = dstY * static_cast<std::size_t>(w) + dstX;
                const uint8_t alpha = g->bitmap.buffer[y * g->bitmap.width + x];
                atlasPixels[dstIdx] = (static_cast<uint>(alpha) << 24) | 0x00FFFFFF;
            }
        }

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

    atlasSurface->copyFromRGBABuffer(atlasPixels.data());

    return true;
}

const FontImpl* FontImpl::get(const Font& parent)
{
    return parent.pImpl_.get();
}

const FontImpl* FontImpl::get(const Font* parent)
{
    return parent->pImpl_.get();
}

const FontImpl::CharData* FontImpl::getChar(int32_t c) const
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

void Font::reload()
{
    if (pImpl_)
        pImpl_->prepareTexture();
}

int Font::pixelSize() const
{
    return pImpl_ ? pImpl_->pixelSize : 0;
}

int Font::height() const
{
    return pImpl_ ? pImpl_->height() : 0;
}

int Font::ascender() const
{
    return pImpl_ ? pImpl_->ascender() : 0;
}

int Font::descender() const
{
    return pImpl_ ? pImpl_->descender() : 0;
}

int Font::capHeight() const
{
    if (! pImpl_)
        return 0;

    // How high a capital is drawn above the baseline is how high the capital H is,
    // which the atlas already knows. Failing that, a line is taken to be all capital.
    const Ren::FontImpl::CharData* charData = pImpl_->getChar('H');

    return (charData && charData->bt) ? charData->bt : pImpl_->ascender();
}

int Font::charWidth(uint32_t character) const
{
    const Ren::FontImpl::CharData* charData = pImpl_->getChar(character);
    if (!charData)
        return 0;

    return charData->ax;
}

Size Font::boundingSize(const std::string_view& text, const TextOptions& options) const
{
    const Ren::FontImpl& font = *pImpl_;
    const Ren::FontImpl::CharData* charData {};

    int textWidth{};
    int textHeight = font.lineHeight();
    int lineTextWidth{};
    int usedSpacing{};

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        uint character = text[i];
        if (character == '\n')
        {
            textWidth = std::max<int>(textWidth, lineTextWidth - usedSpacing);
            lineTextWidth = 0;
            usedSpacing = 0;

            if (i != text.size() - 1)
                textHeight += font.lineHeight() + options.lineSpacing();

            continue;
        }

        charData = font.getChar(character);
        // Ignore missing characters
        if (!charData)
            continue;

        /* Advance the cursor to the start of the next character */
        lineTextWidth += charData->ax + options.letterSpacing();
        usedSpacing = options.letterSpacing();
    }
    textWidth = std::max<int>(textWidth, lineTextWidth - usedSpacing);
    if (options.hasShadow())
    {
        textWidth += options.shadowX();
    }

    return Size{textWidth, textHeight};
}

Size Font::boundingSize(const std::string_view& text) const
{
    return boundingSize(text, {});
}

int Font::horizontalAdvance(const std::string_view& text, const TextOptions& options) const
{
    return boundingSize(text, options).width;
}

int Font::horizontalAdvance(const std::string_view& text) const
{
    return boundingSize(text).width;
}

const Font* Font::getFont(int pixelSize)
{
    return getFont(getDefaultFontName(), pixelSize);
}

const Ren::Font* Font::getFont(const std::string& fontName, int pixelSize)
{
    const auto it = std::find_if(
        s_fonts.cbegin(),
        s_fonts.cend(),
        [pixelSize](const std::unique_ptr<Font>& font)
    {
        if (font->pixelSize() == pixelSize)
            return true;
        return false;
    });

    if (it == s_fonts.cend())
    {
        std::unique_ptr<Font> newFont = std::make_unique<Font>(fontName, pixelSize);
        if (newFont->isValid())
        {
            s_fonts.emplace_back(std::move(newFont));
            return s_fonts.back().get();
        }
        else
        {
            spdlog::critical("Unavailable font is a fatal error. Aborting.");
            exit(1);
        }
    }
    else
    {
        return it->get();
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

} // namespace Ren
