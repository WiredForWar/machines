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
#include "render/colour.hpp"
#include "render/render.hpp"
#include "system/pathname.hpp"

#include <string_view>

class RenSurface;

struct BmpFontCore;

namespace Ren {

class Painter;

class BmpFont
// Canonical form revoked
{
public:
    enum Justification
    {
        LEFT_JUSTIFY,
        RIGHT_JUSTIFY
    };
    enum FontType
    {
        PROPORTIONAL,
        FIXED_SPACE
    };

    BmpFont(); // Default constructor to satisfy std::vector. DO NOT USE!!!
    explicit BmpFont(const SysPathName& fontPath);
    BmpFont(const BmpFont&);
    BmpFont& operator=(const BmpFont&);
    ~BmpFont();

    size_t height() const;

    size_t charWidth(char c) const;

    // The maximum width of a character in the font. The spacing that will be used by
    // Fixed space fonts
    size_t maxCharWidth() const;

    // Display text on screen via painter.
    void drawText(
        Painter& painter,
        const std::string_view& text,
        const Ren::Point& startPos,
        int maxWidth,
        Justification = LEFT_JUSTIFY) const;

    // Display text on bitmap.
    void drawText(
        RenSurface* pBmp,
        const std::string_view& text,
        const Ren::Point& startPos,
        int maxWidth,
        Justification = LEFT_JUSTIFY) const;

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
    void underlineColour(const RenColour&);

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

protected:
    void drawTextLeftJustify(
        Painter& painter,
        const std::string_view& text,
        const Ren::Point& startPos,
        int maxWidth) const;
    void drawTextRightJustify(
        Painter& painter,
        const std::string_view& text,
        const Ren::Point& startPos,
        int maxWidth) const;
    void drawTextLeftJustify(
        RenSurface* pBmp,
        const std::string_view& text,
        const Ren::Point& startPos,
        int maxWidth) const;
    void drawTextRightJustify(
        RenSurface* pBmp,
        const std::string_view& text,
        const Ren::Point& startPos,
        int maxWidth) const;

private:
    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const BmpFont& t);

    BmpFontCore* pFontCore_ = nullptr;
    FontType fontType_ = PROPORTIONAL;
    size_t spaceCharWidth_{};
    size_t spacing_{};
    bool underline_ = false;
    RenColour underlineColour_{};
};

} // namespace Ren
