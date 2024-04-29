/*
 * P L A Y S C R I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/internal/playscri.hpp"
#include "machgui/internal/strings.hpp"

#include "machgui/gui.hpp"
#include "machgui/ui/MenuStyle.hpp"

MachGuiPlayerScore::MachGuiPlayerScore(GuiDisplayable* pParent, const Gui::Coord& topLeft)
    : MachGuiPlayerNameList(
        pParent,
        Gui::Boundary(topLeft, MexSize2d(135, 77) * MachGui::menuScaleFactor()),
        MachGui::Menu::largeFontLight(),
        IDS_MENU_STSSCORE,
        true)
    , statDisplay_(pParent, topLeft + Gui::Coord(59, 35) * MachGui::menuScaleFactor(), MachPhys::N_RACES)
{

    TEST_INVARIANT;
}

MachGuiPlayerScore::~MachGuiPlayerScore()
{
    TEST_INVARIANT;
}

// virtual
void MachGuiPlayerScore::update()
{
    if (statDisplay_.redraw())
    {
        statDisplay_.update();
    }
}

// virtual
void MachGuiPlayerScore::setStatistics(int stat1, int stat2, int stat3, int stat4)
{
    statDisplay_.setStatistics(stat1, stat2, stat3, stat4);
}

void MachGuiPlayerScore::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiPlayerScore& t)
{

    o << "MachGuiPlayerScore " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiPlayerScore " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End PLAYSCRI.CPP *************************************************/
