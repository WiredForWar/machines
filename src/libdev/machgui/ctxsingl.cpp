/*
 * C T X S I N G L . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxsingl.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/internal/strings.hpp"

MachGuiCtxSinglePlayer::MachGuiCtxSinglePlayer(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("si", pStartupScreens)
{
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(401, 62, 588, 105),
        IDS_MENUBTN_CAMPAIGN,
        MachGui::ButtonEvent::CAMPAIGN);
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(401, 169, 588, 211),
        IDS_MENUBTN_SKIRMISH,
        MachGui::ButtonEvent::SKIRMISH);
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(401, 272, 588, 314),
        IDS_MENUBTN_LOADGAME,
        MachGui::ButtonEvent::LOADGAME);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(401, 370, 588, 412),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);
    new MachGuiMenuText(pStartupScreens, Gui::Box(82, 292, 262, 399), IDS_MENU_SINGLEPLAYER, MachGui::Menu::largeFontLight());
    pCancelBtn->escapeControl(true);

    TEST_INVARIANT;
}

MachGuiCtxSinglePlayer::~MachGuiCtxSinglePlayer()
{
    TEST_INVARIANT;
}

void MachGuiCtxSinglePlayer::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxSinglePlayer& t)
{

    o << "MachGuiCtxSinglePlayer " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxSinglePlayer " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxSinglePlayer::update()
{
    animations_.update();
}

/* End CTXSINGL.CPP *************************************************/
