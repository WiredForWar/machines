/*
 * C T X D E B R F . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "network/netnet.hpp"
#include "machgui/ctxdebrf.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/dbscenar.hpp"
#include "machgui/ingame.hpp"
#include "machgui/ui/ScrollableText.hpp"
#include "machgui/dbtexdat.hpp"
#include "machgui/ui/VerticalScrollBar.hpp"
#include "machlog/races.hpp"
#include "machlog/dbhandlr.hpp"
#include "machlog/network.hpp"
#include "machlog/cntrl_pc.hpp"
#include "machgui/internal/strings.hpp"
#include "system/pathname.hpp"
#include "gui/image.hpp"
#include "sound/sndwavid.hpp"
#include "sound/smpparam.hpp"
#include "ani/AniSmacker.hpp"
#include "ani/AniSmackerRegular.hpp"
#include "system/pathname.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "device/cd.hpp"

MachGuiCtxDeBriefing::MachGuiCtxDeBriefing(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sj", pStartupScreens)
    , pDebriefImage_(nullptr)
    , playedMail_(false)
{
    // Create menu buttons. Order of creation denotes TAB order.
    MachGuiMenuButton* pStatisticsButton = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(87, 206, 279, 248),
        IDS_MENUBTN_STATS,
        MachGui::ButtonEvent::STATISTICS);
    MachGuiMenuButton* pContinueButton;
    // display back to lobby on button for zone games.
    if (! NetNetwork::instance().isLobbiedGame())
        pContinueButton = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(87, 310, 279, 353),
            IDS_MENUBTN_CONTINUE,
            MachGui::ButtonEvent::CONTINUE);
    else
        pContinueButton = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(87, 310, 279, 353),
            IDS_MENUBTN_BACK_TO_ZONE,
            MachGui::ButtonEvent::CONTINUE);

    MachGuiMenuButton* pRestartButton = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(87, 100, 279, 142),
        IDS_MENUBTN_RESTART,
        MachGui::ButtonEvent::RESTART);

#ifdef DEMO
    pStatisticsButton->disabled(true);
    pStatisticsButton->hasFocus(false);
    pContinueButton->hasFocus(true);
#endif

    if (pStartupScreens->currentContext() == MachGuiStartupScreens::CTX_MPDEBRIEFING)
        pRestartButton->disabled(true);

    displayDeBriefImage();

    MachGuiScrollableText* pTextInfo = new MachGuiScrollableText(pStartupScreens, Gui::Box(359, 265, 558, 426));
    MachGuiVerticalScrollBar::createWholeBar(pStartupScreens, Gui::Coord(559, 265), 160, pTextInfo);

    string winLoseText;
    // Display win/lose text...
    switch (pStartupScreens_->inGameScreen().gameState())
    {
        case MachInGameScreen::WON:
            winLoseText = pStartupScreens->startupData()->scenario()->textData().winText();
            break;
        case MachInGameScreen::LOST:
            winLoseText = pStartupScreens->startupData()->scenario()->textData().loseText();
            break;
            DEFAULT_ASSERT_BAD_CASE(pStartupScreens_->inGameScreen().gameState());
    }

    GuiResourceString taskHeading(IDS_TASKS);
    GuiResourceString taskCompleted(IDS_COMPLETEDTASK);
    GuiResourceString taskFailed(IDS_FAILEDTASK);
    const MachGuiDbTextData& textData = pStartupScreens_->startupData()->scenario()->textData();

    // Display task info, showing which tasks were completed
    for (int index = 0; index < textData.nTasks(); ++index)
    {
        if (MachLogRaces::instance().databaseHandler().taskIsAvailable(index))
        {
            if (index == 0)
            {
                winLoseText += "\n<w>";
                winLoseText += taskHeading.asString() + "\n";
            }

            winLoseText += "\n";
            winLoseText += textData.taskText(index);

            // Add "Failed" or "Completed" to task text
            winLoseText += "<w>";
            if (MachLogRaces::instance().databaseHandler().taskIsComplete(index))
            {
                winLoseText += taskCompleted.asString() + "\n";
            }
            else
            {
                winLoseText += taskFailed.asString() + "\n";
            }
        }
    }

    pTextInfo->setText(winLoseText);

    playDeBriefingVoicemail();

    // This might break the lobbying code - can't remember why I removed it - probably find out later :)
    //   string ct = pStartupScreens->startupData()->connectionType();
    //   MachLogNetwork::instance().terminateAndReset();
    //   pStartupScreens->startupData()->connectionType( ct );

    TEST_INVARIANT;
}

MachGuiCtxDeBriefing::~MachGuiCtxDeBriefing()
{
    TEST_INVARIANT;

    stopPlayingDeBriefingVoicemail();
}

void MachGuiCtxDeBriefing::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxDeBriefing& t)
{

    o << "MachGuiCtxDeBriefing " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxDeBriefing " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxDeBriefing::update()
{
    animations_.update();
}

// virtual
void MachGuiCtxDeBriefing::buttonEvent(MachGui::ButtonEvent be)
{
    if (be == MachGui::ButtonEvent::RESTART)
    {
        stopPlayingDeBriefingVoicemail();
        pStartupScreens_->restartGame();
    }
}

// virtual
bool MachGuiCtxDeBriefing::okayToSwitchContext()
{
    stopPlayingDeBriefingVoicemail();

    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::CONTINUE)
    {
        // Unload the game that has just finished.
        pStartupScreens_->unloadGame();

        // if we got into this screen via a lobby session then we need to terminate correctly at this point.
        if (NetNetwork::instance().isLobbiedGame())
        {
            MachLogNetwork::instance().terminateAndReset();
            pStartupScreens_->contextFinishFromLobby();
            return false;
        }
    }

    return true;
}

void MachGuiCtxDeBriefing::displayDeBriefImage()
{
    if (pDebriefImage_)
    {
        delete pDebriefImage_;
        pDebriefImage_ = nullptr;
    }
    pStartupScreens_->clearAllSmackerAnimations();
    string debriefPath;

    // get either win or lose debrief picture
    switch (pStartupScreens_->inGameScreen().gameState())
    {
        case MachInGameScreen::WON:
            if (pStartupScreens_->gameType() != MachGuiStartupScreens::CAMPAIGNGAME)
            {
                switch (MachLogRaces::instance().pcController().race())
                {
                    case MachPhys::RED:
                        debriefPath = "flics/gui/v&d/vicr.bmp";
                        break;
                    case MachPhys::YELLOW:
                        debriefPath = "flics/gui/v&d/vicy.bmp";
                        break;
                    case MachPhys::BLUE:
                        debriefPath = "flics/gui/v&d/vicb.bmp";
                        break;
                    case MachPhys::GREEN:
                        debriefPath = "flics/gui/v&d/vicg.bmp";
                        break;
                }
            }
            else
            {
                debriefPath = pStartupScreens_->startupData()->scenario()->debriefingPicture();
            }
            break;
        case MachInGameScreen::LOST:
            if (pStartupScreens_->gameType() != MachGuiStartupScreens::CAMPAIGNGAME)
            {
                switch (MachLogRaces::instance().pcController().race())
                {
                    case MachPhys::RED:
                        debriefPath = "flics/gui/v&d/defr.bmp";
                        break;
                    case MachPhys::YELLOW:
                        debriefPath = "flics/gui/v&d/defy.bmp";
                        break;
                    case MachPhys::BLUE:
                        debriefPath = "flics/gui/v&d/defb.bmp";
                        break;
                    case MachPhys::GREEN:
                        debriefPath = "flics/gui/v&d/defg.bmp";
                        break;
                }
            }
            else
            {
                debriefPath = pStartupScreens_->startupData()->scenario()->debriefingLosePicture();
            }
            break;
            DEFAULT_ASSERT_BAD_CASE(pStartupScreens_->inGameScreen().gameState());
    }

    SysPathName debriefPicture(debriefPath);
    WAYNE_STREAM("MachGuiCtxScenario::updateSelectedScenario debrief filename: " << debriefPicture << std::endl);

    // Get flic off hard-disk or CD-Rom
    if (! debriefPicture.existsAsFile())
    {
        // Make sure the cd is stopped before accessing files on it.
        if (DevCD::instance().isPlayingAudioCd())
        {
            DevCD::instance().stopPlaying();
        }

        string cdRomDrive;

        if (MachGui::getCDRomDriveContainingFile(cdRomDrive, debriefPath))
        {
            debriefPicture = SysPathName(cdRomDrive + debriefPath);

            // Can't play music and smacker anim off CD at same time
            if (debriefPicture.existsAsFile())
            {
                pStartupScreens_->desiredCdTrack(MachGuiStartupScreens::DONT_PLAY_CD);
            }
        }
    }

    if (debriefPicture.existsAsFile())
    {
        if (debriefPicture.extension() == "smk")
        {
            // File is smacker file
            // Construct a smacker player
            //          HWND targetWindow = RenDevice::current()->display()->window();

            //          AniSmacker* pSmackerAnimation = new AniSmacker( debriefPicture, targetWindow, 359 +
            //          pStartupScreens_->xMenuOffset(), 51 + pStartupScreens_->yMenuOffset() );
            // AniSmacker* pSmackerAnimation = new AniSmacker( debriefPicture, 359 + pStartupScreens_->xMenuOffset(),
            // 51 + pStartupScreens_->yMenuOffset() );
            const auto& topLeft = getBackdropTopLeft();
            AniSmacker* pSmackerAnimation
                = new AniSmackerRegular(debriefPicture, 359 + topLeft.second, 51 + topLeft.first);
            pStartupScreens_->addSmackerAnimation(pSmackerAnimation);
        }
        else if (debriefPicture.extension() == "bmp")
        {
            // File is a bitmap
            pDebriefImage_ = new GuiImage(pStartupScreens_, Gui::Coord(359, 51), Gui::bitmap(debriefPicture));
        }
    }
}

void MachGuiCtxDeBriefing::playDeBriefingVoicemail()
{
    stopPlayingDeBriefingVoicemail();

    const MachGuiDbTextData& textData = pStartupScreens_->startupData()->scenario()->textData();

    switch (pStartupScreens_->inGameScreen().gameState())
    {
        case MachInGameScreen::WON:
            if (textData.hasWinVoicemail())
            {
                Snd::LoopCount loopCount = 0; // infinite loop
                SndSampleParameters voicemailParameters(SndWaveformId(textData.winVoicemail()), loopCount);

                debriefVoicemail_ = SndMixer::instance().playSample(voicemailParameters);
                playedMail_ = true;
            }
            break;
        case MachInGameScreen::LOST:
            if (textData.hasLoseVoicemail())
            {
                Snd::LoopCount loopCount = 0; // infinite loop
                SndSampleParameters voicemailParameters(SndWaveformId(textData.loseVoicemail()), loopCount);

                debriefVoicemail_ = SndMixer::instance().playSample(voicemailParameters);
                playedMail_ = true;
            }
            break;
            DEFAULT_ASSERT_BAD_CASE(pStartupScreens_->inGameScreen().gameState());
    }
}

void MachGuiCtxDeBriefing::stopPlayingDeBriefingVoicemail()
{
    if (playedMail_ && SndMixer::instance().isActive(debriefVoicemail_))
    {
        SndMixer::instance().stopSample(debriefVoicemail_);
        SndMixer::instance().freeSampleResources(debriefVoicemail_);
    }
}

/* End CTXDEBRF.CPP *************************************************/
