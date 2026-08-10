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
#include "gui/Root.hpp"
#include "gui/SingleSelectionListBox.hpp"
#include "ctl/PtrVector.hpp"

#include <optional>

class MachGuiStartupScreens;
class MachGuiSingleSelectionListBoxItem;

class MachGuiSingleSelectionListBox
    : public GuiSingleSelectionListBox
// Canonical form revoked
{
public:
    MachGuiSingleSelectionListBox(
        GuiDisplayable* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc);
    ~MachGuiSingleSelectionListBox() override;

    void CLASS_INVARIANT;

    void addListItem(MachGuiSingleSelectionListBoxItem* pItem);
    void removeListItem(MachGuiSingleSelectionListBoxItem* pItem);
    std::optional<std::size_t> getCurrentItemIndex() const;

    bool doHandleNavigationKey(NavKey navKey, GuiDisplayable** ppNavFocusControl) override;
    void hasFocus(bool newValue) override;

    bool isFocusEnabled() const override;

    ctl_pvector<MachGuiSingleSelectionListBoxItem>& listItems();

protected:
    void doDisplay() override;

    virtual void doNavSelectNewItem(MachGuiSingleSelectionListBoxItem*);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiSingleSelectionListBox& t);

    MachGuiSingleSelectionListBox(const MachGuiSingleSelectionListBox&) = delete;
    MachGuiSingleSelectionListBox& operator=(const MachGuiSingleSelectionListBox&) = delete;

    // TODO: Remove this once the focus capable control refactor is done...
    ctl_pvector<MachGuiSingleSelectionListBoxItem> listItems_;

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
};

#endif

/* End SSLISTBX.HPP *************************************************/
