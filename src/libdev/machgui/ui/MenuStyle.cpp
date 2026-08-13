#include "MenuStyle.hpp"

#include "machgui/gui.hpp"
#include "render/BmpFont.hpp"
#include "render/Font.hpp"
#include "render/TextOptions.hpp"

namespace
{

struct MenuStyle
{
    std::string smallFontDark;
    std::string smallFontLight;
    std::string smallFontWhite;
};

MenuStyle& menuStyle()
{
    static MenuStyle style;
    return style;
}

} // namespace

namespace MachGui::Menu
{

const std::string& smallFontDark()
{
    return menuStyle().smallFontDark;
}

const std::string& smallFontLight()
{
    return menuStyle().smallFontLight;
}

const std::string& smallFontWhite()
{
    return menuStyle().smallFontWhite;
}

void updateFonts()
{
    // Measured off the unscaled art. The atlas is six lines tall and its capitals
    // fill the bottom five, the top line being there for the handful of glyphs that
    // reach higher.
    constexpr Ren::BmpFontMetrics smallMetrics{.ascender = 6, .descender = 0, .capHeight = 5};

    menuStyle().smallFontDark = getScaledImagePath("gui/menu/smaldfnt.bmp");
    menuStyle().smallFontLight = getScaledImagePath("gui/menu/smallfnt.bmp");
    menuStyle().smallFontWhite = getScaledImagePath("gui/menu/smalwfnt.bmp");

    Gui::setFontMetrics(menuStyle().smallFontDark, smallMetrics);
    Gui::setFontMetrics(menuStyle().smallFontLight, smallMetrics);
    Gui::setFontMetrics(menuStyle().smallFontWhite, smallMetrics);
}

const Gui::Colour highlightedTextColor()
{
    static const Gui::Colour c = Gui::Colour::fromString("#101863").value_or(Gui::Colour{});
    return c;
}

const Gui::Colour lightTextColor()
{
    static const Gui::Colour c = Gui::Colour::fromString("#e5dd95").value_or(Gui::Colour{});
    return c;
}

const Gui::Colour focusedTextColor()
{
    static const Gui::Colour c = Gui::Colour::fromString("#f4e900").value_or(Gui::Colour{});
    return c;
}

const Ren::Font& font()
{
    static const int pxSize = MachGui::menuScaleFactor() > 1 ? 35 : 18;
    static const Ren::Font *font = Ren::Font::getFont("U001/u001con-bol", pxSize);
    ASSERT(font, "Unable to load menu font");
    return *font;
}

const Ren::TextOptions& menuLightTextOptions()
{
    static Ren::TextOptions options(lightTextColor());
    options.setLetterSpacing(MachGui::menuScaleFactor() > 1 ? 3 : 1);

    return options;
}

} // namespace MachGui::Menu
