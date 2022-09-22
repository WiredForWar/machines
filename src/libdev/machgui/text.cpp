/*
 * T E X T . C P P 
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/text.hpp"
#include "gui/font.hpp"
#include "machgui/menus_helper.hpp"

MachGuiText::MachGuiText(GuiDisplayable* pParent, size_t width, const string& str)
    : MachGuiText(pParent, width, str, "gui/menu/smallfnt.bmp")
{
}

MachGuiText::MachGuiText(GuiDisplayable* pParent, size_t width, const string& str, const string& font)
    : GuiDisplayable(pParent)
    , text_(str)
    , font_(GuiBmpFont::getFont(font))
{
    setRelativeBoundary(Gui::Box(0, 0, width, font_.charHeight()));
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

    // Draw list box item text
    font_.drawText( text_, Gui::Coord( absoluteBoundary().minCorner().x() + textOffsetX_, absoluteBoundary().minCorner().y() + textOffsetY_ ), width() );
}

//static 
size_t MachGuiText::reqHeight()
{
    return GuiBmpFont::getFont("gui/menu/smallfnt.bmp").charHeight() + 1;
}

/* End TEXT.CPP *****************************************************/
