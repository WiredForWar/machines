/*
 * C H C K B O X . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ui/CheckBox.hpp"
#include "system/pathname.hpp"
#include "gui/event.hpp"
#include "gui/painter.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuStyle.hpp"

#define MGCB_BOX_DIMENSIONX 12
#define MGCB_BOX_DIMENSIONY 11

MachGuiCheckBox::MachGuiCheckBox(
    MachGuiStartupScreens* pStartupScreens,
    GuiDisplayable* pParent,
    const Gui::Coord& boxTopLeft,
    const ResolvedUiString& label,
    bool isChecked)
    : GuiDisplayable(pParent, Gui::Box(boxTopLeft, MGCB_BOX_DIMENSIONX, MGCB_BOX_DIMENSIONY))
    , MachGuiFocusCapableControl(pStartupScreens)
    , label_(label)
    , isChecked_(isChecked)
    , font_(GuiBmpFont::getFont(MachGui::Menu::smallFontLight()))
    , checkBmp_(Gui::requestScaledImage("gui/menu/check.bmp", MachGui::menuScaleFactor()))
    , uncheckBmp_(Gui::requestScaledImage("gui/menu/uncheck.bmp", MachGui::menuScaleFactor()))
    , checkFocusBmp_(Gui::requestScaledImage("gui/menu/checkf.bmp", MachGui::menuScaleFactor()))
    , uncheckFocusBmp_(Gui::requestScaledImage("gui/menu/uncheckf.bmp", MachGui::menuScaleFactor()))
    , checkHighlightBmp_(Gui::requestScaledImage("gui/menu/checkh.bmp", MachGui::menuScaleFactor()))
    , uncheckHighlightBmp_(Gui::requestScaledImage("gui/menu/uncheckh.bmp", MachGui::menuScaleFactor()))
{
    checkBmp_.enableColourKeying();
    uncheckBmp_.enableColourKeying();
    checkFocusBmp_.enableColourKeying();
    uncheckFocusBmp_.enableColourKeying();
    checkHighlightBmp_.enableColourKeying();
    uncheckHighlightBmp_.enableColourKeying();

    TEST_INVARIANT;
}

MachGuiCheckBox::~MachGuiCheckBox()
{
    TEST_INVARIANT;
}

void MachGuiCheckBox::setChecked(bool isChecked)
{
    isChecked_ = isChecked;
    if (callback_)
    {
        callback_(this);
    }
}

void MachGuiCheckBox::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void MachGuiCheckBox::doDisplay()
{
    uint x = absoluteCoord().x();
    uint y = absoluteCoord().y();

    Gui::Coord startText(
        x - font_.textWidth(label_) - 8,
        y
            + (font_.charHeight() > height() ? (font_.charHeight() - height()) / 2
                                             : (height() - font_.charHeight()) / 2));
    font_.drawText(label_, startText, font_.textWidth(label_));

    if (isChecked_)
    {
        if (isFocusControl())
        {
            GuiPainter::instance().blit(checkFocusBmp_, absoluteCoord());
        }
        else if (isHighlighted_)
        {
            GuiPainter::instance().blit(checkHighlightBmp_, absoluteCoord());
        }
        else
        {
            GuiPainter::instance().blit(checkBmp_, absoluteCoord());
        }
    }
    else
    {
        if (isFocusControl())
        {
            GuiPainter::instance().blit(uncheckFocusBmp_, absoluteCoord());
        }
        else if (isHighlighted_)
        {
            GuiPainter::instance().blit(uncheckHighlightBmp_, absoluteCoord());
        }
        else
        {
            GuiPainter::instance().blit(uncheckBmp_, absoluteCoord());
        }
    }
}

// virtual
void MachGuiCheckBox::doHandleMouseClickEvent(const GuiMouseEvent& rel)
{
    if (rel.leftButton() == Gui::PRESSED)
    {
        setChecked(!isChecked_);
        changed();
    }
}

std::ostream& operator<<(std::ostream& o, const MachGuiCheckBox& t)
{

    o << "MachGuiCheckBox " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCheckBox " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiCheckBox::executeControl()
{
    setChecked(!isChecked_);
    changed();

    return true;
}

// virtual
void MachGuiCheckBox::hasFocus(bool newValue)
{
    MachGuiFocusCapableControl::hasFocus(newValue);

    changed();
}

// virtual
void MachGuiCheckBox::doHandleMouseEnterEvent(const GuiMouseEvent&)
{
    isHighlighted_ = true;

    changed();
}

// virtual
void MachGuiCheckBox::doHandleMouseExitEvent(const GuiMouseEvent&)
{
    isHighlighted_ = false;

    changed();
}

/* End CHCKBOX.CPP **************************************************/
