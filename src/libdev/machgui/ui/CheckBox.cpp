/*
 * C H C K B O X . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ui/CheckBox.hpp"

#include "gui/gui.hpp"
#include "system/pathname.hpp"
#include "gui/event.hpp"
#include "gui/painter.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuStyle.hpp"

static constexpr MexSize2d IndicatorSize = MexSize2d(12, 11);

MachGuiCheckBox::MachGuiCheckBox(
    MachGuiStartupScreens* pStartupScreens,
    GuiDisplayable* pParent,
    const Gui::Box& box,
    const ResolvedUiString& label)
    : GuiDisplayable(
        pParent,
        box)
    , MachGuiFocusCapableControl(pStartupScreens)
    , label_(label)
    , font_(GuiBmpFont::getFont(MachGui::Menu::smallFontLight()))
    , checkBmp_(MachGui::getScaledImage("gui/menu/check.bmp", MachGui::menuScaleFactor()))
    , uncheckBmp_(MachGui::getScaledImage("gui/menu/uncheck.bmp", MachGui::menuScaleFactor()))
    , checkFocusBmp_(MachGui::getScaledImage("gui/menu/checkf.bmp", MachGui::menuScaleFactor()))
    , uncheckFocusBmp_(MachGui::getScaledImage("gui/menu/uncheckf.bmp", MachGui::menuScaleFactor()))
    , checkHighlightBmp_(MachGui::getScaledImage("gui/menu/checkh.bmp", MachGui::menuScaleFactor()))
    , uncheckHighlightBmp_(MachGui::getScaledImage("gui/menu/uncheckh.bmp", MachGui::menuScaleFactor()))
{
    checkBmp_.enableColourKeying();
    uncheckBmp_.enableColourKeying();
    checkFocusBmp_.enableColourKeying();
    uncheckFocusBmp_.enableColourKeying();
    checkHighlightBmp_.enableColourKeying();
    uncheckHighlightBmp_.enableColourKeying();

    const auto ScaledSize = IndicatorSize * MachGui::menuScaleFactor();
    Gui::Coord indicatorPosition(absoluteBoundary().bottomRight() - Gui::Vec(ScaledSize.width(), ScaledSize.height()));

    constexpr int spacing{8};
    textPos_ = Gui::Coord(
        width() - (IndicatorSize.width() + spacing) * MachGui::menuScaleFactor() - font_.textWidth(label_),
        font_.charHeight() > height() ? 0 : (height() - font_.charHeight()) / 2 + 1 * Gui::uiScaleFactor());

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
    const auto ScaledSize = IndicatorSize * MachGui::menuScaleFactor();
    Gui::Coord indicatorPosition(absoluteBoundary().bottomRight() - Gui::Vec(ScaledSize.width(), ScaledSize.height()));

    font_.drawText(label_, absoluteBoundary().topLeft() + textPos_, font_.textWidth(label_));

    if (isChecked_)
    {
        if (isFocusControl())
        {
            GuiPainter::instance().blit(checkFocusBmp_, indicatorPosition);
        }
        else if (isHighlighted_)
        {
            GuiPainter::instance().blit(checkHighlightBmp_, indicatorPosition);
        }
        else
        {
            GuiPainter::instance().blit(checkBmp_, indicatorPosition);
        }
    }
    else
    {
        if (isFocusControl())
        {
            GuiPainter::instance().blit(uncheckFocusBmp_, indicatorPosition);
        }
        else if (isHighlighted_)
        {
            GuiPainter::instance().blit(uncheckHighlightBmp_, indicatorPosition);
        }
        else
        {
            GuiPainter::instance().blit(uncheckBmp_, indicatorPosition);
        }
    }
}

// virtual
void MachGuiCheckBox::doHandleMouseClickEvent(const GuiMouseEvent& event)
{
    if (!hitButton(event.coord()))
        return;

    if (event.leftButton() == Gui::PRESSED)
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

int MachGuiCheckBox::implicitHeight()
{
    return IndicatorSize.height() * MachGui::menuScaleFactor();
}

// virtual
void MachGuiCheckBox::doHandleMouseEnterEvent(const GuiMouseEvent& event)
{
    if (!hitButton(event.coord()))
        return;

    isHighlighted_ = true;
    changed();
}

void MachGuiCheckBox::doHandleContainsMouseEvent(const GuiMouseEvent& event)
{
    isHighlighted_ = hitButton(event.coord());
    changed();
}

// virtual
void MachGuiCheckBox::doHandleMouseExitEvent(const GuiMouseEvent& event)
{
    isHighlighted_ = false;

    changed();
}

bool MachGuiCheckBox::hitButton(Gui::Coord relCoord) const
{
    return relCoord.x() >= textPos_.x();
}

/* End CHCKBOX.CPP **************************************************/
