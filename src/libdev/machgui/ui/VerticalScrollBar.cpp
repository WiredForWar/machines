/*
 * S C R O L B A R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "VerticalScrollBar.hpp"

#include "machgui/gui.hpp"
#include "gui/painter.hpp"
#include "gui/border.hpp"
#include "gui/scrolbut.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////
MachGuiVerticalScrollBar::MachGuiVerticalScrollBar(
    GuiDisplayable* pParent,
    const Gui::Box& area,
    GuiSimpleScrollableList* pList)
    : GuiVerticalScrollBar(pParent, area, pList)
{
    changed();
}

// virtual
void MachGuiVerticalScrollBar::doDisplayBar()
{
    GuiPainter::instance().filledRectangle(absoluteBoundary(), MachGui::MENUDARKGREEN());
    const int frameWidth = MachGui::menuScaleFactor() * 1;
    GuiPainter::instance().line(
        absoluteBoundary().minCorner(),
        Gui::Coord(absoluteBoundary().minCorner().x(), absoluteBoundary().maxCorner().y()),
        MachGui::DARKSANDY(),
        frameWidth);
    GuiPainter::instance().line(
        Gui::Coord(absoluteBoundary().maxCorner().x() - frameWidth, absoluteBoundary().minCorner().y()),
        Gui::Coord(absoluteBoundary().maxCorner().x() - frameWidth, absoluteBoundary().maxCorner().y()),
        MachGui::DARKSANDY(),
        frameWidth);
}

// virtual
void MachGuiVerticalScrollBar::doDisplayBox(const Gui::Box& absoluteBox)
{
    // Draw low lights and high lights
    const int penWidth = 1 * MachGui::menuScaleFactor();
    GuiPainter::instance().line(
        Gui::Coord(absoluteBox.maxCorner().x() - 2 * MachGui::menuScaleFactor(), absoluteBox.minCorner().y()),
        Gui::Coord(absoluteBox.maxCorner().x() - 2 * MachGui::menuScaleFactor(), absoluteBox.maxCorner().y()),
        MachGui::SANDY(),
        penWidth);
    GuiPainter::instance().line(
        Gui::Coord(absoluteBox.minCorner().x() + 1 * MachGui::menuScaleFactor(), absoluteBox.maxCorner().y() - 1 * MachGui::menuScaleFactor()),
        Gui::Coord(absoluteBox.maxCorner().x() - 1 * MachGui::menuScaleFactor(), absoluteBox.maxCorner().y() - 1 * MachGui::menuScaleFactor()),
        MachGui::SANDY(),
        penWidth);
    GuiPainter::instance().line(
        Gui::Coord(absoluteBox.minCorner().x() + 1 * MachGui::menuScaleFactor(), absoluteBox.minCorner().y()),
        Gui::Coord(absoluteBox.maxCorner().x() - 1 * MachGui::menuScaleFactor(), absoluteBox.minCorner().y()),
        MachGui::SANDY(),
        penWidth);
    GuiPainter::instance().line(
        Gui::Coord(absoluteBox.minCorner().x() + 1 * MachGui::menuScaleFactor(), absoluteBox.minCorner().y()),
        Gui::Coord(absoluteBox.minCorner().x() + 1 * MachGui::menuScaleFactor(), absoluteBox.maxCorner().y()),
        MachGui::SANDY(),
        penWidth);

    int halfBoxHeight = (absoluteBox.maxCorner().y() - absoluteBox.minCorner().y()) / 2;

    // Draw three lines in middle of scroll box
    GuiPainter::instance().line(
        Gui::Coord(
            absoluteBox.minCorner().x() + 3 * MachGui::menuScaleFactor(),
            absoluteBox.minCorner().y() + halfBoxHeight),
        Gui::Coord(
            absoluteBox.maxCorner().x() - 3 * MachGui::menuScaleFactor(),
            absoluteBox.minCorner().y() + halfBoxHeight),
        MachGui::SANDY(),
        penWidth);
    GuiPainter::instance().line(
        Gui::Coord(
            absoluteBox.minCorner().x() + 3 * MachGui::menuScaleFactor(),
            absoluteBox.minCorner().y() + halfBoxHeight - 2 * MachGui::menuScaleFactor()),
        Gui::Coord(
            absoluteBox.maxCorner().x() - 3 * MachGui::menuScaleFactor(),
            absoluteBox.minCorner().y() + halfBoxHeight - 2 * MachGui::menuScaleFactor()),
        MachGui::SANDY(),
        penWidth);
    GuiPainter::instance().line(
        Gui::Coord(
            absoluteBox.minCorner().x() + 3 * MachGui::menuScaleFactor(),
            absoluteBox.minCorner().y() + halfBoxHeight + 2 * MachGui::menuScaleFactor()),
        Gui::Coord(
            absoluteBox.maxCorner().x() - 3 * MachGui::menuScaleFactor(),
            absoluteBox.minCorner().y() + halfBoxHeight + 2 * MachGui::menuScaleFactor()),
        MachGui::SANDY(),
        penWidth);
}

// static
void MachGuiVerticalScrollBar::createWholeBar(
    GuiDisplayable* pParent,
    const Gui::Coord& topLeft,
    size_t height,
    GuiSimpleScrollableList* pList)
{
    GuiFilledBorderColours gfbc(
        MachGui::SANDY(),
        MachGui::MENUDARKGREEN(),
        MachGui::MENUDARKGREEN(),
        MachGui::MENUDARKGREEN(),
        MachGui::MENUDARKGREEN());

    GuiScrollButton* pButton
        = new GuiScrollButton(pParent, topLeft, pList, GuiScrollButton::BACKWARD, gfbc, MachGui::menuScrollUpBmp());

    const size_t buttonWidth = pButton->width();
    const size_t buttonHeight = pButton->height();

    Gui::Coord but2Pos(topLeft.x(), topLeft.y() + height - buttonHeight);
    Gui::Box scrollBarPos(
        topLeft.x(),
        topLeft.y() + buttonHeight,
        topLeft.x() + buttonWidth,
        topLeft.y() + height - buttonHeight);

    new GuiScrollButton(pParent, but2Pos, pList, GuiScrollButton::FOWARD, gfbc, MachGui::menuScrollDownBmp());
    new MachGuiVerticalScrollBar(pParent, scrollBarPos, pList);
}

/* End SCROLBAR.CPP *************************************************/
