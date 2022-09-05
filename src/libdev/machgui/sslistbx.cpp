/*
 * S S L I S T B X . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/sslistbx.hpp"
#include "machgui/sslistit.hpp"
#include "machgui/startup.hpp"
#include "gui/painter.hpp"
#include "machgui/menus_helper.hpp"
#include <algorithm>

MachGuiSingleSelectionListBox::MachGuiSingleSelectionListBox(MachGuiStartupScreens* pStartupScreens,
                                                             GuiDisplayable* pParent, const Gui::Box& box,
                                                             size_t horizontalSpacing, size_t verticalSpacing,
                                                             size_t scrollInc)
:	GuiSingleSelectionListBox(pStartupScreens, box, horizontalSpacing, verticalSpacing, scrollInc ),
    MachGuiFocusCapableControl(pStartupScreens ),
    pStartupScreens_(pStartupScreens )
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    TEST_INVARIANT;
}

MachGuiSingleSelectionListBox::~MachGuiSingleSelectionListBox()
{
    TEST_INVARIANT;

    deleteAllChildren();
}

void MachGuiSingleSelectionListBox::CLASS_INVARIANT
{
    INVARIANT( this != NULL );
}

ostream& operator <<( ostream& o, const MachGuiSingleSelectionListBox& t )
{

    o << "MachGuiSingleSelectionListBox " << reinterpret_cast<void*>( const_cast<MachGuiSingleSelectionListBox*>(&t) ) << " start" << std::endl;
    o << "MachGuiSingleSelectionListBox " << reinterpret_cast<void*>( const_cast<MachGuiSingleSelectionListBox*>(&t) ) << " end" << std::endl;

    return o;
}

//virtual
void MachGuiSingleSelectionListBox::doDisplay()
{
    // Blit background to list box
    auto* shared = pRootParent_->getSharedBitmaps();
    auto backdrop = shared->getNamedBitmap("backdrop");
    shared->blitNamedBitmapFromArea(
            backdrop,
            absoluteBoundary(),
            absoluteBoundary().minCorner(),
            [shared, backdrop](const Gui::Box& box) {
                using namespace machgui::helper::menus;
                return centered_bitmap_transform(
                        box,
                        shared->getWidthOfNamedBitmap(backdrop),
                        shared->getHeightOfNamedBitmap(backdrop)
                );
            });
}

void MachGuiSingleSelectionListBox::addListItem( MachGuiSingleSelectionListBoxItem* pItem )
{
    listItems_.push_back( pItem );
}

void MachGuiSingleSelectionListBox::removeListItem( MachGuiSingleSelectionListBoxItem* pItem )
{
    auto i = find( listItems_.begin(), listItems_.end(), pItem );

    ASSERT( i != listItems_.end(), "Trying to remove pItem which does not exist" );

    listItems_.erase( i );
}

//virtual
void MachGuiSingleSelectionListBox::hasFocus( bool newValue )
{
    MachGuiFocusCapableControl::hasFocus( newValue );

    changed();

    // If no items are selected in list then select first one
    if ( isFocusControl() )
    {
        bool itemSelected = false;

        for ( 	auto i = listItems_.begin();
                i != listItems_.end() and not itemSelected;
                ++i )
        {
            itemSelected = (*i)->selected();
        }

        // Select first item
        if ( !itemSelected and !listItems_.empty() )
        {
            listItems_.front()->selectThisItem();
        }
    }
}

//virtual
bool MachGuiSingleSelectionListBox::doHandleNavigationKey( MachGuiFocusCapableControl::NavKey navKey, MachGuiFocusCapableControl** ppNavFocusControl)
{
    bool retValue = false;

    if ( navKey == MachGuiFocusCapableControl::UP_ARROW )
    {
        MachGuiSingleSelectionListBoxItem* pPreviousItem = nullptr;
        MachGuiSingleSelectionListBoxItem* pCurrentItem = nullptr;
        bool itemSelected = false;

        // Find item before one selected
        for ( 	auto i = listItems_.begin();
                i != listItems_.end() and not itemSelected;
                ++i )
        {
            pPreviousItem = pCurrentItem;

            pCurrentItem = *i;

            itemSelected = pCurrentItem->selected();
        }

        // Didn't find previous item (first item was selected or there isn't any items to select)
        if ( !pPreviousItem and !listItems_.empty() )
        {
            pPreviousItem = listItems_.back();
        }

        // Select the new item
        if ( pPreviousItem )
        {
            doNavSelectNewItem( pPreviousItem );
            retValue = true;
        }
    }
    else if ( navKey == MachGuiFocusCapableControl::DOWN_ARROW )
    {
        bool itemSelected = false;

        // Find item after one selected
        auto i = listItems_.begin();

        for ( 	/*empty*/;
                i != listItems_.end() and not itemSelected;
                ++i )
        {
            itemSelected = (*i)->selected();
        }

        // Found selected item, no select one after it...
        if ( itemSelected )
        {
            if ( i != listItems_.end() )
            {
                doNavSelectNewItem( (*i) );
            }
            else
            {
                doNavSelectNewItem( listItems_.front() );
            }

            retValue = true;
        }
    }

    return retValue;
}

//virtual
bool MachGuiSingleSelectionListBox::isEnabled() const
{
    return MachGuiFocusCapableControl::isEnabled() && !listItems_.empty();
}

ctl_pvector< MachGuiSingleSelectionListBoxItem >& MachGuiSingleSelectionListBox::listItems()
{
    return listItems_;
}

//virtual
void MachGuiSingleSelectionListBox::doNavSelectNewItem( MachGuiSingleSelectionListBoxItem* pItem )
{
    pItem->selectThisItem();
}

/* End SSLISTBX.CPP *************************************************/
