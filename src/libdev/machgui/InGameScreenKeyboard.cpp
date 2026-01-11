#include "base/memwatch.hpp"

#include "machgui/ConsoleDropDown.hpp"
#include "machgui/IInputRegistry.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/ProductionBank.hpp"
#include "machgui/HWResearchBank.hpp"
#include "machgui/Cameras.hpp"
#include "machgui/SquadronBank.hpp"
#include "machgui/ContinentMap.hpp"
#include "machgui/NavigatorBase.hpp"
#include "machgui/WorldViewWindow.hpp"
#include "machgui/commands/command.hpp"
#include "machgui/FirstPerson.hpp"
#include "machgui/PromptText.hpp"
#include "world4d/scenemgr.hpp"
#include "phys/mcmotion.hpp"
#include "phys/motchunk.hpp"
#include "mathex/point3d.hpp"
#include "mathex/eulerang.hpp"
#include "mathex/poly2d.hpp"
#include "profiler/profiler.hpp"
#include "sim/manager.hpp"
#include "gui/event.hpp"
#include "gui/gui.hpp"
#include "machgui/internal/InGameScreenImpl.hpp"
#include "machlog/RecentEventsManager.hpp"
#include "machlog/mcmotseq.hpp"
#include "machlog/vmman.hpp"
#include "machphys/mcmovinf.hpp"
#include "render/surfmgr.hpp"
#include "machlog/network.hpp"

#include <optional>

