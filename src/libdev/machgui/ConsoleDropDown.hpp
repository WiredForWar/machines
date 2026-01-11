#pragma once

#include "gui/Displayable.hpp"

class MachInGameScreen;

class MachGuiConsoleDropDown : public GuiDisplayable
{
public:
    explicit MachGuiConsoleDropDown(GuiDisplayable* parent);

    void setViewportSize(Gui::Size size);

    void doDisplay() override;

    void setOpen(bool open);
    bool isOpen() const;
    void toggle();
    void close();

    bool doHandleKeyEvent(const GuiKeyEvent& event) override;

private:
    int borderThickness_{};
    bool open_{};
};
