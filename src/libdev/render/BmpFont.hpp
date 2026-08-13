/*
 * B M P F O N T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    Ren::BmpFont

    Support for bitmap fonts. Characters can be different widths so you can set the font as
    proportional or fixed spacing. At present, this font only draws text in a single line, it
    will ignore carridge return etc.
*/

#pragma once

#include "base/base.hpp"
#include "render/Alignment.hpp"
#include "render/Colour.hpp"
#include "render/render.hpp"
#include "system/PathName.hpp"

#include <string_view>

#include <cstddef>

class RenSurface;

struct BmpFontCore;

namespace Ren {

class Painter;

// What a line of a bitmap font is made of, in the measurements of the unscaled art.
// The ascender is the rise above the baseline and the descender the drop below it,
// negative, so that the height is the two apart. The cap height is how far a capital
// letter reaches above the baseline.
struct BmpFontMetrics
{
    int ascender{};
    int descender{};
    int capHeight{};
};

class BmpFont
// Canonical form revoked
{
public:
    enum FontType
    {
        PROPORTIONAL,
        FIXED_SPACE
    };

    BmpFont(); // Default constructor to satisfy std::vector. DO NOT USE!!!

    // The scale the atlas was produced at, which says how many lines at the bottom of
    // it are the undisplayable end-of-character markers rather than glyph. An atlas is
    // drawn by hand and scaled by hand, so its metrics are given by hand too; given
    // none, a line is taken to be all ascender and all capital.
    explicit BmpFont(const SysPathName& fontPath, std::size_t scale = 1, const BmpFontMetrics& metrics = {});
    BmpFont(const BmpFont&);
    BmpFont& operator=(const BmpFont&);
    ~BmpFont();

    size_t height() const;
    int ascender() const;
    int descender() const;
    int capHeight() const;

    size_t charWidth(char c) const;

    // The maximum width of a character in the font. The spacing that will be used by
    // Fixed space fonts
    size_t maxCharWidth() const;

    // The font's glyph bitmap atlas.
    const RenSurface& fontBitmap() const;

    // Horizontal offset (in pixels) of glyph for character c in the atlas.
    size_t charOffset(unsigned char c) const;

    const RenColour& underlineColour() const;

    // Get/Set font type ( proportional or fixed space ).
    FontType fontType() const;
    void fontType(FontType);

    // Set/Get width of space character. Note that the "spacing" is still used after a space char is displayed.
    size_t spaceCharWidth() const;
    void spaceCharWidth(size_t);

    // Set/Get spacing between characters
    size_t spacing() const;
    void spacing(size_t);

    // Switch underline on/off
    void underline(bool);
    bool underline() const;
    void underlineColour(RenColour colour);

    // Return the display width of "text" without actually rendering the text.
    int horizontalAdvance(const std::string_view& text) const;

    const SysPathName& fontPath() const;

    static char redCharIndex();
    static char greenCharIndex();
    static char blueCharIndex();
    static char yellowCharIndex();

    static char arrowUpIndex();
    static char arrowDownIndex();
    static char arrowLeftIndex();
    static char arrowRightIndex();

    static char healthPointsIndex(); // 1980 // 0x9d
    static char armorPointsIndex(); // 1990 // 0x80
    static char bmuPointsIndex(); // 1991 // 0x81
    static char bmuMinedPointsIndex(); // 1992 // 0x99
    static char researchPointsIndex(); // 2012 // 0x9e

private:
    void CLASS_INVARIANT;

    friend class Painter;
    friend std::ostream& operator<<(std::ostream& o, const BmpFont& t);

    BmpFontCore* pFontCore_ = nullptr;
    FontType fontType_ = PROPORTIONAL;
    size_t spaceCharWidth_{};
    size_t spacing_{};
    bool underline_ = false;
    RenColour underlineColour_{};
};

} // namespace Ren
