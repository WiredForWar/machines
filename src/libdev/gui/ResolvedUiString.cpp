#include "ResolvedUiString.hpp"

#include "gui/restring.hpp"

ResolvedUiString::ResolvedUiString(Gui::StringId stringId)
{
    const GuiResourceString resourceLabel(stringId);
    str_ = resourceLabel.asString();
}
