/*
 * T E X T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "Text.hpp"

#include "gui/Font.hpp"
#include "gui/GuiPainter.hpp"
#include "gui/Root.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuStyle.hpp"

MachGuiText::MachGuiText(GuiDisplayable* pParent, size_t width, const std::string& str)
    : MachGuiText(pParent, width, str, MachGui::Menu::smallFontLight())
{
}

MachGuiText::MachGuiText(GuiDisplayable* pParent, size_t width, const std::string& str, const std::string& font)
    : GuiDisplayable(pParent)
    , text_(str)
    , font_(Gui::getFont(font))
{
    setRelativeBoundary(Gui::Box(0, 0, width, font_.height()));
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
}

void MachGuiText::setTextOffset(Gui::Coord offset)
{
    textOffset_ = offset;
}

// virtual
void MachGuiText::doDisplay()
{
    // Draw list box item text
    {
        Gui::Coord pos = absoluteBoundary().minCorner() + textOffset_;
        GuiPainter::instance().drawText(text_, Ren::Point(pos.x(), pos.y()), font_, width());
    }
}

// static
size_t MachGuiText::reqHeight()
{
    return Gui::getFont(MachGui::Menu::smallFontLight()).height() + 1 * MachGui::menuScaleFactor();
}
