/*
 * D R O P D O W N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/DropDownListBox.hpp"
#include "machgui/gui.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "gui/GuiPainter.hpp"
#include "gui/Font.hpp"
#include "gui/Manager.hpp"
#include "device/Time.hpp"

MachGuiDropDownListBoxItem::MachGuiDropDownListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiDropDownListBox* pListBox,
    size_t width,
    const std::string& text)
    : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, text)
    , pListBox_(pListBox)
    , whiteFont_(false)
{
    changed();
}

MachGuiDropDownListBoxItem::MachGuiDropDownListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiDropDownListBox* pListBox,
    size_t width,
    const std::string& text,
    bool whiteFont)
    : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, text)
    , pListBox_(pListBox)
    , whiteFont_(whiteFont)
{
    changed();
}

// static
GuiBmpFont MachGuiDropDownListBoxItem::getWhiteFont()
{
    GuiBmpFont bmpFont = Gui::getFont(SysPathName(MachGui::Menu::smallFontWhite()));

    return bmpFont;
}

// virtual
void MachGuiDropDownListBoxItem::doDisplay()
{
    // An item stands one scaled pixel short of its own height, since it shares that
    // pixel with the item below it, and the highlight is drawn to that shorter figure.
    // The capitals sit in the middle of it, which is not a whole number of scaled
    // pixels down: it is one and a half of them, so it has to be worked out rather
    // than written down.
    const int itemHeight = static_cast<int>(height()) - 1 * static_cast<int>(MachGui::menuScaleFactor());

    const Ren::Point textCoord(
        absoluteBoundary().minCorner().x() + 2 * MachGui::menuScaleFactor(),
        absoluteBoundary().minCorner().y() + Gui::textTopIn(itemHeight, whiteFont_ ? getWhiteFont() : getFont()));

    if (isSelected())
    {
        GuiPainter::instance().blit(
            MachGui::longGlowBmp(),
            Gui::Box(0, 0, width(), height() - 1 * MachGui::menuScaleFactor()),
            absoluteBoundary().minCorner());
        GuiPainter::instance().drawText(text(), textCoord, getUnderlineFont(), width());
    }
    else if (isHighlighted())
    {
        GuiPainter::instance().blit(
            MachGui::longGlowBmp(),
            Gui::Box(0, 0, width(), height() - 1 * MachGui::menuScaleFactor()),
            absoluteBoundary().minCorner());
        GuiPainter::instance().drawText(text(), textCoord, getHighlightFont(), width());
    }
    else
    {
        // Draw background to list box item
        GuiPainter::instance().filledRectangle(absoluteBoundary(), MachGui::DROPDOWNDARKGREEN());
        GuiPainter::instance().hollowRectangle(
            absoluteBoundary(),
            MachGui::DROPDOWNLIGHTGREEN(),
            1 * MachGui::menuScaleFactor());

        // Draw list box item text
        GuiPainter::instance().drawText(text(), textCoord, whiteFont_ ? getWhiteFont() : getFont(), width());
    }
}

// virtual
void MachGuiDropDownListBoxItem::select()
{
    pListBox_->itemSelected(text());
}

void MachGuiDropDownListBoxItem::doHandleMouseClickEvent(const GuiMouseEvent& rel)
{
    PRE(pListBox_);

    if (rel.leftButton() == Gui::RELEASED)
    {
        selectThisItem();
    }
}

MachGuiDropDownListBox::MachGuiDropDownListBox(
    MachGuiStartupScreens* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc,
    size_t itemWidth,
    const ctl_vector<std::string>& itemText)
    : GuiSingleSelectionListBox(pParent, box, horizontalSpacing, verticalSpacing, scrollInc)
    , MachGuiAutoDeleteDisplayable(pParent)
    , itemText_(itemText)
    , itemWidth_(itemWidth)
    , whiteFont_(false)
{
    setLayer(GuiDisplayable::LAYER2);

    timeInterval_ = 0.10 / itemText_.size();

    nextItem_ = 0;

    timeStart_ = DevTime::instance().time();
}

MachGuiDropDownListBox::MachGuiDropDownListBox(
    MachGuiStartupScreens* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc,
    size_t itemWidth,
    const ctl_vector<std::string>& itemText,
    bool whiteFont)
    : GuiSingleSelectionListBox(pParent, box, horizontalSpacing, verticalSpacing, scrollInc)
    , MachGuiAutoDeleteDisplayable(pParent)
    , itemText_(itemText)
    , itemWidth_(itemWidth)
    , whiteFont_(whiteFont)
{
    setLayer(GuiDisplayable::LAYER2);

    timeInterval_ = 0.10 / itemText_.size();

    nextItem_ = 0;

    timeStart_ = DevTime::instance().time();
}

MachGuiDropDownListBox::~MachGuiDropDownListBox()
{
}

// virtual
void MachGuiDropDownListBox::itemSelected(const std::string& /*itemText*/)
{
    // You are forced to derive from this class and supply an implementation for this function.
    // This base class function should be called from the derived class!

    // Out of sight at once, but destroyed by the startup screens once the call
    // that selected the item has returned: the item doing the selecting is a
    // child of this box and is still on the stack.
    setVisible(false);
    startupScreens().closeAutoDeleteGuiElement();
}

// virtual
void MachGuiDropDownListBox::doDisplay()
{
}

// virtual
bool MachGuiDropDownListBox::containsMousePointer()
{
    Gui::Coord mousePos(DevMouse::instance().position().first, DevMouse::instance().position().second);

    return contains(mousePos);
}

// virtual
void MachGuiDropDownListBox::update()
{
    if (DevTime::instance().time() - timeStart_ > timeInterval_)
    {
        if (nextItem_ < itemText_.size())
        {
            // new MachGuiDropDownListBoxItem( &startupScreens(), this, itemWidth_, itemText_[nextItem_], whiteFont_ );
            createListBoxItem(&startupScreens(), this, itemWidth_, itemText_[nextItem_], whiteFont_);

            childAdded();
            ++nextItem_;
        }

        timeStart_ = DevTime::instance().time();
    }
}

// virtual
MachGuiDropDownListBoxItem* MachGuiDropDownListBox::createListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiDropDownListBox* pListBox,
    size_t width,
    const std::string& text,
    bool whiteFont)
{
    return new MachGuiDropDownListBoxItem(pStartupScreens, pListBox, width, text, whiteFont);
}

void MachGuiDropDownListBox::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDropDownListBox& t)
{

    o << "MachGuiDropDownListBox " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDropDownListBox " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End DROPDOWN.CPP *************************************************/
