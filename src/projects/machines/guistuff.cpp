#include <cstdio>
#include "base/Diag.hpp"
#include "machines/sdlapp.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "render/Device.hpp"
#include "gui/Manager.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/MessageBroker.hpp"
#include "machgui/StartupData.hpp"
#include "machgui/db/Database.hpp"
#include "machgui/db/DbPlanet.hpp"
#include "machgui/db/DbScenario.hpp"
#include "machgui/db/DbSystem.hpp"
#include "machlog/Messaging/Network.hpp"
#include "network/Network.hpp"
#include "recorder/Recorder.hpp"
#include "device/Timer.hpp"
#include "device/Time.hpp"
#include "machines/scrndump.hpp"
#include "sim/Manager.hpp"

#include "system/ConfigVariables.hpp"

#include "spdlog/spdlog.h"

void debugTiming(const char*, bool);

void SDLApp::cleanUpGui()
{
    delete pStartupScreens_;
    pStartupScreens_ = nullptr;
}

void SDLApp::initializeGuiDevice(RenDevice &device)
{
    spdlog::info("Initializing GUI rendering...");
    Gui::backBuffer(device.backSurface());
}

void SDLApp::initialiseGui(StartedFromLobby startedFromLobby, IProgressReporter* pReporter)
{
    spdlog::info("Initializing GUI...");
    RenSurface backBuf = manager_->pDevice()->backSurface();
    spdlog::info("SWScale enabled: {}", USE_SWSCALE != 0);

    HAL_STREAM("SDLApp::initialiseGui new MachGuiStartupScreens\n");
    pStartupScreens_ = new MachGuiStartupScreens(manager_, pRoot_, pReporter);
    if (startedFromLobby == LOBBY_START)
    {
        // have to verify that the system really is in a lobbied state and doesn't just think it is.
        if (NetNetwork::instance().isLobbiedGame())
        {
            pStartupScreens_->startupData()->playerName(NetNetwork::instance().localPlayerName());
            if (MachLogNetwork::instance().isNodeLogicalHost())
            {
                pStartupScreens_->startupData()->hostGame(true);
            }
            else
            {
                pStartupScreens_->startupData()->hostGame(false);
                // Send join message so that host can update availablePlayers list etc.
                pStartupScreens_->messageBroker().sendJoinMessage(
                    pStartupScreens_->startupData()->playerName(),
                    pStartupScreens_->startupData()->uniqueMachineNumber());
            }
            // Initialise multiplayer settings ( assumes at least 1 planet and 1 scenario in Small map size ).
            MachGuiDbSystem& system = MachGuiDatabase::instance().multiPlayerSystem(MachGuiDatabase::SMALL);
            MachGuiDbPlanet& planet = system.planet(0);
            MachGuiDbScenario& scenario = planet.scenario(0);
            pStartupScreens_->startupData()->scenario(&scenario);
            pStartupScreens_->switchContext(MachGuiStartupScreens::CTX_IMREADY);
        }
    }

    HAL_STREAM("SDLApp::initialiseGui complete\n");
}

void SDLApp::updateGui()
{
    debugTiming("D3Dapp:: update gui", true);

    pStartupScreens_->updateGui();

    debugTiming("D3Dapp:: update gui complete", false);
}

void SDLApp::displayGui()
{
    debugTiming("D3Dapp:: display gui", true);

    pStartupScreens_->displayGui();

    debugTiming("D3Dapp:: display gui complete", false);
}

void SDLApp::checkFinishApp()
{
    if (pStartupScreens_->finishApp())
        finish();
}

//  TBD: The display of recorder info should be on a keypush somewhere.
static bool displayRecorderInfo()
{
    static bool result = !(
        getenv("cb_recorder_display") &&
        (strcasecmp(getenv("cb_recorder_display"), "off") == 0));

    return result;
}

// Update the display.
void SDLApp::loopCycle()
{
    if (displayRecorderInfo())
    {
        if (RecRecorder::instance().state() != RecRecorder::INACTIVE)
            RenDevice::current()->out() << RecRecorder::instance() << std::endl;

        if (RecRecorder::instance().state() == RecRecorder::PLAYING)
            RenDevice::current()->out() << SimManager::instance().currentTime() << std::endl;
    }

    // Prevent processing before clientStartup() call
    if (! initialised_)
    {
        frameTimer_.time(0.0);
        return;
    }

    checkFinishApp();

    MachScreenDumper::instance().update();

    pStartupScreens_->loopCycle();

    MachScreenDumper::instance().dump();

    applyFrameRateLimit();
}

void SDLApp::applyFrameRateLimit()
{
    if (targetFrameRate_ <= 0)
    {
        frameTimer_.time(0.0);
        return;
    }

    const double targetSeconds = 1.0 / static_cast<double>(targetFrameRate_);
    double frameElapsed = frameTimer_.time();

    constexpr double AheadThresholdSeconds = 0.003;
    constexpr double MinSleep = 1;
    constexpr double MaxSleep = 5;

    double newSleepMs = frameSleepMs_;
    if (frameElapsed + AheadThresholdSeconds < targetSeconds)
    {
        double requestedSleepMs = std::clamp(frameSleepMs_, MinSleep, MaxSleep);

        const double sleepStart = DevTime::instance().time();
        // TODO: Try SDL_DelayNS() of SDL3
        SDL_Delay(static_cast<uint32_t>(requestedSleepMs));
        const double sleptMs = (DevTime::instance().time() - sleepStart) * 1000.0;
        frameElapsed = frameTimer_.time();

        if (frameElapsed > targetSeconds)
        {
            const double overshootMs = (frameElapsed - targetSeconds) * 1000.0;
            newSleepMs = frameSleepMs_ - static_cast<uint32_t>(overshootMs + 1);
        }
        else
        {
            const double headroomMs = (targetSeconds - frameElapsed) * 1000.0;
            if (headroomMs > (AheadThresholdSeconds + 0.001) && sleptMs >= requestedSleepMs - 0.25)
            {
                newSleepMs = frameSleepMs_ + 0.5;
            }
        }
    }
    else
    {
        newSleepMs = MinSleep;
    }

    const double clampedSleepMs = std::clamp(newSleepMs, MinSleep, MaxSleep);

#ifndef PRODUCTION
    uint32_t oldSleep = frameSleepMs_;
    uint32_t newSleep = clampedSleepMs;

    if (newSleep > oldSleep)
    {
        spdlog::debug(
            "Frame sleep increased to {:.2f} ms (target FPS {})",
            clampedSleepMs,
            targetFrameRate_);
    }
    else if (newSleep < oldSleep)
    {
        spdlog::debug(
            "Frame sleep decreased to {:.2f} ms (target FPS {})",
            clampedSleepMs,
            targetFrameRate_);
    }
#endif

    frameSleepMs_ = clampedSleepMs;

    frameTimer_.time(0.0);
}

void SDLApp::activateGui()
{
    if (pStartupScreens_)
        pStartupScreens_->activate();
}

void SDLApp::initDefaultFontSize(float scaleFactor)
{
    int size = 10 * scaleFactor;

    static bool first = true;
    if (first)
    {
        first = false;

        const char* envVar = getenv("CB_RENDER_FONT");
        if (envVar)
        {
            char* copy = strdup(envVar);
            strtok(copy, ":");
            const char* sizeStr = strtok(nullptr, ":");

            if (sizeStr)
                size = atoi(sizeStr);

            free(copy);
        }
    }

    RenSurface::setDefaultFontSize(size);
}