bool MachInGameScreen::doHandleKeyEvent(const GuiKeyEvent& e)
{
    CB_DEPIMPL_AUTO(allCommands_);
    CB_DEPIMPL_AUTO(pActiveCommand_);
    CB_DEPIMPL_AUTO(pFirstPerson_);
    CB_DEPIMPL_AUTO(gameStateTimer_);
    CB_DEPIMPL_AUTO(pConsoleDropDown_);

    NEIL_STREAM(
        "InGame button event : " << static_cast<int>(e.key()) << " ctrl " << e.isCtrlPressed() << " shift "
                                 << e.isShiftPressed() << " alt " << e.isAltPressed() << std::endl);

    bool processed = false;
    bool finished = false;
    unsigned int count = 0;

    // If the game has been won/lost then only allow escape key to work
    if (actualGameState() != MachInGameScreen::PLAYING)
    {
        processed = true;

        if ((e.key() == Device::KeyCode::ESCAPE || e.key() == Device::KeyCode::SPACE) && e.state() == Gui::PRESSED)
        {
            gameStateTimer_ = 0;
        }
    }

    while (!finished && !processed)
    {
        switch (count)
        {
        case 0:
        {
            static const auto& consoleTrigger = MachGui::inputRegistry()->getBinds("ui-toggle-console"_bind);
            if (e.state() == Gui::PRESSED && consoleTrigger.matches(e.keyWithMods()))
            {
                toggleConsoleDropDown();
            }
            else if (pConsoleDropDown_ && pConsoleDropDown_->isOpen())
            {
                static const auto& menusTrigger = MachGui::inputRegistry()->getBinds("show-menus"_bind);
                if (e.state() == Gui::PRESSED)
                {
                    if (menusTrigger.matches(e.keyWithMods()))
                    {
                        toggleConsoleDropDown();
                    }
                    else
                    {
                        processed = true;
                        pConsoleDropDown_->doHandleKeyEvent(e);
                    }
                }
            }
            else
            {
                processed = pImpl_->pPromptText_->doHandleKeyEvent(e);
            }
            break;
        }
        case 1:
        {
            ASSERT(pImpl_->pCameras_, "pCameras_ is NULL");
            processed = pImpl_->pCameras_->processButtonEvent(e);

            static const auto& hidePanelTrigger = MachGui::inputRegistry()->getBinds("ui-controlpanel-hide"_bind);
            static const auto& showPanelTrigger = MachGui::inputRegistry()->getBinds("ui-controlpanel-show"_bind);

            if (e.state() == Gui::PRESSED)
            {
                if (pImpl_->controlPanelOn_ && hidePanelTrigger.matches(e.keyWithMods()))
                {
                    pImpl_->controlPanelOn_ = false;
                    processed = true;
                }
                else if (showPanelTrigger.matches(e.keyWithMods()))
                {
                    pImpl_->controlPanelOn_ = true;
                    processed = true;
                }
            }
            break;
        }
        case 2:
            ASSERT(pImpl_->pSquadronBank_, "pSquadronBank_ is NULL");
            processed = pImpl_->pSquadronBank_->processButtonEvent(e.buttonEvent());
            break;
        case 3:
            ASSERT(pImpl_->pMachineNavigation_, "pMachineNavigation_ is NULL");
            processed = pImpl_->pMachineNavigation_->processButtonEvent(e.buttonEvent());
            break;
        case 4:
            ASSERT(pImpl_->pConstructionNavigation_, "pConstructionNavigation_ is NULL");
            processed = pImpl_->pConstructionNavigation_->processButtonEvent(e.buttonEvent());
            break;
        case 5:
            ASSERT(pImpl_->pWorldViewWindow_, "pWorldViewWindow_ is NULL");
            processed = pImpl_->pWorldViewWindow_->processButtonEvent(e.buttonEvent());
            break;
        case 6:
        case 7:
            break;
        case 8:
            if (e.key() == Device::KeyCode::ESCAPE && e.state() == Gui::PRESSED)
            {
                if (isMachineNavigationContext() || isConstructionNavigationContext())
                {
                    mainMenuOrSingleFactoryContext();
                }
                else
                {
                    pImpl_->switchToMenus_ = true;
                }

                processed = true;
            }
            break;
        case 9: // F10 gets you into menus
        {
            static const auto& menusTrigger = MachGui::inputRegistry()->getBinds("show-menus"_bind);
            if (e.state() == Gui::PRESSED && menusTrigger.matches(e.keyWithMods()))
            {
                pImpl_->switchToMenus_ = true;
                processed = true;
            }
            break;
        }
        case 10: // Screen shot
        {
            static const auto& screenshotTrigger = MachGui::inputRegistry()->getBinds("screenshot"_bind);
            if (e.state() == Gui::PRESSED && screenshotTrigger.matches(e.keyWithMods()))
            {
                initiateScreenShot();
                processed = true;
            }
            break;
        }
        case 11: // Command hot keys
        {
            std::optional<uint> skipCommand;
            if (pActiveCommand_)
            {
                skipCommand = pActiveCommand_->cursorPromptStringId();
                processed = pActiveCommand_->processButtonEvent(e);
            }
            for (Commands::iterator iter = allCommands_.begin(); iter != allCommands_.end() && !processed; ++iter)
            {
                if (skipCommand.has_value() && ((*iter)->cursorPromptStringId() == skipCommand))
                {
                    continue;
                }
                processed = (*iter)->processButtonEvent(e);
            }
            break;
        }
        case 12: // Go inhead
        {
            static const auto& toggleFpvTrigger = MachGui::inputRegistry()->getBinds("view-toggle-fpv"_bind);
            if (e.state() == Gui::PRESSED && toggleFpvTrigger.matches(e.keyWithMods()))
            {
                if (pFirstPerson_->okayToSwitchTo1stPerson())
                {
                    switchToInHead();
                    processed = true;
                }
            }
            break;
        }
        case 13: // Pause game
            if (e.key() == Device::KeyCode::BREAK && e.state() == Gui::PRESSED)
            {
                // Can't pause game in multiplayer games
                if (!MachLogNetwork::instance().isNetworkGame() && actualGameState() == PLAYING)
                {
                    if (SimManager::instance().isSuspended())
                    {
                        SimManager::instance().resume();
                        MachLogVoiceMailManager::instance().acceptMailPostings(true);
                    }
                    else
                    {
                        SimManager::instance().suspend();
                        MachLogVoiceMailManager::instance().acceptMailPostings(false);
                    }

                    processed = true;
                }
            }
            break;
        case 14:
            processed = MachLogRecentEventsManager::instance().doHandleKeyEvent(e);
            break;
        default:
            finished = true;
            break;
        }

        NEIL_STREAM("Keyboard event " << count << " processed " << (processed ? "true" : "false") << std::endl);

        ++count;
    }

    NEIL_STREAM("COUNT " << count << std::endl);

#ifndef PRODUCTION
    if ((!getenv("cb_nokeyboardhacks")) && (!(count == 1)))
    {
        doHandleKeyEventHacks(e);
    }
#endif

    return processed;
}

