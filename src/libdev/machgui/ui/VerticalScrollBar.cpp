/*
 * S C R O L B A R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "VerticalScrollBar.hpp"

#include "machgui/gui.hpp"
#include "gui/GuiPainter.hpp"
#include "gui/Border.hpp"
#include "gui/ScrollButton.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////
MachGuiVerticalScrollBar::MachGuiVerticalScrollBar(
    GuiDisplayable* pParent,
    const Gui::Box& area,
    GuiSimpleScrollableList* pList)
    : GuiVerticalScrollBar(pParent, area, pList)
{
    changed();
}

std::size_t MachGuiVerticalScrollBar::minBoxHeight() const
{
    return 9 * MachGui::menuScaleFactor();
}

std::size_t MachGuiVerticalScrollBar::barFrameWidth() const
{
    return 1 * MachGui::menuScaleFactor();
}

void MachGuiVerticalScrollBar::doDisplayBar()
{
    const Gui::Box& bar = absoluteBoundary();
    const int frameWidth = barFrameWidth();

    GuiPainter::instance().filledRectangle(bar, MachGui::MENUDARKGREEN());

    // A frame down either side, drawn as the bands they are. A line of a given width
    // is put where it rasterises, which past one pixel across is not where it was
    // asked for.
    auto band = [](int x, int y, int bandWidth, int bandHeight, const Gui::Colour& colour) {
        if (bandWidth > 0 && bandHeight > 0)
            GuiPainter::instance().filledRectangle(
                Gui::Box(Gui::Coord(x, y), bandWidth, bandHeight),
                colour);
    };

    band(bar.minCorner().x(), bar.minCorner().y(), frameWidth, bar.height(), MachGui::DARKSANDY());
    band(bar.maxCorner().x() - frameWidth, bar.minCorner().y(), frameWidth, bar.height(), MachGui::DARKSANDY());
}

void MachGuiVerticalScrollBar::doDisplayBox(const Gui::Box& absoluteBox)
{
    const int penWidth = 1 * MachGui::menuScaleFactor();
    const int left = absoluteBox.minCorner().x();
    const int top = absoluteBox.minCorner().y();
    const int boxWidth = absoluteBox.width();
    const int boxHeight = absoluteBox.height();

    auto band = [](int x, int y, int bandWidth, int bandHeight) {
        if (bandWidth > 0 && bandHeight > 0)
            GuiPainter::instance().filledRectangle(
                Gui::Box(Gui::Coord(x, y), bandWidth, bandHeight),
                MachGui::SANDY());
    };

    // Outlined a pen in from either side and against its own top and bottom, which is
    // where it came out when a pen was a single pixel.
    band(left + penWidth, top, penWidth, boxHeight);
    band(left + boxWidth - (2 * penWidth), top, penWidth, boxHeight);
    band(left + penWidth, top, boxWidth - (2 * penWidth), penWidth);
    band(left + penWidth, top + boxHeight - penWidth, boxWidth - (2 * penWidth), penWidth);

    // Three lines across the middle of it, a pen apart from one another.
    const int middle = top + (boxHeight / 2) - (penWidth / 2);

    for (const int offset : { -2 * penWidth, 0, 2 * penWidth })
    {
        band(left + (3 * penWidth), middle + offset, boxWidth - (6 * penWidth), penWidth);
    }
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
