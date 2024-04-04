/*
 * S C R L T E X T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiScrollableText

    A brief description of the class should go in here
*/

#pragma once

#include "gui/scrolist.hpp"
#include "machgui/ResolvedUiString.hpp"

class GuiRoot;

class MachGuiScrollableText : public GuiSimpleScrollableList
// Canonical form revoked
{
public:
    MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box, uint columnWidth = 1000);
    ~MachGuiScrollableText() override;

    void setText(const ResolvedUiString& text);

    void doDisplay() override;

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiScrollableText& t);

    MachGuiScrollableText(const MachGuiScrollableText&);
    MachGuiScrollableText& operator=(const MachGuiScrollableText&);

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
};
