#ifndef GUI_ACTION_HPP
#define GUI_ACTION_HPP

#include <string>

class GuiKeyEvent;

class GuiKeySequence
{
public:
    GuiKeySequence(const std::string& key);

    bool matches(const GuiKeyEvent& event) const;

private:
    int modifiers_ = 0;
};

class GuiAction
{
public:
    void shortcut() const;
    void setShortcut(GuiKeySequence shortcut);
};

#endif // GUI_ACTION_HPP