bool MachInGameScreen::doHandleKeyEventHacks(const GuiKeyEvent& e)
{
#ifndef PRODUCTION

    NEIL_STREAM("MachInGameScreen::doHandleKeyEventHacks " << std::endl);

    // Quick hack to switch "fog of war" on/off
    if (e.key() == Device::KeyCode::KEY_M && e.isShiftPressed() && e.isCtrlPressed() && e.state() == Gui::PRESSED)
    {
        static bool fogOfWarOn = true;
        fogOfWarOn = !fogOfWarOn;
        pImpl_->pContinentMap_->fogOfWarOn(fogOfWarOn);
    }

    // *** Todo : This stuff needs sorting out. It's been taken from the Machines app and needs a serious tidy...
    // mouse events are handled seperately...
    PRE(e.key() != Device::KeyCode::MOUSE_LEFT);
    PRE(e.key() != Device::KeyCode::MOUSE_RIGHT);

    PhysMotionControl::processButtonEvent(e.buttonEvent());

    if (e.state() == Gui::PRESSED)
    {
        if (e.key() == Device::KeyCode::KEY_U)
        {
            if (e.isShiftPressed())
                pImpl_->pSceneManager_->autoAdjustFrameRate(false);
            else
                pImpl_->pSceneManager_->autoAdjustFrameRate(true);
        }

        if (e.key() == Device::KeyCode::F5 && e.isShiftPressed())
        {
            if (e.isCtrlPressed())
            {
                MemChkMemoryWatchingOn(false);
                BaseMemWatcher::instance().traceOustandingAllocations(ProProfiler::instance().outputStream());
                BaseMemWatcher::instance().clear();
            }
            else
                MemChkMemoryWatchingOn(true);
        }

        if (e.key() == Device::KeyCode::F7 && e.isShiftPressed() && e.isCtrlPressed())
        {
            static bool showStats = false;

            if (showStats)
                pImpl_->pSceneManager_->hideStats();
            else
                pImpl_->pSceneManager_->showStats(0.333);

            showStats = !showStats;
        }

        if (e.key() == Device::KeyCode::F8 && e.isShiftPressed() && e.isCtrlPressed())
        {
            for (MachInGameScreenImpl::Actors::iterator i = pImpl_->selectedActors_.begin();
                 i != pImpl_->selectedActors_.end();
                 ++i)
            {
                MachActor* pActor = *i;
                ASSERT(pActor, "");
                if (pActor->objectIsMachine())
                {
                    MachLogMachine& machine = pActor->asMachine();
                    machine.motionSeq().toggleLogging();
                }
            }
        }

        if (e.key() == Device::KeyCode::F9 && e.isShiftPressed() && e.isCtrlPressed())
        {
            pImpl_->showCurrentMachine_ = !pImpl_->showCurrentMachine_;
        }

        if (e.key() == Device::KeyCode::F11)
        {
            RENDER_STREAM(RenSurfaceManager::instance());
        }

        if (e.key() == Device::KeyCode::KEY_N && e.isShiftPressed())
        {
            pImpl_->showNetworkPing_ = e.isCtrlPressed();
        }
        if (e.key() == Device::KeyCode::ASTERISK_PAD)
        {
            if (e.isShiftPressed())
            {
                if (e.isCtrlPressed())
                {
                    NETWORK_STREAM_DISABLE;
                }
                else
                {
                    NETWORK_STREAM_ENABLE;
                }
            }
        }
    }
#endif

    return false;
}

/* End D3DAPP.CPP ***************************************************/
