/*
 * D R O P D O W N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/dropdown.hpp"
#include "machgui/gui.hpp"
#include "machgui/startup.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "gui/painter.hpp"
#include "gui/font.hpp"
#include "gui/manager.hpp"
#include "device/time.hpp"

MachGuiDropDownListBoxItem::MachGuiDropDownListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiDropDownListBox* pListBox,
    size_t width,
    const string& text)
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
    const string& text,
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
    GuiBmpFont bmpFont = GuiBmpFont::getFont(SysPathName(MachGui::Menu::smallFontWhite()));

    return bmpFont;
}

// virtual
void MachGuiDropDownListBoxItem::doDisplay()
{
    const Gui::Coord textCoord(
        absoluteBoundary().minCorner().x() + 2 * MachGui::menuScaleFactor(),
        absoluteBoundary().minCorner().y() + 1 * MachGui::menuScaleFactor());

    if (selected())
    {
        GuiPainter::instance().blit(
            MachGui::longGlowBmp(),
            Gui::Box(0, 0, width(), height() - 1 * MachGui::menuScaleFactor()),
            absoluteBoundary().minCorner());
        getUnderlineFont().drawText(text(), textCoord, width());
    }
    else if (highlighted())
    {
        GuiPainter::instance().blit(
            MachGui::longGlowBmp(),
            Gui::Box(0, 0, width(), height() - 1 * MachGui::menuScaleFactor()),
            absoluteBoundary().minCorner());
        getHighlightFont().drawText(text(), textCoord, width());
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
        if (whiteFont_)
        {
            getWhiteFont().drawText(text(), textCoord, width());
        }
        else
        {
            getFont().drawText(text(), textCoord, width());
        }
    }
}

// virtual
void MachGuiDropDownListBoxItem::select()
{
    pListBox_->itemSelected(text());
}

MachGuiDropDownListBox::MachGuiDropDownListBox(
    MachGuiStartupScreens* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc,
    size_t itemWidth,
    const ctl_vector<string>& itemText)
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

    selectItemWhen(GuiSingleSelectionListBox::RELEASED);
}

MachGuiDropDownListBox::MachGuiDropDownListBox(
    MachGuiStartupScreens* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc,
    size_t itemWidth,
    const ctl_vector<string>& itemText,
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

    selectItemWhen(GuiSingleSelectionListBox::RELEASED);
}

MachGuiDropDownListBox::~MachGuiDropDownListBox()
{
}

// virtual
void MachGuiDropDownListBox::itemSelected(const string& /*itemText*/)
{
    // You are forced to derive from this class and supply an implementation for this function.
    // This base class function should be called from the derived class!
    delete this;
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
            //new MachGuiDropDownListBoxItem( &startupScreens(), this, itemWidth_, itemText_[nextItem_], whiteFont_ )
            //);
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
    const string& text,
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
