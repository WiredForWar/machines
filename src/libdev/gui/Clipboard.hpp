#pragma once

#include <string>

class GuiClipboard
{
public:
    static GuiClipboard& instance();

    std::string getText() const;

private:
    GuiClipboard() = default;
    GuiClipboard(const GuiClipboard&) = delete;
    GuiClipboard& operator=(const GuiClipboard&) = delete;
};
