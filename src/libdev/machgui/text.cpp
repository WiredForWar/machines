/*
 * T E X T . C P P 
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/text.hpp"
#include "machgui/startup.hpp"
#include "gui/painter.hpp"
#include "gui/font.hpp"

MachGuiText::MachGuiText(GuiDisplayable* pParent, size_t width, const string& str)
:	GuiDisplayable( pParent, Gui::Box(0,0,width,GuiBmpFont::getFont("gui/menu/smallfnt.bmp").charHeight() ) ),
    text_( str ),
    textOffsetX_( 0 ),
    textOffsetY_( 0 ),
    font_( GuiBmpFont::getFont("gui/menu/smallfnt.bmp") )
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
}

MachGuiText::MachGuiText(GuiDisplayable* pParent, size_t width, const string& str, const string& font)
:	GuiDisplayable( pParent, Gui::Box(0,0,width,GuiBmpFont::getFont(font).charHeight() ) ),
    text_( str ),
    textOffsetX_( 0 ),
    textOffsetY_( 0 ),
    font_( GuiBmpFont::getFont(font) )
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
}

void MachGuiText::textOffset( int x, int y )
{
    textOffsetX_ = x;
    textOffsetY_ = y;
}

//virtual 
void MachGuiText::doDisplay()
{
    // Blit background to list box item
    auto backdrop = pRootParent_->getSharedBitmaps()->getNamedBitmap("backdrop");
    pRootParent_->getSharedBitmaps()->blitNamedBitmapFromArea(
            backdrop,
            absoluteBoundary(),
            absoluteBoundary().minCorner(),
            [](Gui::Box box) {
                return Gui::Box(Gui::Coord(box.minCorner().x() - MachGuiStartupScreens::xMenuOffset(),
                                           box.minCorner().y() - MachGuiStartupScreens::yMenuOffset()),
                                box.maxCorner().x() - box.minCorner().x(),
                                box.maxCorner().y() - box.minCorner().y()
                );
            });

    // Draw list box item text
    font_.drawText( text_, Gui::Coord( absoluteBoundary().minCorner().x() + textOffsetX_, absoluteBoundary().minCorner().y() + textOffsetY_ ), width() );
}

//static 
size_t MachGuiText::reqHeight()
{
    return GuiBmpFont::getFont("gui/menu/smallfnt.bmp").charHeight() + 1;
}

/* End TEXT.CPP *****************************************************/
