#include "MenuStyle.hpp"

#include "machgui/gui.hpp"

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

} // namespace MachGui::Menu
