/*
 * S S L I S T B X . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "gui/sslistbx.hpp"
#include "gui/sslistit.hpp"

GuiSingleSelectionListBox::GuiSingleSelectionListBox(
    GuiDisplayable* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc)
    : GuiSimpleScrollableList(pParent, box, horizontalSpacing, verticalSpacing, scrollInc)
    , pCurrentSelection_(nullptr)
{

    TEST_INVARIANT;
}

GuiSingleSelectionListBox::~GuiSingleSelectionListBox()
{
    TEST_INVARIANT;
}

void GuiSingleSelectionListBox::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const GuiSingleSelectionListBox& t)
{

    o << "GuiSingleSelectionListBox " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "GuiSingleSelectionListBox " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void GuiSingleSelectionListBox::notifyListItemSelection(GuiSingleSelectionListBoxItem* pNewSelection)
{
    if (pNewSelection != pCurrentSelection_)
    {
        if (pCurrentSelection_)
        {
            pCurrentSelection_->setSelected(false);
            pCurrentSelection_->unselect();
        }

        pCurrentSelection_ = pNewSelection;

        pCurrentSelection_->setSelected(true);
        pCurrentSelection_->select();
    }
}

void GuiSingleSelectionListBox::deleteAllItems()
{
    deleteAllChildren();
    pCurrentSelection_ = nullptr;
}

/* End SSLISTBX.CPP *************************************************/
