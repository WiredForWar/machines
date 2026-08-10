/*
 * C T R L A D O N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiControlPanelAddOn

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTRLADON_HPP
#define _MACHGUI_CTRLADON_HPP

#include "base/base.hpp"
#include "gui/Displayable.hpp"
#include "gui/gui.hpp"

class MachInGameScreen;

class MachGuiControlPanelAddOn : public GuiDisplayable
// Canonical form revoked
{
public:
    MachGuiControlPanelAddOn(GuiDisplayable* pParent, const Gui::Coord& coord, MachInGameScreen* pInGameScreen);
    ~MachGuiControlPanelAddOn() override;

    void doDisplay() override;

    void refresh();

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiControlPanelAddOn& t);

    MachGuiControlPanelAddOn(const MachGuiControlPanelAddOn&);
    MachGuiControlPanelAddOn& operator=(const MachGuiControlPanelAddOn&);

    // Data members...
    GuiBitmap backgroundBmp_;
    MachInGameScreen* pInGameScreen_{};
};

#endif

/* End CTRLADON.HPP *************************************************/
