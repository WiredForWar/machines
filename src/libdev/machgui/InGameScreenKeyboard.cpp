#include "base/MemWatcher.hpp"
#include "machgui/IInputRegistry.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/ProductionBank.hpp"
#include "machgui/HWResearchBank.hpp"
#include "machgui/Cameras.hpp"
#include "machgui/ConsoleDropDown.hpp"
#include "machgui/SquadronBank.hpp"
#include "machgui/ContinentMap.hpp"
#include "machgui/NavigatorBase.hpp"
#include "machgui/WorldViewWindow.hpp"
#include "machgui/commands/command.hpp"
#include "machgui/FirstPerson.hpp"
#include "machgui/PromptText.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "phys/MotionControl/Motion.hpp"
#include "phys/Plans/MotionChunk.hpp"
#include "mathex/Point3d.hpp"
#include "mathex/EulerAngles.hpp"
#include "mathex/Polygon2d.hpp"
#include "profiler/Profiler.hpp"
#include "sim/Manager.hpp"
#include "gui/Event.hpp"
#include "gui/gui.hpp"
#include "machlog/Messaging/RecentEventsManager.hpp"
#include "machlog/Actors/MotionSequencer.hpp"
#include "machlog/Messaging/VoiceMailManager.hpp"
#include "machphys/Machines/MachineMoveInfo.hpp"
#include "render/SurfaceManager.hpp"
#include "render/RenderVariables.hpp"
#include "machlog/Messaging/Network.hpp"

#include "system/ConfigVariables.hpp"

#include <optional>

bool MachInGameScreen::doHandleKeyEvent(const GuiKeyEvent& e)
{
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
                processed = pConsoleDropDown_->doHandleKeyEvent(e);
                static const auto& menusTrigger = MachGui::inputRegistry()->getBinds("show-menus"_bind);
                // Check 'processed' to trigger the menu only if the event was not explicitly consumed by the console
                if (!processed && e.state() == Gui::PRESSED)
                {
                    if (menusTrigger.matches(e.keyWithMods()))
                    {
                        toggleConsoleDropDown();
                        processed = true;
                    }
                }

                // Assume the event is anyway processed if the console was open.
                // This prevents leakage of doHandleCharEvent()
                processed = true;
            }
            else
            {
                processed = pPromptText_->doHandleKeyEvent(e);
            }
            break;
        }
        case 1:
        {
            ASSERT(pCameras_, "pCameras_ is NULL");
            processed = pCameras_->processButtonEvent(e);

            static const auto& hidePanelTrigger = MachGui::inputRegistry()->getBinds("ui-controlpanel-hide"_bind);
            static const auto& showPanelTrigger = MachGui::inputRegistry()->getBinds("ui-controlpanel-show"_bind);

            if (e.state() == Gui::PRESSED)
            {
                if (controlPanelOn_ && hidePanelTrigger.matches(e.keyWithMods()))
                {
                    controlPanelOn_ = false;
                    processed = true;
                }
                else if (showPanelTrigger.matches(e.keyWithMods()))
                {
                    controlPanelOn_ = true;
                    processed = true;
                }
            }
            break;
        }
        case 2:
            ASSERT(pSquadronBank_, "pSquadronBank_ is NULL");
            processed = pSquadronBank_->processButtonEvent(e.buttonEvent());
            break;
        case 3:
            ASSERT(pMachineNavigation_, "pMachineNavigation_ is NULL");
            processed = pMachineNavigation_->processButtonEvent(e.buttonEvent());
            break;
        case 4:
            ASSERT(pConstructionNavigation_, "pConstructionNavigation_ is NULL");
            processed = pConstructionNavigation_->processButtonEvent(e.buttonEvent());
            break;
        case 5:
            ASSERT(pWorldViewWindow_, "pWorldViewWindow_ is NULL");
            processed = pWorldViewWindow_->processButtonEvent(e.buttonEvent());
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
                    switchToMenus_ = true;
                }

                processed = true;
            }
            break;
        case 9: // F10 gets you into menus
        {
            static const auto& menusTrigger = MachGui::inputRegistry()->getBinds("show-menus"_bind);
            if (e.state() == Gui::PRESSED && menusTrigger.matches(e.keyWithMods()))
            {
                switchToMenus_ = true;
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

            static const auto & toggleRendering = MachGui::inputRegistry()->getBinds("gfx-toggle-rendering"_bind);
            if (e.state() == Gui::PRESSED && toggleRendering.matches(e.keyWithMods()))
            {
                Config::gfxModernRendering.set(!Config::gfxModernRendering.get());
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
        case 14: // Look at the recent events in turn
            if (e.key() == Device::KeyCode::SPACE && e.state() == Gui::PRESSED)
            {
                MachLogRecentEventsManager& recentEvents = MachLogRecentEventsManager::instance();
                if (recentEvents.hasEvents())
                {
                    ASSERT(pCameras_, "pCameras_ is NULL");
                    pCameras_->moveTo(recentEvents.nextEventPosition());
                }

                processed = true;
            }
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
#else
    if (e.state() == Gui::PRESSED)
    {
        if (e.key() == Device::KeyCode::F7 && e.isShiftPressed() && e.isCtrlPressed())
        {
            Config::debugShowRenderStats.set(!Config::debugShowRenderStats.get());
            processed = true;
        }
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
        pContinentMap_->fogOfWarOn(fogOfWarOn);
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
                pSceneManager_->autoAdjustFrameRate(false);
            else
                pSceneManager_->autoAdjustFrameRate(true);
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
            Config::debugShowRenderStats.set(!Config::debugShowRenderStats.get());
        }

        if (e.key() == Device::KeyCode::F8 && e.isShiftPressed() && e.isCtrlPressed())
        {
            for (MachInGameScreen::Actors::iterator i = selectedActors_.begin();
                 i != selectedActors_.end();
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
            showCurrentMachine_ = !showCurrentMachine_;
        }

        if (e.key() == Device::KeyCode::F11)
        {
            RENDER_STREAM(RenSurfaceManager::instance());
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
