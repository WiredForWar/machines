/*
 * C T X B R I E F . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
//

#include "machgui/ctxbrief.hpp"
#include "machgui/menubutn.hpp"
#include "machgui/menutext.hpp"
#include "machgui/scrltext.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/dbtexdat.hpp"
#include "machgui/dbscenar.hpp"
#include "machgui/dbplyscn.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/database.hpp"
#include "machgui/scrolbar.hpp"
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
    : MachGuiStartupScreenContext(pStartupScreens)
    , animations_(pStartupScreens, SysPathName("gui/menu/sk_anims.anm"))
    , pBriefImage_(nullptr)
    , playedMail_(false)
    , autoLoadGame_(false)
{
    pOkBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(131, 363, 301, 401),
        IDS_MENUBTN_OK,
        MachGuiStartupScreens::BE_DUMMY_OK);
    MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(132, 416, 301, 453),
        IDS_MENUBTN_CANCEL,
        MachGuiStartupScreens::EXIT);
    pHintsBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(357, 363, 526, 401),
        IDS_MENUBTN_HINTS,
        MachGuiStartupScreens::HINTS);
    pObjectiveBtn_ = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(356, 416, 526, 453),
        IDS_MENUBTN_OBJECTIVES,
        MachGuiStartupScreens::OBJECTIVES);

    pCancelBtn->escapeControl(true);

    changeBackdrop("gui/menu/sk.bmp");

    pStartupScreens->cursorOn(true);
    pStartupScreens->desiredCdTrack(MachGuiStartupScreens::MENU_MUSIC);

    pTextInfo_ = new MachGuiScrollableText(pStartupScreens, Gui::Box(23, 21, 305, 336));
    MachGuiVerticalScrollBar::createWholeBar(pStartupScreens, Gui::Coord(306, 21), 315, pTextInfo_);

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
        pStartupScreens_->buttonAction(MachGuiStartupScreens::STARTGAME);
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

    string objectiveAndTaskText;
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
void MachGuiCtxBriefing::buttonEvent(MachGuiStartupScreens::ButtonEvent be)
{
    if (be == MachGuiStartupScreens::HINTS)
    {
        displayHints();
    }
    else if (be == MachGuiStartupScreens::OBJECTIVES)
    {
        displayObjectives();
    }
    else if (be == MachGuiStartupScreens::BE_DUMMY_OK)
    {
#ifndef DEMO
// if ( MachGui::machinesCDIsAvailable( 2 )
#ifndef PRODUCTION
//   or getenv("cb_dontcheckcd") )
#else
//      )
#endif
        {
            pStartupScreens_->buttonAction(MachGuiStartupScreens::STARTGAME);
        }
// else
//{
//  silence voicemail
//   stopPlayingBriefingVoicemail();
//  Stop playing briefing animation
//   pStartupScreens_->clearAllSmackerAnimations();

//  pStartupScreens_->displayMsgBox( IDS_MENUMSG_INSERTCD, new MachGuiStartBriefingMessageBoxResponder(this) );
//}
#else // DEMO
        {
            pStartupScreens_->buttonAction(MachGuiStartupScreens::STARTGAME);
        }
#endif
    }
}

// virtual
bool MachGuiCtxBriefing::okayToSwitchContext()
{
    if ((pStartupScreens_->lastButtonEvent() == MachGuiStartupScreens::STARTGAME)
        || (pStartupScreens_->lastButtonEvent() == MachGuiStartupScreens::BE_CANCEL))
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
    const string& briefPath = pStartupScreens_->startupData()->scenario()->briefingPicture();
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

        string cdRomDrive;

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
        if (briefPicture.extension() == "smk")
        {
            // File is smacker file
            // Construct a smacker player
            //          HWND targetWindow = RenDevice::current()->display()->window();

            //         AniSmacker* pSmackerAnimation = new AniSmacker( briefPicture, targetWindow, 355 +
            //         pStartupScreens_->xMenuOffset(), 56 + pStartupScreens_->yMenuOffset() );
            // AniSmacker* pSmackerAnimation = new AniSmacker( briefPicture, 355 + pStartupScreens_->xMenuOffset(), 56
            // + pStartupScreens_->yMenuOffset() );
            const auto& topLeft = getBackdropTopLeft();
            AniSmacker* pSmackerAnimation
                = new AniSmackerRegular(briefPicture, 355 + topLeft.second, 56 + topLeft.first);
            pStartupScreens_->addSmackerAnimation(pSmackerAnimation);
        }
        else if (briefPicture.extension() == "bmp")
        {
            // File is a bitmap
            pBriefImage_ = new GuiImage(pStartupScreens_, Gui::Coord(355, 56), Gui::bitmap(briefPicture));
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
            string voicemail = textData.objectiveVoicemail(0);
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
