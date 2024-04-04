#include "ResolvedUiString.hpp"

#include "gui/restring.hpp"

ResolvedUiString::ResolvedUiString(unsigned int stringId)
{
    const GuiResourceString resourceLabel(stringId);
    str_ = resourceLabel.asString();
}
