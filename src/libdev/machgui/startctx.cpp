/*
 * S T A R T C T X . C P P 
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/startctx.hpp"

MachGuiStartupScreenContext::MachGuiStartupScreenContext( MachGuiStartupScreens* pStartupScreens ) 
:	pStartupScreens_( pStartupScreens )
{

    TEST_INVARIANT;
}

MachGuiStartupScreenContext::~MachGuiStartupScreenContext()
{
    TEST_INVARIANT;

}

//virtual
void MachGuiStartupScreenContext::update()
{
    // Intentionally empty
}

// virtual
bool MachGuiStartupScreenContext::okayToSwitchContext()
{
    return true;
}

//virtual 
void MachGuiStartupScreenContext::buttonEvent( MachGuiStartupScreens::ButtonEvent )
{
    // Intentionally empty
}

//virtual 
bool MachGuiStartupScreenContext::doHandleKeyEvent( const GuiKeyEvent& )
{
    // Intentionally empty
    return false;
}

void MachGuiStartupScreenContext::CLASS_INVARIANT
{
    INVARIANT( this != NULL );
}

ostream& operator <<( ostream& o, const MachGuiStartupScreenContext& t )
{

    o << "MachGuiStartupScreenContext " << reinterpret_cast<void*>( const_cast<MachGuiStartupScreenContext*>(&t) ) << " start" << std::endl;
    o << "MachGuiStartupScreenContext " << reinterpret_cast<void*>( const_cast<MachGuiStartupScreenContext*>(&t) ) << " end" << std::endl;

    return o;
}

void MachGuiStartupScreenContext::changeBackdrop(const char* newBackdrop)
{
    // This is here to show that the background image, or backdrop, is managed by the active GuiRoot.
    GuiRoot* root = pStartupScreens_;

    root->getSharedBitmaps()->createUpdateNamedBitmap("backdrop", newBackdrop);
    root->changed();
}

/* End STARTCTX.CPP *************************************************/
