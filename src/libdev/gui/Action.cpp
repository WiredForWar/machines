#include "Action.hpp"
#include "device/key.hpp"

GuiKeySequence::GuiKeySequence(const std::string& key)
{
}

bool GuiKeySequence::matches(const GuiKeyEvent& event) const
{
    return false;
}

void GuiAction::shortcut() const
{
}

void GuiAction::setShortcut(GuiKeySequence shortcut)
{
}
