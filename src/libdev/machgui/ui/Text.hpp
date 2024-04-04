/*
 * T E X T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiText

    A brief description of the class should go in here
*/

#pragma once

#include "gui/displaya.hpp"
#include "gui/font.hpp"

class GuiRoot;

class MachGuiText : public GuiDisplayable
{
public:
    MachGuiText(GuiDisplayable* pParent, size_t width, const std::string& str);
    MachGuiText(GuiDisplayable* pParent, size_t width, const std::string& str, const std::string& font);

    static size_t reqHeight();

    void doDisplay() override;

    void setTextOffset(Gui::Coord offset);

private:
    std::string text_;
    Gui::Coord textOffset_;
    GuiBmpFont font_;

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
};
