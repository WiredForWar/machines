/*
 * C T X B R I E F . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
//

#include "machgui/ctxbrief.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/ui/ScrollableText.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/dbtexdat.hpp"
#include "machgui/dbscenar.hpp"
#include "machgui/dbplyscn.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/database.hpp"
#include "machgui/ui/VerticalScrollBar.hpp"
#include "machgui/gui.hpp"
#include "machgui/msgbox.hpp"
#include "machlog/dbhandlr.hpp"
#include "machlog/races.hpp"
#include "machgui/internal/strings.hpp"
#include "gui/image.hpp"
#include "ani/AniSmacker.hpp"
#include "ani/AniSmackerRegular.hpp"
#include "sound/soundmix.hpp"
#include "sound/smpparam.hpp"
#include "sound/sndwavid.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "device/cd.hpp"
#include <algorithm>

MachGuiCtxBriefing::MachGuiCtxBriefing(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sk", pStartupScreens)
    , pBriefImage_(nullptr)
    , playedMail_(false)
    , autoLoadGame_(false)
{
    pOkBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(131, 363, 301, 401) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_OK,
        MachGui::ButtonEvent::DUMMY_OK);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(132, 416, 301, 453) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_CANCEL,
        MachGui::ButtonEvent::EXIT);
    pHintsBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(357, 363, 526, 401) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_HINTS,
        MachGui::ButtonEvent::HINTS);
    pObjectiveBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(356, 416, 526, 453) * MachGui::menuScaleFactor(),
        IDS_MENUBTN_OBJECTIVES,
        MachGui::ButtonEvent::OBJECTIVES);

    pCancelBtn->escapeControl(true);

    pTextInfo_ = new MachGuiScrollableText(pStartupScreens, Gui::Box(23, 21, 305, 336) * MachGui::menuScaleFactor());
    MachGuiVerticalScrollBar::createWholeBar(
        pStartupScreens,
        Gui::Coord(306, 21) * MachGui::menuScaleFactor(),
        315 * MachGui::menuScaleFactor(),
        pTextInfo_);

    displayObjectives();
    displayBriefingImage();
    playBriefingVoicemail();

    TEST_INVARIANT;
}

MachGuiCtxBriefing::~MachGuiCtxBriefing()
{
    stopPlayingBriefingVoicemail();
    TEST_INVARIANT;
}

void MachGuiCtxBriefing::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxBriefing& t)
{

    o << "MachGuiCtxBriefing " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxBriefing " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxBriefing::update()
{
    if (autoLoadGame_)
    {
        pStartupScreens_->buttonAction(MachGui::ButtonEvent::STARTGAME);
    }
    else
    {
        animations_.update();
    }
}

void MachGuiCtxBriefing::displayObjectives()
{
    // Disable the objective button, enable hints button if hints are available.
    pObjectiveBtn_->disabled(true);

    MachGuiDbPlayerScenario* pPlayerScenario = nullptr;

    if (MachGuiDatabase::instance().currentPlayer().hasPlayed(
            *pStartupScreens_->startupData()->scenario(),
            &pPlayerScenario))
    {
        pHintsBtn_->disabled(false);

        if (pObjectiveBtn_->isFocusControl())
        {
            pHintsBtn_->hasFocus(true);
            pObjectiveBtn_->hasFocus(false);
        }
    }
    else
    {
        pHintsBtn_->disabled(true);

        if (pObjectiveBtn_->isFocusControl())
        {
            pOkBtn_->hasFocus(true);
            pObjectiveBtn_->hasFocus(false);
        }
    }

    std::string objectiveAndTaskText;
    // Display of objective info.
    const MachGuiDbTextData& textData = pStartupScreens_->startupData()->scenario()->textData();
    if (textData.nObjectives())
    {
        objectiveAndTaskText = textData.objectiveText(0);
    }

    GuiResourceString taskHeading(IDS_TASKS);
    GuiResourceString taskCompleted(IDS_COMPLETEDTASK);

    if (pStartupScreens_->currentContext() == MachGuiStartupScreens::CTX_BRIEFING)
    {
        // Display task info before game has started
        for (int index = 0; index < textData.nTasks(); ++index)
        {
            if (MachLogRaces::instance().databaseHandler().taskStartsAvailable(index))
            {
                if (index == 0)
                {
                    objectiveAndTaskText += "\n<w>";
                    objectiveAndTaskText += taskHeading.asString() + "\n";
                }
                objectiveAndTaskText += "\n";
                objectiveAndTaskText += textData.taskText(index);
            }
        }
    }
    else
    {
        // Display task info after game has started
        for (int index = 0; index < textData.nTasks(); ++index)
        {
            if (MachLogRaces::instance().databaseHandler().taskIsAvailable(index))
            {
                if (index == 0)
                {
                    objectiveAndTaskText += "\n<w>";
                    objectiveAndTaskText += taskHeading.asString() + "\n";
                }

                objectiveAndTaskText += "\n";
                objectiveAndTaskText += textData.taskText(index);

                if (MachLogRaces::instance().databaseHandler().taskIsComplete(index))
                {
                    objectiveAndTaskText += "<w>";
                    objectiveAndTaskText += taskCompleted.asString() + "\n";
                }
            }
        }
    }

    pTextInfo_->setText(objectiveAndTaskText);
}

void MachGuiCtxBriefing::displayHints()
{
    pHintsBtn_->disabled(true);
    pObjectiveBtn_->disabled(false);

    if (pHintsBtn_->isFocusControl())
    {
        pObjectiveBtn_->hasFocus(true);
        pHintsBtn_->hasFocus(false);
    }

    const MachGuiDbTextData& textData = pStartupScreens_->startupData()->scenario()->textData();
    pTextInfo_->setText(textData.hintText());
}

///////////////////////////////////////////////////////////////////////////////////////////

class MachGuiStartBriefingMessageBoxResponder : public MachGuiMessageBoxResponder
{
public:
    MachGuiStartBriefingMessageBoxResponder(MachGuiCtxBriefing* pBriefingCtx)
        : pBriefingCtx_(pBriefingCtx)
    {
    }

    bool okPressed() override
    {
        // if ( MachGui::machinesCDIsAvailable( 2 ) )
        {
            pBriefingCtx_->autoLoadGame();
            return true;
        }
        // return false;
    }

    bool cancelPressed() override
    {
        pBriefingCtx_->restartVoiceMailAndAnimation();
        return true;
    }

private:
    MachGuiCtxBriefing* pBriefingCtx_;
};

///////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiCtxBriefing::buttonEvent(MachGui::ButtonEvent be)
{
    if (be == MachGui::ButtonEvent::HINTS)
    {
        displayHints();
    }
    else if (be == MachGui::ButtonEvent::OBJECTIVES)
    {
        displayObjectives();
    }
    else if (be == MachGui::ButtonEvent::DUMMY_OK)
    {
        pStartupScreens_->buttonAction(MachGui::ButtonEvent::STARTGAME);
    }
}

// virtual
bool MachGuiCtxBriefing::okayToSwitchContext()
{
    if ((pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::STARTGAME)
        || (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::CANCEL))
    {
        // silence voicemail
        stopPlayingBriefingVoicemail();
    }

    return true;
}

void MachGuiCtxBriefing::displayBriefingImage()
{
    if (pBriefImage_)
    {
        delete pBriefImage_;
        pBriefImage_ = nullptr;
    }
    pStartupScreens_->clearAllSmackerAnimations();
    const std::string& briefPath = pStartupScreens_->startupData()->scenario()->briefingPicture();
    SysPathName briefPicture(briefPath);
    WAYNE_STREAM("MachGuiCtxScenario::updateSelectedScenario brief filename: " << briefPicture << std::endl);

    // Get flic off hard-disk or CD-Rom
    if (! briefPicture.existsAsFile())
    {
        // Make sure the cd is stopped before accessing files on it.
        if (DevCD::instance().isPlayingAudioCd())
        {
            DevCD::instance().stopPlaying();
        }

        std::string cdRomDrive;

        if (MachGui::getCDRomDriveContainingFile(cdRomDrive, briefPath))
        {
            briefPicture = SysPathName(cdRomDrive + briefPath);

            // Can't play music and smacker anim off CD at same time
            if (briefPicture.existsAsFile())
            {
                pStartupScreens_->desiredCdTrack(MachGuiStartupScreens::DONT_PLAY_CD);
            }
        }
        else
        {
            briefPicture = SysPathName("gui/menu/cdb.smk");
        }
    }

    if (briefPicture.existsAsFile())
    {
        const Gui::Coord animationCoord = Gui::Coord(355, 56) * MachGui::menuScaleFactor();
        if (briefPicture.extension() == "smk")
        {
            // File is smacker file
            const auto& topLeft = getBackdropTopLeft();
            AniSmacker* pSmackerAnimation = new AniSmackerRegular(
                briefPicture,
                animationCoord.x() + topLeft.second,
                animationCoord.y() + topLeft.first);
            pSmackerAnimation->setScaleFactor(MachGui::menuScaleFactor());
            pStartupScreens_->addSmackerAnimation(pSmackerAnimation);
        }
        else if (briefPicture.extension() == "bmp")
        {
            // File is a bitmap
            pBriefImage_ = new GuiImage(
                pStartupScreens_,
                animationCoord,
                MachGui::getScaledImage(briefPath, MachGui::menuScaleFactor()));
        }
    }
}

void MachGuiCtxBriefing::playBriefingVoicemail()
{
    stopPlayingBriefingVoicemail();

    const MachGuiDbTextData& textData = pStartupScreens_->startupData()->scenario()->textData();
    if (textData.nObjectives())
    {
        if (textData.hasobjectiveVoicemail(0))
        {
            Snd::LoopCount loopCount = 0; // infinite loop
            std::string voicemail = textData.objectiveVoicemail(0);
            std::transform(voicemail.begin(), voicemail.end(), voicemail.begin(), ::tolower);
            SndSampleParameters voicemailParameters(SndWaveformId(voicemail), loopCount);

            briefVoicemail_ = SndMixer::instance().playSample(voicemailParameters);
            playedMail_ = true;
        }
    }
}

void MachGuiCtxBriefing::stopPlayingBriefingVoicemail()
{
    if (playedMail_ && SndMixer::instance().isActive(briefVoicemail_))
    {
        SndMixer::instance().stopSample(briefVoicemail_);
        SndMixer::instance().freeSampleResources(briefVoicemail_);
    }
}

void MachGuiCtxBriefing::autoLoadGame()
{
    autoLoadGame_ = true;
}

void MachGuiCtxBriefing::restartVoiceMailAndAnimation()
{
    displayBriefingImage();
    playBriefingVoicemail();
}

/* End CTXBRIEF.CPP *************************************************/
