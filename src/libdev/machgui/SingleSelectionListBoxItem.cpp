/*
 * S S L I S T I T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "gui/sslistit.hpp"
#include "machgui/SingleSelectionListBoxItem.hpp"
#include "machgui/SingleSelectionListBox.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/gui.hpp"
#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "machgui/internal/SoundManager.hpp"
#include "machgui/ui/MenuStyle.hpp"

MachGuiSingleSelectionListBoxItem::MachGuiSingleSelectionListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiSingleSelectionListBox* pParentListBox,
    size_t width,
    const std::string& text)
    : GuiSingleSelectionListBoxItem(pParentListBox, width, reqHeight())
    , text_(text)
    , highlighted_(false)
    , pStartupScreens_(pStartupScreens)
    , pMyListBox_(pParentListBox)
{
    pMyListBox_->addListItem(this);

    pRootParent_ = static_cast<GuiRoot*>(pParentListBox->findRoot(this));

    TEST_INVARIANT;
}

// This variant utilized by MachGuiDropDownListBoxItem to initialize it. Hence, pMyListBox_ is null.
MachGuiSingleSelectionListBoxItem::MachGuiSingleSelectionListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    GuiSingleSelectionListBox* pParentListBox,
    size_t width,
    const std::string& text)
    : GuiSingleSelectionListBoxItem(pParentListBox, width, reqHeight())
    , text_(text)
    , highlighted_(false)
    , pStartupScreens_(pStartupScreens)
    , pMyListBox_(nullptr)
{
    pRootParent_ = static_cast<GuiRoot*>(pParentListBox->findRoot(this));

    TEST_INVARIANT;
}

MachGuiSingleSelectionListBoxItem::~MachGuiSingleSelectionListBoxItem()
{
    TEST_INVARIANT;

    if (pMyListBox_)
        pMyListBox_->removeListItem(this);
}

void MachGuiSingleSelectionListBoxItem::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiSingleSelectionListBoxItem& t)
{

    o << "MachGuiSingleSelectionListBoxItem "
      << reinterpret_cast<void*>(const_cast<MachGuiSingleSelectionListBoxItem*>(&t)) << " start" << std::endl;
    o << "MachGuiSingleSelectionListBoxItem "
      << reinterpret_cast<void*>(const_cast<MachGuiSingleSelectionListBoxItem*>(&t)) << " end" << std::endl;

    return o;
}

// static
size_t MachGuiSingleSelectionListBoxItem::reqHeight()
{
    size_t myReqHeight = getFont().height();

    myReqHeight += 5 * MachGui::menuScaleFactor();

    return myReqHeight;
}

// static
GuiBmpFont MachGuiSingleSelectionListBoxItem::getFont()
{
    GuiBmpFont bmpFont = Gui::getFont(MachGui::Menu::smallFontLight());

    return bmpFont;
}

// static
GuiBmpFont MachGuiSingleSelectionListBoxItem::getHighlightFont()
{
    GuiBmpFont bmpFont = Gui::getFont(MachGui::Menu::smallFontDark());

    return bmpFont;
}

// static
GuiBmpFont MachGuiSingleSelectionListBoxItem::getUnderlineFont()
{
    GuiBmpFont bmpFont = Gui::getFont(MachGui::Menu::smallFontDark());

    bmpFont.underline(true);

    return bmpFont;
}

// virtual
void MachGuiSingleSelectionListBoxItem::select()
{
    // Play select sound
    MachGuiSoundManager::instance().playSound("gui/sounds/listclik.wav");

    changed();
}

// virtual
void MachGuiSingleSelectionListBoxItem::unselect()
{
    changed();
}

// virtual
void MachGuiSingleSelectionListBoxItem::doDisplay()
{
    const Gui::Coord textPos(
        absoluteBoundary().minCorner().x() + 1 * Gui::uiScaleFactor(),
        absoluteBoundary().minCorner().y() + 1 * Gui::uiScaleFactor());

    GuiBmpFont textFont = isSelected() ? getUnderlineFont() : (isHighlighted() ? getHighlightFont() : getFont());

    if (isSelected() || isHighlighted())
    {
        const Gui::Box itemBox(0, 0, width(), height() - 1 * Gui::uiScaleFactor());

        if (pMyListBox_ && pMyListBox_->isFocusControl())
        {
            GuiPainter::instance().blit(MachGui::longYellowGlowBmp(), itemBox, absoluteBoundary().minCorner());
        }
        else
        {
            GuiPainter::instance().blit(MachGui::longGlowBmp(), itemBox, absoluteBoundary().minCorner());
        }
    }

    GuiPainter::instance().drawText(text_, Ren::Point(textPos.x(), textPos.y()), textFont, static_cast<int>(width()));
}

bool MachGuiSingleSelectionListBoxItem::isHighlighted() const
{
    return highlighted_;
}

// virtual
void MachGuiSingleSelectionListBoxItem::doHandleMouseEnterEvent(const GuiMouseEvent& /*rel*/)
{
    // Play enter sound
    MachGuiSoundManager::instance().playSound("gui/sounds/listhigh.wav");

    highlighted_ = true;

    if (!isSelected())
        changed();
}

// virtual
void MachGuiSingleSelectionListBoxItem::doHandleMouseExitEvent(const GuiMouseEvent& /*rel*/)
{
    highlighted_ = false;

    if (!isSelected())
        changed();
}

const std::string& MachGuiSingleSelectionListBoxItem::text() const
{
    return text_;
}

MachGuiStartupScreens* MachGuiSingleSelectionListBoxItem::startupScreens()
{
    return pStartupScreens_;
}

MachGuiSingleSelectionListBox* MachGuiSingleSelectionListBoxItem::myListBox()
{
    return pMyListBox_;
}

/* End SSLISTIT.CPP *************************************************/
