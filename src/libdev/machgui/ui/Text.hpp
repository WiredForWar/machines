/*
 * T E X T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiText

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_TEXT_HPP
#define _MACHGUI_TEXT_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "gui/displaya.hpp"
#include "gui/font.hpp"
#include "gui/root.hpp"

class MachGuiText : public GuiDisplayable
{
public:
    MachGuiText(GuiDisplayable* pParent, size_t width, const string& str);
    MachGuiText(GuiDisplayable* pParent, size_t width, const string& str, const string& font);

    static size_t reqHeight();

    void doDisplay() override;

    void setTextOffset(Gui::Coord offset);

private:
    string text_;
    Gui::Coord textOffset_;
    GuiBmpFont font_;

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
};

#endif

/* End TEXT.HPP *****************************************************/
