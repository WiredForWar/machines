#include "MenuStyle.hpp"

#include "machgui/gui.hpp"
#include "render/Font.hpp"
#include "render/TextOptions.hpp"

namespace
{

struct MenuStyle
{
    std::string smallFontDark;
    std::string smallFontLight;
    std::string smallFontWhite;

    std::string largeFontDark;
    std::string largeFontLight;
    std::string largeFontFocus;
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

const std::string& largeFontDark()
{
    return menuStyle().largeFontDark;
}

const std::string& largeFontLight()
{
    return menuStyle().largeFontLight;
}

const std::string& largeFontFocus()
{
    return menuStyle().largeFontFocus;
}

void updateFonts()
{
    menuStyle().smallFontDark = getScaledImagePath("gui/menu/smaldfnt.bmp");
    menuStyle().smallFontLight = getScaledImagePath("gui/menu/smallfnt.bmp");
    menuStyle().smallFontWhite = getScaledImagePath("gui/menu/smalwfnt.bmp");

    menuStyle().largeFontDark = getScaledImagePath("gui/menu/largdfnt.bmp");
    menuStyle().largeFontLight = getScaledImagePath("gui/menu/largefnt.bmp");
    menuStyle().largeFontFocus = getScaledImagePath("gui/menu/largyfnt.bmp");
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
