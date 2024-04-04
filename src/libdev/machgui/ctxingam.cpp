/*
 * C T X I N G A M . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "network/netnet.hpp"
#include "machlog/network.hpp"
#include "machgui/ctxingam.hpp"
#include "system/pathname.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/database.hpp"
#include "machgui/startup.hpp"
#include "machgui/msgboxei.hpp"
#include "machgui/internal/strings.hpp"

#include "world4d/soundman.hpp"

MachGuiCtxInGameOptions::MachGuiCtxInGameOptions(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("se", pStartupScreens)
{
    // Setup all the buttons on this screen.
    MachGuiMenuButton* pContinueBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(90, 72, 281, 115),
        IDS_MENUBTN_CONTINUE,
        MachGui::ButtonEvent::CONTINUE);
    MachGuiMenuButton* pBriefingBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(90, 178, 281, 222),
        IDS_MENUBTN_BRIEFING,
        MachGui::ButtonEvent::BRIEFING);
    MachGuiMenuButton* pRestartBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(90, 282, 281, 326),
        IDS_MENUBTN_RESTART,
        MachGui::ButtonEvent::RESTART);
    MachGuiMenuButton* pHotkeysBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(90, 377, 281, 420),
        IDS_MENUBTN_HOTKEYS,
        MachGui::ButtonEvent::HOTKEYS);
    MachGuiMenuButton* pOptionsBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(344, 72, 536, 114),
        IDS_MENUBTN_OPTIONS,
        MachGui::ButtonEvent::OPTIONS);
    MachGuiMenuButton* pSaveBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(344, 178, 536, 221),
        IDS_MENUBTN_SAVEGAME,
        MachGui::ButtonEvent::SAVEGAME);
    MachGuiMenuButton* pLoadBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(344, 282, 536, 325),
        IDS_MENUBTN_LOADGAME,
        MachGui::ButtonEvent::LOADGAME);
    MachGuiMenuButton* pExitBtn;
    if (! NetNetwork::instance().isLobbiedGame())
        pExitBtn = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(344, 377, 536, 419),
            IDS_MENUBTN_EXITGAME,
            MachGui::ButtonEvent::EXIT);
    else
        pExitBtn = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(344, 377, 536, 419),
            IDS_MENUBTN_EXIT_TO_ZONE,
            MachGui::ButtonEvent::EXIT);

    // Setup nav overrides
    pContinueBtn->setRightNavControl(pOptionsBtn);
    pContinueBtn->setLeftNavControl(pOptionsBtn);
    pContinueBtn->setUpNavControl(pHotkeysBtn);
    pContinueBtn->setDownNavControl(pBriefingBtn);

    pBriefingBtn->setRightNavControl(pSaveBtn);
    pBriefingBtn->setLeftNavControl(pSaveBtn);
    pBriefingBtn->setUpNavControl(pContinueBtn);
    pBriefingBtn->setDownNavControl(pRestartBtn);

    pRestartBtn->setRightNavControl(pLoadBtn);
    pRestartBtn->setLeftNavControl(pLoadBtn);
    pRestartBtn->setUpNavControl(pBriefingBtn);
    pRestartBtn->setDownNavControl(pHotkeysBtn);

    pHotkeysBtn->setRightNavControl(pExitBtn);
    pHotkeysBtn->setLeftNavControl(pExitBtn);
    pHotkeysBtn->setUpNavControl(pRestartBtn);
    pHotkeysBtn->setDownNavControl(pContinueBtn);

    pOptionsBtn->setRightNavControl(pContinueBtn);
    pOptionsBtn->setLeftNavControl(pContinueBtn);
    pOptionsBtn->setUpNavControl(pExitBtn);
    pOptionsBtn->setDownNavControl(pSaveBtn);

    pSaveBtn->setRightNavControl(pBriefingBtn);
    pSaveBtn->setLeftNavControl(pBriefingBtn);
    pSaveBtn->setUpNavControl(pOptionsBtn);
    pSaveBtn->setDownNavControl(pLoadBtn);

    pLoadBtn->setRightNavControl(pRestartBtn);
    pLoadBtn->setLeftNavControl(pRestartBtn);
    pLoadBtn->setUpNavControl(pSaveBtn);
    pLoadBtn->setDownNavControl(pExitBtn);

    pExitBtn->setRightNavControl(pHotkeysBtn);
    pExitBtn->setLeftNavControl(pHotkeysBtn);
    pExitBtn->setUpNavControl(pLoadBtn);
    pExitBtn->setDownNavControl(pOptionsBtn);

    // Disable buttons that don't make sense in particular game modes
#ifdef DEMO
    pLoadBtn->disabled(true);
    pSaveBtn->disabled(true);
    pHotkeysBtn->disabled(true);
#endif
    if (pStartupScreens->gameType() == MachGuiStartupScreens::MULTIGAME)
    {
        pBriefingBtn->disabled(true);
        pRestartBtn->disabled(true);
        pSaveBtn->disabled(true);
        pLoadBtn->disabled(true);
    }
    else if (pStartupScreens->gameType() == MachGuiStartupScreens::SKIRMISHGAME)
    {
        pBriefingBtn->disabled(true);
    }

    TEST_INVARIANT;
}

MachGuiCtxInGameOptions::~MachGuiCtxInGameOptions()
{
    TEST_INVARIANT;
}

void MachGuiCtxInGameOptions::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxInGameOptions& t)
{

    o << "MachGuiCtxInGameOptions " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxInGameOptions " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxInGameOptions::update()
{
    animations_.update();
}

// virtual
bool MachGuiCtxInGameOptions::okayToSwitchContext()
{
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::EXIT)
    {
        if (NetNetwork::instance().isLobbiedGame())
        {
            pStartupScreens_->displayMsgBox(
                IDS_MENUMSG_QUIT,
                new MachGuiExitToInternetMessageBoxResponder(
                    pStartupScreens_,
                    MachGuiExitToInternetMessageBoxResponder::UNLOAD_GAME)),
                true;
            return false;
        }

        pStartupScreens_->unloadGame();
        W4dSoundManager::instance().unloadAll();
        // if we got into this screen via a lobby session then we need to terminate correctly at this point.
    }

    return true;
}

// virtual
void MachGuiCtxInGameOptions::buttonEvent(MachGui::ButtonEvent be)
{
    PRE(pStartupScreens_->gameType() != MachGuiStartupScreens::NOGAME);
    PRE(pStartupScreens_->gameType() != MachGuiStartupScreens::MULTIGAME);

    if (be == MachGui::ButtonEvent::RESTART)
    {
        pStartupScreens_->restartGame();
    }
}

/* End CTXINGAM.CPP *************************************************/
