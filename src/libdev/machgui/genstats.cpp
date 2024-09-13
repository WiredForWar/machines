/*
 * G E N S T A T S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "machgui/genstats.hpp"
#include "gui/displaya.hpp"
#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/ui/MenuStyle.hpp"

constexpr int GS_BOX_WIDTH = 77;

MachGuiGeneralStatistics::MachGuiGeneralStatistics(
    GuiDisplayable* pParent,
    Gui::Coord topLeft,
    uint titleString,
    MachPhys::Race race)
    : GuiDisplayable(pParent, Gui::Boundary(topLeft, Gui::Size(GS_BOX_WIDTH, 77) * MachGui::menuScaleFactor()))
    , statDisplay_(pParent, topLeft + Gui::Coord(1, 35) * MachGui::menuScaleFactor(), race)
{
    // Create title text
    new MachGuiMenuText(
        pParent,
        Gui::Box(topLeft, Gui::Size(GS_BOX_WIDTH, 35) * MachGui::menuScaleFactor()),
        titleString,
        MachGui::Menu::smallFontLight(),
        Gui::AlignCenter);

    TEST_INVARIANT;
}

MachGuiGeneralStatistics::MachGuiGeneralStatistics(
    GuiDisplayable* pParent,
    Gui::Coord topLeft,
    uint titleString,
    MachPhys::Race race,
    const std::string& player)
    : GuiDisplayable(pParent, Gui::Boundary(topLeft, Gui::Size(GS_BOX_WIDTH, 77) * MachGui::menuScaleFactor()))
    , statDisplay_(pParent, topLeft + Gui::Coord(1, 35) * MachGui::menuScaleFactor(), race)
{
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::smallFontWhite()));

    // Create title text
    new MachGuiMenuText(
        pParent,
        Gui::Box(
            topLeft,
            GS_BOX_WIDTH * MachGui::menuScaleFactor(),
            33 * MachGui::menuScaleFactor() - font.charHeight() - 2 * MachGui::menuScaleFactor()),
        titleString,
        MachGui::Menu::smallFontLight(),
        Gui::AlignCenter);

    std::string newPlayer = truncate(player);
    // Create player name text
    new MachGuiMenuText(
        pParent,
        Gui::Box(
            Gui::Coord(
                topLeft.x(),
                topLeft.y() + 33 * MachGui::menuScaleFactor() - font.charHeight() - 2 * MachGui::menuScaleFactor()),
            GS_BOX_WIDTH * MachGui::menuScaleFactor(),
            font.charHeight() + 2 * MachGui::menuScaleFactor()),
        newPlayer,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignCenter);

    TEST_INVARIANT;
}

MachGuiGeneralStatistics::~MachGuiGeneralStatistics()
{
    TEST_INVARIANT;
}

// virtual
void MachGuiGeneralStatistics::update()
{
    if (statDisplay_.redraw())
    {
        statDisplay_.update();
    }
}

// virtual
void MachGuiGeneralStatistics::setStatistics(int stat1, int stat2, int stat3, int stat4)
{
    statDisplay_.setStatistics(stat1, stat2, stat3, stat4);
}

// virtual
void MachGuiGeneralStatistics::doDisplay()
{
    const Gui::Boundary& boundary = absoluteBoundary();

    // Draw bounding box
    GuiPainter::instance().hollowRectangle(boundary, MachGui::DROPDOWNLIGHTGREEN(), 1 * MachGui::menuScaleFactor());

    // Line across box
    GuiPainter::instance().line(
        Gui::Coord(boundary.minCorner().x(), boundary.minCorner().y() + 34 * MachGui::menuScaleFactor()),
        Gui::Coord(boundary.maxCorner().x(), boundary.minCorner().y() + 34 * MachGui::menuScaleFactor()),
        MachGui::DROPDOWNLIGHTGREEN(),
        1 * MachGui::menuScaleFactor());
}

void MachGuiGeneralStatistics::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// static
std::string MachGuiGeneralStatistics::truncate(const std::string& name)
{
    constexpr int GS_INDENT = 6;

    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::smallFontWhite()));
    std::string truncatedName = name;

    const uint maxWidth = (GS_BOX_WIDTH - (GS_INDENT * 2) - 10) * MachGui::menuScaleFactor();
    const uint maxChars = maxWidth / font.maxCharWidth() + font.spacing();

    if (font.textWidth(name) > maxWidth)
    {
        truncatedName.resize(maxChars);
    }
    POST(truncatedName.length() <= maxChars);

    return truncatedName;
}

std::ostream& operator<<(std::ostream& o, const MachGuiGeneralStatistics& t)
{

    o << "MachGuiGeneralStatistics " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiGeneralStatistics " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End GENSTATS.CPP *************************************************/
