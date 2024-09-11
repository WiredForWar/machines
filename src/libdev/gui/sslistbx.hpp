/*
 * S S L I S T B X . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    GuiSingleSelectionListBox

    A brief description of the class should go in here
*/

#ifndef _GUI_SSLISTBX_HPP
#define _GUI_SSLISTBX_HPP

#include "base/base.hpp"
#include "gui/scrolist.hpp"

class GuiSingleSelectionListBoxItem;

class GuiSingleSelectionListBox : public GuiSimpleScrollableList
// Canonical form revoked
{
public:
    using Callback = std::function<void(GuiSingleSelectionListBox*)>;

    GuiSingleSelectionListBox(
        GuiDisplayable* pParent,
        const Gui::Box&,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc);
    ~GuiSingleSelectionListBox() override;

    void setSelectionChangedCallback(Callback callback);
    const GuiSingleSelectionListBoxItem* currentItem() const { return pCurrentSelection_; }

    void CLASS_INVARIANT;

    void deleteAllItems();

protected:
    friend class GuiSingleSelectionListBoxItem;

    void notifyListItemSelection(GuiSingleSelectionListBoxItem* pNewSelection);

private:
    friend std::ostream& operator<<(std::ostream& o, const GuiSingleSelectionListBox& t);

    GuiSingleSelectionListBox(const GuiSingleSelectionListBox&);
    GuiSingleSelectionListBox& operator=(const GuiSingleSelectionListBox&);

    Callback selectionChangedCallback_;
    GuiSingleSelectionListBoxItem* pCurrentSelection_{};
};

#endif

/* End SSLISTBX.HPP *************************************************/
