/*
 * P L A Y E R I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/internal/playeri.hpp"
#include "gui/gui.hpp"
#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/ui/MenuStyle.hpp"

constexpr int PNL_BOX_WIDTH = 77;
constexpr int PNL_INDENT = 6;

MachGuiPlayerNameList::MachGuiPlayerNameList(
    GuiDisplayable* pParent,
    const Gui::Coord& topLeft,
    const string& fontName,
    uint title)
    : GuiDisplayable(pParent, Gui::Boundary(topLeft, Gui::Size(PNL_BOX_WIDTH, 77) * MachGui::menuScaleFactor()))
    , isWhite_(false)
{
    // Create title text
    new MachGuiMenuText(
        this,
        Gui::Box(Gui::Coord(0, 0), Gui::Size(PNL_BOX_WIDTH, 35) * MachGui::menuScaleFactor()),
        title,
        fontName,
        Gui::AlignCenter);

    TEST_INVARIANT;
}

MachGuiPlayerNameList::MachGuiPlayerNameList(
    GuiDisplayable* pParent,
    const Gui::Boundary& boundary,
    const string& fontName,
    uint title,
    bool isWhite)
    : GuiDisplayable(pParent, boundary)
    , isWhite_(isWhite)
{
    // Create title text
    new MachGuiMenuText(
        pParent,
        Gui::Box(
            boundary.minCorner(),
            boundary.maxCorner().x() - boundary.minCorner().x(),
            35 * MachGui::menuScaleFactor()),
        title,
        fontName,
        Gui::AlignCenter);

    TEST_INVARIANT;
}

MachGuiPlayerNameList::~MachGuiPlayerNameList()
{
    TEST_INVARIANT;
}

void MachGuiPlayerNameList::names(
    const string& player1,
    const string& player2,
    const string& player3,
    const string& player4)
{

    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::smallFontWhite()));

    uint spacing = font.charHeight() + 2 * MachGui::menuScaleFactor();
    const uint maxWidth = (PNL_BOX_WIDTH - (PNL_INDENT * 2) - 10) * MachGui::menuScaleFactor();

    string newPlayer1 = truncate(player1, maxWidth);
    string newPlayer2 = truncate(player2, maxWidth);
    string newPlayer3 = truncate(player3, maxWidth);
    string newPlayer4 = truncate(player4, maxWidth);

    const int PosX = 38 * MachGui::menuScaleFactor();

    new MachGuiMenuText(
        this,
        Gui::Box(Gui::Coord(PNL_INDENT, PosX), font.textWidth(newPlayer1), spacing),
        newPlayer1,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft);

    new MachGuiMenuText(
        this,
        Gui::Box(Gui::Coord(PNL_INDENT, PosX + spacing), font.textWidth(newPlayer2), spacing),
        newPlayer2,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft);

    new MachGuiMenuText(
        this,
        Gui::Box(Gui::Coord(PNL_INDENT, PosX + (spacing * 2)), font.textWidth(newPlayer3), spacing),
        newPlayer3,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft);

    new MachGuiMenuText(
        this,
        Gui::Box(Gui::Coord(PNL_INDENT, PosX + (spacing * 3)), font.textWidth(newPlayer4), spacing),
        newPlayer4,
        MachGui::Menu::smallFontWhite(),
        Gui::AlignLeft);
}

// virtual
void MachGuiPlayerNameList::doDisplay()
{
    const Gui::Boundary& boundary = absoluteBoundary();

    if (isWhite_)
    {
        // Draw bounding box
        GuiPainter::instance().hollowRectangle(boundary, MachGui::OFFWHITE(), 1 * MachGui::menuScaleFactor());
        // Line across box
        GuiPainter::instance().line(
            Gui::Coord(boundary.minCorner().x(), boundary.minCorner().y() + 34 * MachGui::menuScaleFactor()),
            Gui::Coord(boundary.maxCorner().x(), boundary.minCorner().y() + 34 * MachGui::menuScaleFactor()),
            MachGui::OFFWHITE(),
            1 * MachGui::menuScaleFactor());
    }
    else
    {
        GuiPainter::instance().hollowRectangle(boundary, MachGui::DROPDOWNLIGHTGREEN(), 1 * MachGui::menuScaleFactor());
        // Line across box
        GuiPainter::instance().line(
            Gui::Coord(boundary.minCorner().x(), boundary.minCorner().y() + 34 * MachGui::menuScaleFactor()),
            Gui::Coord(boundary.maxCorner().x(), boundary.minCorner().y() + 34 * MachGui::menuScaleFactor()),
            MachGui::DROPDOWNLIGHTGREEN(),
            1 * MachGui::menuScaleFactor());
    }
}

void MachGuiPlayerNameList::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// static
string MachGuiPlayerNameList::truncate(const string& name, uint maxWidth)
{
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::smallFontWhite()));
    string truncatedName = name;

    const uint maxChars = maxWidth / font.maxCharWidth() + font.spacing();

    if (font.textWidth(name) > maxWidth)
    {
        truncatedName.resize(maxChars);
    }
    POST(truncatedName.length() <= maxChars);

    return truncatedName;
}

std::ostream& operator<<(std::ostream& o, const MachGuiPlayerNameList& t)
{

    o << "MachGuiPlayerNameList " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiPlayerNameList " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End PLAYERS.CPP **************************************************/
