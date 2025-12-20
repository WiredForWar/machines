#include "gui/gui.hpp"

#include <string>

namespace MachGui::Menu
{

const std::string& smallFontDark();
const std::string& smallFontLight();
const std::string& smallFontWhite();

const std::string& largeFontDark();
const std::string& largeFontLight();
const std::string& largeFontFocus();

const Gui::Colour highlightedTextColor();
const Gui::Colour lightTextColor();
const Gui::Colour focusedTextColor();

const Render::Font& font();
const Render::TextOptions& menuLightTextOptions();

void updateFonts();

} // namespace MachGui::Menu
