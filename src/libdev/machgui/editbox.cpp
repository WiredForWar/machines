/*
 * E D I T B O X . C P P 
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/editbox.hpp"

#include "base/diag.hpp"
#include "machgui/startup.hpp"
#include "gui/painter.hpp"
#include "device/butevent.hpp"
#include "machgui/menus_helper.hpp"

unsigned char pPromptText1[] = { 87, 244, 105, 128, 48, 182, 149, 151, 72, 203, 222, 59, 150, 19, 218, 139, 154, 243, 40, 190, 242, 54, 243, 50, 98, 159, 84, 250, 37, 136, 151, 150, 0 };
unsigned char pPromptText8[] = { 4, 212, 167, 27, 173, 11, 155, 126, 58, 37, 114, 151, 128 };

MachGuiSingleLineEditBox::MachGuiSingleLineEditBox(GuiDisplayable* pParent, const Gui::Box& box, const GuiBmpFont& font )
:	GuiSingleLineEditBox(pParent, box, font ),
    clearTextOnNextChar_( false ),
    ignoreSpaceAtBeginning_( true )
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    TEST_INVARIANT;
}

MachGuiSingleLineEditBox::~MachGuiSingleLineEditBox()
{

    TEST_INVARIANT;
}

//virtual 
void MachGuiSingleLineEditBox::drawBackground()
{
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

void MachGuiSingleLineEditBox::CLASS_INVARIANT
{
    INVARIANT( this != NULL );
}

ostream& operator <<( ostream& o, const MachGuiSingleLineEditBox& t )
{

    o << "MachGuiSingleLineEditBox " << reinterpret_cast<void*>( const_cast<MachGuiSingleLineEditBox*>(&t) ) << " start" << std::endl;
    o << "MachGuiSingleLineEditBox " << reinterpret_cast<void*>( const_cast<MachGuiSingleLineEditBox*>(&t) ) << " end" << std::endl;

    return o;
}

//virtual 
bool MachGuiSingleLineEditBox::doHandleCharEvent( const GuiCharEvent& e )
{
    DEBUG_STREAM( DIAG_NEIL, "MachGuiSingleLineEditBox::doHandleCharEvent " << e.getChar() << " " << static_cast<int>(e.getChar()) << std::endl );

    if ( ignoreSpaceAtBeginning_ and leftText().empty() and e.getChar() == ' ' )
    {
        return true;
    }

    if ( clearTextOnNextChar_ )
    {
        text( "" );
        clearTextOnNextChar_ = false;
    }

    return GuiSingleLineEditBox::doHandleCharEvent( e );
}

void MachGuiSingleLineEditBox::clearTextOnNextChar( bool newVal )
{
    clearTextOnNextChar_ = newVal;
}

bool MachGuiSingleLineEditBox::clearTextOnNextChar() const
{
    return clearTextOnNextChar_;
}

void MachGuiSingleLineEditBox::ignoreSpaceAtBeginning( bool ignore )
{
    ignoreSpaceAtBeginning_ = ignore;
}


/* End EDITBOX.CPP **************************************************/
