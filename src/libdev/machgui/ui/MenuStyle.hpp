#include "gui/gui.hpp"

#include <string>

namespace MachGui::Menu
{

const std::string& smallFontDark();
const std::string& smallFontLight();
const std::string& smallFontWhite();

const Gui::Colour highlightedTextColor();
const Gui::Colour lightTextColor();
const Gui::Colour focusedTextColor();

const Ren::Font& font();
const Ren::TextOptions& menuLightTextOptions();

void updateFonts();

} // namespace MachGui::Menu
