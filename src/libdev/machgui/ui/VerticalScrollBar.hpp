/*
 * S C R O L B A R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiVerticalScrollBar

    A brief description of the class should go in here
*/

#pragma once

#include "gui/scrolbar.hpp"

class MachGuiVerticalScrollBar : public GuiVerticalScrollBar
{
public:
    MachGuiVerticalScrollBar(GuiDisplayable* pParent, const Gui::Box& area, GuiSimpleScrollableList* pList);

    // Creates bar with two buttons. Bit of a hack really but...
    static void
    createWholeBar(GuiDisplayable* pParent, const Gui::Coord& topLeft, size_t height, GuiSimpleScrollableList* pList);

protected:
    void doDisplayBar() override;
    void doDisplayBox(const Gui::Box& absoluteBox) override;
};
