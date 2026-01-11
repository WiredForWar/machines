#pragma once

#include "gui/displaya.hpp"

#include "gui/displaya.hpp"

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

    bool doHandleKeyEvent(const GuiKeyEvent& event) override;

private:
    int borderThickness_{};
    bool open_{};
};
