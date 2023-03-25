/*
 * S S L I S T B X . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiSingleSelectionListBox

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_SSLISTBX_HPP
#define _MACHGUI_SSLISTBX_HPP

#include "base/base.hpp"
#include "gui/root.hpp"
#include "gui/sslistbx.hpp"
#include "machgui/focusctl.hpp"
#include "ctl/pvector.hpp"

class MachGuiStartupScreens;
class MachGuiSingleSelectionListBoxItem;

class MachGuiSingleSelectionListBox
    : public GuiSingleSelectionListBox
    , public MachGuiFocusCapableControl
// Canonical form revoked
{
public:
    // TODO: Eliminate entirely MachGuiStartupScreens from constructor. Focus capable control stuff is what MGSS still
    // needed for
    MachGuiSingleSelectionListBox(
        MachGuiStartupScreens* pStartupScreens,
        GuiDisplayable* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc);
    ~MachGuiSingleSelectionListBox() override;

    void CLASS_INVARIANT;

    void addListItem(MachGuiSingleSelectionListBoxItem* pItem);
    void removeListItem(MachGuiSingleSelectionListBoxItem* pItem);

    bool doHandleNavigationKey(NavKey navKey, MachGuiFocusCapableControl** ppNavFocusControl) override;
    void hasFocus(bool newValue) override;

    bool isFocusEnabled() const override;

    ctl_pvector<MachGuiSingleSelectionListBoxItem>& listItems();

protected:
    void doDisplay() override;

    virtual void doNavSelectNewItem(MachGuiSingleSelectionListBoxItem*);

private:
    friend ostream& operator<<(ostream& o, const MachGuiSingleSelectionListBox& t);

    MachGuiSingleSelectionListBox(const MachGuiSingleSelectionListBox&);
    MachGuiSingleSelectionListBox& operator=(const MachGuiSingleSelectionListBox&);

    // TODO: Remove this once the focus capable control refactor is done...
    DECL_DEPRECATED MachGuiStartupScreens* pStartupScreens_;
    ctl_pvector<MachGuiSingleSelectionListBoxItem> listItems_;

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
};

#endif

/* End SSLISTBX.HPP *************************************************/
