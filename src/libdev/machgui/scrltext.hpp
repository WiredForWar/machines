/*
 * S C R L T E X T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiScrollableText

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_SCRLTEXT_HPP
#define _MACHGUI_SCRLTEXT_HPP

#include "base/base.hpp"
#include "gui/scrolist.hpp"
#include "gui/root.hpp"

class MachGuiScrollableText : public GuiSimpleScrollableList
// Canonical form revoked
{
public:
    MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box);
    MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box, uint stringId);
    MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box, const string& text);
    MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box, uint columnWidth, const string& text);
    ~MachGuiScrollableText() override;

    void setText(uint stringId);
    void setText(const string&);

    void doDisplay() override;

    void CLASS_INVARIANT;

private:
    friend ostream& operator<<(ostream& o, const MachGuiScrollableText& t);

    MachGuiScrollableText(const MachGuiScrollableText&);
    MachGuiScrollableText& operator=(const MachGuiScrollableText&);

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
};

#endif

/* End SCRLTEXT.HPP *************************************************/
