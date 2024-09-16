/*
 * C T X O P T N S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxoptns.hpp"
#include "stdlib/string.hpp"
#include "ctl/list.hpp"
#include "system/pathname.hpp"
#include "system/registry.hpp"
#include "system/memcaps.hpp"
#include "sound/soundmix.hpp"

#include "device/cd.hpp"

#include "world4d/soundman.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "render/capable.hpp"
#include "render/drivsel.hpp"
#include "gui/restring.hpp"
#include "gui/font.hpp"
#include "gui/image.hpp"
#include "machgui/ddrawdrop.hpp"
#include "machphys/compmgr.hpp"
#include "machphys/compitem.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/msgbox.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/ui/SlideBar.hpp"
#include "machgui/optlayout.hpp"
#include "machgui/dropdwnc.hpp"
#include "machgui/ui/CheckBox.hpp"
#include "machgui/internal/mgsndman.hpp"
#include "machgui/internal/strings.hpp"
#include "machlog/races.hpp"
#include "machlog/actor.hpp"

#define OPTIONS_AREA_MINX 95
#define OPTIONS_AREA_MINY 50
#define OPTIMISATIONS_AREA_MINX OPTIONS_AREA_MINX
#define OPTIMISATIONS_AREA_MINY 239

const char c_ScaleFactorOptionKey[] = "Options\\Scale Factor";
const char c_GrabCursorOptionKey[] = "Options\\Grab Cursor";

class MachGuiCtxOptions;

class MachGuiDDrawDropDownCallback : public MachGuiDropDownCallback
{
public:
    MachGuiDDrawDropDownCallback(MachGuiCtxOptions& optionsScreen)
        : optionsScreen_(optionsScreen)
    {
    }
    void callBack() override { optionsScreen_.showDirect3DDrivers(); }

private:
    MachGuiCtxOptions& optionsScreen_;
};

class MachGuiOptionsExitMessageBoxResponder : public MachGuiMessageBoxResponder
{
public:
    MachGuiOptionsExitMessageBoxResponder(MachGuiCtxOptions* pOptionsCtx)
        : pOptionsCtx_(pOptionsCtx)
    {
    }

    bool okPressed() override
    {
        pOptionsCtx_->exitFromOptions();
        return true;
    }

private:
    MachGuiCtxOptions* pOptionsCtx_;
};

MachGuiCtxOptions::MachGuiCtxOptions(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sf", pStartupScreens)
{
    // Get the display, required for things like gamma correction, display drivers etc
    RenDisplay* pDisplay_ = W4dManager::instance().sceneManager()->pDevice()->display();

    // Load the layout information
    MachGuiOptionsLayout screenLayout(SysPathName("gui/layout/options.dat"), MachGui::menuScaleFactor());

    // Create buttons
    {
        const MachGuiOptionsLayout::MenuButtonInfo& okButtonInfo = screenLayout.menuButtonInfo(0);
        const MachGuiOptionsLayout::MenuButtonInfo& cancelButtonInfo = screenLayout.menuButtonInfo(1);

        MachGuiMenuButton* pOkBtn = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(okButtonInfo.topLeft, okButtonInfo.bottomRight),
            IDS_MENUBTN_OK,
            MachGui::ButtonEvent::DUMMY_OK);
        MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(cancelButtonInfo.topLeft, cancelButtonInfo.bottomRight),
            IDS_MENUBTN_CANCEL,
            MachGui::ButtonEvent::DUMMY_EXIT);
        pCancelBtn->escapeControl(true);
        pOkBtn->defaultControl(true);
    }

    // Display large headings
    GuiBmpFont font = GuiBmpFont::getFont(SysPathName(MachGui::Menu::largeFontLight()));
    GuiBmpFont smallFont = GuiBmpFont::getFont(MachGui::Menu::smallFontLight());
    GuiResourceString optionsHeading(IDS_MENULB_OPTIONS);
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(OPTIONS_AREA_MINX, OPTIONS_AREA_MINY) * MachGui::menuScaleFactor(),
            font.textWidth(optionsHeading.asString()),
            font.charHeight() + 2),
        IDS_MENULB_OPTIONS,
        MachGui::Menu::largeFontLight());

    GuiResourceString optimisationsHeading(IDS_MENULB_OPTIMISATIONS);
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(OPTIMISATIONS_AREA_MINX, OPTIMISATIONS_AREA_MINY) * MachGui::menuScaleFactor(),
            font.textWidth(optimisationsHeading.asString()),
            font.charHeight() + 2),
        IDS_MENULB_OPTIMISATIONS,
        MachGui::Menu::largeFontLight());
    // Initialise gui items

    const MachGuiOptionsLayout::SlidebarInfo& musicVolSl = screenLayout.slidebarInfo(0);
    const MachGuiOptionsLayout::SlidebarInfo& soundVolSl = screenLayout.slidebarInfo(1);
    const MachGuiOptionsLayout::SlidebarInfo& gammaCorrectionSl = screenLayout.slidebarInfo(2);
    const MachGuiOptionsLayout::MenuTextInfo& musicVolTxt = screenLayout.menuTextInfo(0);
    const MachGuiOptionsLayout::MenuTextInfo& soundVolTxt = screenLayout.menuTextInfo(1);
    const MachGuiOptionsLayout::MenuTextInfo& gammaCorrectionTxt = screenLayout.menuTextInfo(2);
    const MachGuiOptionsLayout::MenuTextInfo& screenSizeTxt = screenLayout.menuTextInfo(3);
    const MachGuiOptionsLayout::MenuTextInfo& directDrawTxt = screenLayout.menuTextInfo(4);
    const MachGuiOptionsLayout::MenuTextInfo& direct3DTxt = screenLayout.menuTextInfo(5);
    const MachGuiOptionsLayout::MenuTextInfo& scaleFactorTxt = screenLayout.menuTextInfo(5);
    const MachGuiOptionsLayout::CheckBoxInfo& sound3dCB = screenLayout.checkBoxInfo(0);
    const MachGuiOptionsLayout::CheckBoxInfo& transitionsCB = screenLayout.checkBoxInfo(1);
    const MachGuiOptionsLayout::CheckBoxInfo& screenResolutionLock = screenLayout.checkBoxInfo(2);
    const MachGuiOptionsLayout::CheckBoxInfo& cursorType = screenLayout.checkBoxInfo(3);
    const MachGuiOptionsLayout::CheckBoxInfo& reverseKeys = screenLayout.checkBoxInfo(4);
    const MachGuiOptionsLayout::CheckBoxInfo& reverseMouse = screenLayout.checkBoxInfo(5);
    const MachGuiOptionsLayout::CheckBoxInfo& grabMouse = screenLayout.checkBoxInfo(6);

    // Create control labels
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(musicVolTxt.topLeft, musicVolTxt.bottomRight),
        musicVolTxt.idsStringId,
        MachGui::getScaledImagePath(musicVolTxt.font),
        Gui::AlignRight);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(soundVolTxt.topLeft, soundVolTxt.bottomRight),
        soundVolTxt.idsStringId,
        MachGui::getScaledImagePath(soundVolTxt.font),
        Gui::AlignRight);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(screenSizeTxt.topLeft, screenSizeTxt.bottomRight),
        screenSizeTxt.idsStringId,
        MachGui::getScaledImagePath(screenSizeTxt.font),
        Gui::AlignRight);

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(directDrawTxt.topLeft, directDrawTxt.bottomRight),
        directDrawTxt.idsStringId,
        MachGui::getScaledImagePath(directDrawTxt.font),
        Gui::AlignRight);

    //  new MachGuiMenuText( pStartupScreens, Gui::Box( direct3DTxt.topLeft, direct3DTxt.bottomRight ),
    //                         direct3DTxt.idsStringId, direct3DTxt.font, Gui::AlignRight );

    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(scaleFactorTxt.topLeft, scaleFactorTxt.bottomRight),
        scaleFactorTxt.idsStringId,
        MachGui::getScaledImagePath(scaleFactorTxt.font),
        Gui::AlignRight);

    // Create check boxes
    pSound3d_ = new MachGuiCheckBox(pStartupScreens, pStartupScreens, sound3dCB.box(), sound3dCB.stringId);

    pTransitions_
        = new MachGuiCheckBox(pStartupScreens, pStartupScreens, transitionsCB.box(), transitionsCB.stringId);

    pScreenResolutionLock_ = new 
        MachGuiCheckBox(pStartupScreens, pStartupScreens, screenResolutionLock.box(), screenResolutionLock.stringId);

    pCursorType_ = new MachGuiCheckBox(pStartupScreens, pStartupScreens, cursorType.box(), cursorType.stringId);

    pReverseKeys_ = new MachGuiCheckBox(pStartupScreens, pStartupScreens, reverseKeys.box(), reverseKeys.stringId);

    pReverseMouse_
        = new MachGuiCheckBox(pStartupScreens, pStartupScreens, reverseMouse.box(), reverseMouse.stringId);

    pGrabMouse_
        = new MachGuiCheckBox(pStartupScreens, pStartupScreens, grabMouse.box(), grabMouse.stringId);

    // Create volume sliders
    pMusicVolume_ = new MachGuiSlideBar(pStartupScreens, pStartupScreens, musicVolSl.topLeft, musicVolSl.range);
    pMusicVolume_->minMax(0, 100);
    pMusicVolume_->setValueChangedHandler([](float newValue) {
        DevCD::instance().volume(newValue + 0.5 /*stop rounding errors from slowly reducing volume*/);
    });

    pSoundVolume_ = new MachGuiSlideBar(pStartupScreens, pStartupScreens, soundVolSl.topLeft, soundVolSl.range);
    pSoundVolume_->minMax(0, 100);
    pSoundVolume_->setValueChangedHandler([](float newValue) {
        SndMixer::instance().masterSampleVolume(newValue + 0.5 /*stop rounding errors from slowly reducing volume*/);
    });

    const RenCapabilities& caps = W4dManager::instance().sceneManager()->pDevice()->capabilities();

    // Get current resolution
    int inGameResolutionWidth = pDisplay_->currentMode().width();
    int inGameResolutionHeight = pDisplay_->currentMode().height();
    int inGameResolutionRefresh = pDisplay_->currentMode().refreshRate();

    // Check that minimum resolution is specified
    if (inGameResolutionWidth < 640 || inGameResolutionHeight < 480)
    {
        inGameResolutionWidth = 640;
        inGameResolutionHeight = 480;
    }

    GuiStrings strings;
    MachGuiDropDownListBoxCreator::DropDownListBoxItems modeList;
    strings.reserve(4);
    modeList.reserve(4);

    // Iterate through screen modes selecting compatible ones ( i.e. conform to minimum size and colour bit depth ).
    const RenDisplay::Modes& modes = pDisplay_->modeList();
    const RenDisplay::Mode& lowestMode = pDisplay_->lowestAllowedMode();
    const RenDisplay::Mode& highestMode = pDisplay_->highestAllowedMode();
    for (const RenDisplay::Mode& mode : modes)
    {
        // I'd like to use the highest allowed mode to decide whether a mode is accepted or
        // not (instead of testing the memory required versus the available video memory)
        // unfortunately I can't since the modes are sorted by number of pixels and not by memory
        // requirement.
        // if ( mode >= lowestMode and mode.bitDepth() == 16 and
        if (mode >= lowestMode
            && caps.maxAvailableDisplayMemoryAfterTextures() >= 3 * mode.memoryRequired())
        {
            // Construct a string to go in the drop down list box ( e.g. "640x480" )
            char buffer[30];
            //          string resolutionStr = itoa( mode.width(), buffer, 10 );
            //          resolutionStr += itoa( mode.height(), buffer, 10 );
            sprintf(buffer, "%dx%d %d hz", mode.width(), mode.height(), mode.refreshRate());
            std::string resolutionStr = buffer;

            // If this mode is the current mode for ingame then make sure it appears first in the
            // drop down list box
            if (inGameResolutionWidth == mode.width() && inGameResolutionHeight == mode.height()
                && (inGameResolutionRefresh == 0 || inGameResolutionRefresh == mode.refreshRate()))
            {
                strings.insert(strings.begin(), resolutionStr);
                modeList.insert(modeList.begin(), (void*)&mode);
            }
            else
            {
                strings.push_back(resolutionStr);
                modeList.push_back((void*)&mode);
            }
        }
    }

    pScreenSize_ = new MachGuiDropDownListBoxCreator(
        pStartupScreens,
        pStartupScreens,
        Gui::Coord(353, 119) * MachGui::menuScaleFactor(),
        153 * MachGui::menuScaleFactor(),
        strings,
        false,
        true);
    pScreenSize_->items(modeList);

    // Create list of avaliable direct draw drivers/direct 3d drivers

    pDriverSelector_ = new RenDriverSelector(pDisplay_);
    MachGuiDropDownListBoxCreator::DropDownListBoxItems dDrawDrivers;
    GuiStrings dDrawDriverNames;
    dDrawDriverNames.reserve(4);
    dDrawDrivers.reserve(4);

    const RenDriverSelector::RenDrivers& dDrawDriversList = pDriverSelector_->dDrawDrivers();

    for (RenDriverSelector::RenDrivers::const_iterator i = dDrawDriversList.begin(); i != dDrawDriversList.end(); ++i)
    {
        dDrawDrivers.push_back((MachGuiDropDownListBoxCreator::DropDownListBoxItem) & (*i));
        dDrawDriverNames.push_back((*i)->name());
    }
    dDrawDriverNames.push_back("default");

    pDirectDrawDrivers_ = new MachGuiDDrawDropDownListBoxCreator(
        pStartupScreens,
        pStartupScreens,
        Gui::Coord(353, 139) * MachGui::menuScaleFactor(),
        153 * MachGui::menuScaleFactor(),
        dDrawDriverNames,
        false,
        true,
        new MachGuiDDrawDropDownCallback(*this));

    pDirectDrawDrivers_->items(dDrawDrivers);

    //  pDirectDrawDrivers_->text( pDriverSelector_->currentDDrawDriver()->name() );

    //  showDirect3DDrivers();

    // Only display gamma correction slider if gamma correction is supported
    if (pDisplay_->supportsGammaCorrection())
    {
        new MachGuiMenuText(
            pStartupScreens,
            Gui::Box(gammaCorrectionTxt.topLeft, gammaCorrectionTxt.bottomRight),
            gammaCorrectionTxt.idsStringId,
            MachGui::getScaledImagePath(gammaCorrectionTxt.font),
            Gui::AlignRight);

        pGammaCorrection_ = new MachGuiSlideBar(
            pStartupScreens,
            pStartupScreens,
            gammaCorrectionSl.topLeft,
            gammaCorrectionSl.range,
            GAMMA_LOWER_LIMIT,
            GAMMA_UPPER_LIMIT);
        // Store initial value

        pGammaCorrection_->setValueChangedHandler([](float newValue) {
            W4dManager::instance().sceneManager()->pDevice()->display()->gammaCorrection(newValue);
        });

        gammaCorrection_ = W4dManager::instance().sceneManager()->pDevice()->display()->gammaCorrection();
        pGammaCorrection_->setValue(gammaCorrection_);

        // Show gamma correction image (helps get gamma setting correct)
        new GuiImage(pStartupScreens, Gui::Coord(353, 198) * MachGui::menuScaleFactor(), MachGui::getScaledImage("gui/menu/gammacal.bmp"));
    }

    {
        const MachPhysComplexityManager::BooleanItems& boolItems = MachPhysComplexityManager::instance().booleanItems();
        // Access boolean items
        uint index = 0;
        const auto optimizationAreaCoord = Gui::Coord(95, OPTIMISATIONS_AREA_MINY + 33) * MachGui::menuScaleFactor();
        const int verticalStep = 20 * MachGui::menuScaleFactor();
        const int boxWidth = 157 * MachGui::menuScaleFactor();
        for (const MachPhysComplexityBooleanItem *item : boolItems)
        {
            booleanOptimisations_.push_back(new MachGuiCheckBox(
                pStartupScreens,
                pStartupScreens,
                Gui::Box(optimizationAreaCoord + Gui::Coord(0, verticalStep * index), MexSize2d(boxWidth, MachGuiCheckBox::implicitHeight())),
                item->id()));
            ++index;
        }
    }

    {
        // Access all the choices items, their id and # of choice per id
        const MachPhysComplexityManager::ChoiceItems& chItems = MachPhysComplexityManager::instance().choiceItems();
        uint index = 0;
        const auto optimizationAreaCoord = Gui::Coord(347, OPTIMISATIONS_AREA_MINY + 33) * MachGui::menuScaleFactor();
        const auto choicesBaseCoord = Gui::Coord(353, OPTIMISATIONS_AREA_MINY + 35) * MachGui::menuScaleFactor();
        const int verticalStep = 20 * MachGui::menuScaleFactor();
        for (MachPhysComplexityManager::ChoiceItems::const_iterator it = chItems.begin(); it != chItems.end(); ++it)
        {
            uint id = (*it)->id();
            uint nch = (*it)->nChoices();

            GuiStrings choices;
            MachGuiDropDownListBoxCreator::DropDownListBoxItems choiceIds;
            choices.reserve(nch);
            choiceIds.reserve(nch);

            for (uint ch = 0; ch < nch; ++ch)
            {
                GuiResourceString choice(ch + id + 1);
                std::string choiceString = choice.asString();
                choices.push_back(choiceString);
                choiceIds.push_back((MachGuiDropDownListBoxCreator::DropDownListBoxItem)(ch + 1));
            }
            GuiResourceString choiceTitle(id);

            new MachGuiMenuText(
                pStartupScreens,
                Gui::Box(
                    optimizationAreaCoord
                        + Gui::Coord(-smallFont.textWidth(choiceTitle.asString()), verticalStep * index),
                    smallFont.textWidth(choiceTitle.asString()),
                    smallFont.charHeight() + 8 * MachGui::menuScaleFactor()),
                id,
                MachGui::Menu::smallFontLight());

            choicesOptimisations_.push_back(new MachGuiDropDownListBoxCreator(
                pStartupScreens,
                pStartupScreens,
                choicesBaseCoord + Gui::Coord(0, verticalStep * index),
                153 * MachGui::menuScaleFactor(),
                choices,
                false,
                true));
            (*(choicesOptimisations_.end() - 1))->items(choiceIds);
            ++index;
        }
    }

    // Retain original settings in case user cancels subsequent settings
    musicVolume_ = pMusicVolume_->value();
    soundVolume_ = pSoundVolume_->value();

    initialDDrawDriver_ = pDriverSelector_->currentDDrawDriver();
    GuiResourceString defaultStr(IDS_MENU_DEFAULT);

    GuiStrings scaleNames = {
        defaultStr.asString(),
        "100%",
        "200%",
    };

    static const int scaleValues[] = {
        0,
        100,
        200,
    };

    pScaleFactorSelector_ = new MachGuiDropDownListBoxCreator(
        pStartupScreens,
        pStartupScreens,
        Gui::Coord(353, 159) * MachGui::menuScaleFactor(),
        153 * MachGui::menuScaleFactor(),
        scaleNames,
        false,
        true);
    auto items = MachGuiDropDownListBoxCreator::createBoxItems(scaleValues);
    pScaleFactorSelector_->items(items);

    readFromConfig();

    pGrabMouse_->setCallback([](MachGuiCheckBox* pCheckBox) {
        RenDisplay* pDisplay_ = W4dManager::instance().sceneManager()->pDevice()->display();
        pDisplay_->setCursorGrabEnabled(pCheckBox->isChecked());
    });

    TEST_INVARIANT;
}

MachGuiCtxOptions::~MachGuiCtxOptions()
{
    TEST_INVARIANT;
}

void MachGuiCtxOptions::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxOptions& t)
{

    o << "MachGuiCtxOptions " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxOptions " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCtxOptions::update()
{
    if (exitFromOptions_)
    {
        pStartupScreens_->buttonAction(MachGui::ButtonEvent::OK);
    }
    else
    {
        animations_.update();
    }
}

// virtual
bool MachGuiCtxOptions::okayToSwitchContext()
{
    return true;
}

// virtual
void MachGuiCtxOptions::buttonEvent(MachGui::ButtonEvent buttonEvent)
{
    if (buttonEvent == MachGui::ButtonEvent::DUMMY_OK)
    {
        int currentScaleFactorValue = SysRegistry::instance().queryIntegerValue(c_ScaleFactorOptionKey, "Value");

        writeToConfig();

        SysRegistry::instance().reload();

        uint idsMessage;

        bool bDisplayMessageBox = false;

        // Determine if the video driver settings have changed
        /*if( ( pDriverSelector_->currentDDrawDriver()->name() != initialDDrawDriver_->name() ) or
            ( pDriverSelector_->currentDDrawDriver()->description() != initialDDrawDriver_->description() ) )
        {
            idsMessage = IDS_MENUMSG_DISPLAYDRIVER;
            bDisplayMessageBox = true;
        }*/

        // Determine if the 2D selection boxes are enabled and screen resolution has changed
        const RenDisplay::Mode* pNewMode = (const RenDisplay::Mode*)pScreenSize_->item();
        const RenDisplay::Mode& pCurrentMode
            = W4dManager::instance().sceneManager()->pDevice()->display()->currentMode();

        int newScaleFactorValue = SysRegistry::instance().queryIntegerValue(c_ScaleFactorOptionKey, "Value");

        if (pScreenResolutionLock_->isChecked()
            && ((pNewMode->width() != pCurrentMode.width()) || (pNewMode->height() != pCurrentMode.height())))
        {
            // If we already have a message to display from the previous settings change, then use a combined message
            if (bDisplayMessageBox)
            {
                idsMessage = IDS_MENUMESSAGE_DRIVER_RESOLUTION;
            }
            else
            {
                idsMessage = IDS_MENUMESSAGE_RESOLUTION;
            }
            bDisplayMessageBox = true;
        }

        if (!bDisplayMessageBox && (currentScaleFactorValue != newScaleFactorValue))
        {
            idsMessage = IDS_MENUMESSAGE_SCALE_FACTOR;
            bDisplayMessageBox = true;
        }

        if (bDisplayMessageBox)
        {
            // Inform user that changes will not take effect until the machine is rebooted
            pStartupScreens_->displayOKMsgBox(idsMessage, new MachGuiOptionsExitMessageBoxResponder(this));
        }
        else
        {
            pStartupScreens_->buttonAction(MachGui::ButtonEvent::OK);
        }
    }
    else if (buttonEvent == MachGui::ButtonEvent::DUMMY_EXIT)
    {
        pMusicVolume_->setValue(musicVolume_);
        pSoundVolume_->setValue(soundVolume_);
        pDriverSelector_->useDDrawDriver(initialDDrawDriver_);
        // Only restore gamma correction if gamma correction is supported
        if (pGammaCorrection_)
        {
            pGammaCorrection_->setValue(gammaCorrection_);
        }
        pStartupScreens_->buttonAction(MachGui::ButtonEvent::EXIT);

        const bool grabCursorEnabled = SysRegistry::instance().queryBooleanValue(c_GrabCursorOptionKey, "on", true);
        pGrabMouse_->setChecked(grabCursorEnabled);
    }
}

void MachGuiCtxOptions::writeToConfig()
{
    // Used to set current game settings to be that of gui items

    if (pSound3d_->isChecked() != SysRegistry::instance().queryIntegerValue("Options\\3DSound", "on"))
    {
        SOUND_STREAM("Updating sounds" << std::endl);
        //      //It is necessary to clear all sounds from the system
        //      //before loading a new set.
        MachGuiSoundManager::instance().clearAll();
        W4dSoundManager::instance().stopAll();
        load3dSoundFiles(pSound3d_->isChecked());
        // We now have to change all the in game sounds to the correct
        // number of dimensions
        if (pSound3d_->isChecked())
        {
            W4dSoundManager::instance().convertSoundDimensions(Snd::THREE_D);
        }
        else
        {
            W4dSoundManager::instance().convertSoundDimensions(Snd::TWO_D);
        }
        SysRegistry::instance().setIntegerValue("Options\\3DSound", "on", pSound3d_->isChecked());
    }

    pStartupScreens_->startupData()->transitionFlicsOn(pTransitions_->isChecked());
    SysRegistry::instance().setIntegerValue(
        "Options\\transitions",
        "on",
        pStartupScreens_->startupData()->transitionFlicsOn());

    // Store the new screen size in the registry
    const RenDisplay::Mode* pNewMode = (const RenDisplay::Mode*)pScreenSize_->item();
    SysRegistry::instance().setIntegerValue("Screen Resolution", "Width", pNewMode->width());
    SysRegistry::instance().setIntegerValue("Screen Resolution", "Height", pNewMode->height());
    SysRegistry::instance().setIntegerValue("Screen Resolution", "Refresh Rate", pNewMode->refreshRate());

    // Store sound and CD settings
    SysRegistry::instance().setIntegerValue(
        "Options\\CD",
        "Volume",
        pMusicVolume_->value() + 0.5 /*stop rounding errors*/);
    SysRegistry::instance().setIntegerValue(
        "Options\\Sound",
        "Volume",
        pSoundVolume_->value() + 0.5 /*stop rounding errors*/);

    // Store gamma correction value
    if (pGammaCorrection_)
    {
        SysRegistry::instance().setIntegerValue(
            "Options\\Gamma Correction",
            "Value",
            static_cast<double>(pGammaCorrection_->value()) * GAMMA_REG_MULTIPLIER);
    }

    // Store option to maintain screen res of in-game menus in menus
    SysRegistry::instance().setIntegerValue("Screen Resolution", "Lock Resolution", pScreenResolutionLock_->isChecked());

    // Store cursor type (2D/3D)
    SysRegistry::instance().setIntegerValue("Options\\Cursor Type", "2D", pCursorType_->isChecked());

    // If cursor type has changed then refresh all selection boxes
    if (cursorType2d_ != pCursorType_->isChecked())
    {
        MachLogRaces::Objects& allObjects = MachLogRaces::instance().objects();

        for (MachLogRaces::Objects::iterator iter = allObjects.begin(); iter != allObjects.end(); ++iter)
        {
            MachActor* pActor = *iter;
            if (pActor->selectionState() == MachLog::SELECTED)
            {
                // Deselect, then reselect to refresh bounding box
                pActor->selectionState(MachLog::NOT_SELECTED);
                pActor->selectionState(MachLog::SELECTED);
            }
            else if (pActor->selectionState() == MachLog::HIGHLIGHTED)
            {
                // Deselect, then reselect to refresh bounding box
                pActor->selectionState(MachLog::NOT_SELECTED);
                pActor->selectionState(MachLog::HIGHLIGHTED);
            }
        }
    }

    // Store reverse direction of up/down keys/mouse
    SysRegistry::instance().setIntegerValue("Options\\Reverse UpDown Keys", "on", pReverseKeys_->isChecked());
    SysRegistry::instance().setIntegerValue("Options\\Reverse BackForward Mouse", "on", pReverseMouse_->isChecked());
    SysRegistry::instance().setIntegerValue(c_GrabCursorOptionKey, "on", pGrabMouse_->isChecked());

    // Access all the boolean optimisations
    const MachPhysComplexityManager::BooleanItems& boolItems = MachPhysComplexityManager::instance().booleanItems();
    uint index = 0;
    for (MachPhysComplexityManager::BooleanItems::const_iterator it = boolItems.begin(); it != boolItems.end(); ++it)
    {
        uint id = (*it)->id();
        MachPhysComplexityManager::instance().changeBooleanItem(id, booleanOptimisations_[index]->isChecked());
        ++index;
    }
    index = 0;

    // Access all the choices items, their id and # of choice per id
    const MachPhysComplexityManager::ChoiceItems& chItems = MachPhysComplexityManager::instance().choiceItems();
    for (MachPhysComplexityManager::ChoiceItems::const_iterator it = chItems.begin(); it != chItems.end(); ++it)
    {
        uint id = (*it)->id();
        // TODO: void* to uint??, this ptr value is stored simply as number?
        // MachPhysComplexityManager::instance().changeChoiceItem( id, ( uint ) choicesOptimisations_[index]->item() - 1
        // );
        MachPhysComplexityManager::instance().changeChoiceItem(id, (size_t)choicesOptimisations_[index]->item() - 1);
        ++index;
    }

    // Set Direct3DDriver to be the one selected in the list of available Direct3DDrivers
    // CtlCountedPtr<RenDriver> const & newDriver = ( CtlCountedPtr<RenDriver> const & ) *(pDirect3DDrivers_->item());
    //  CtlCountedPtr<RenDriver> const & newDriver =
    //        ( CtlCountedPtr<RenDriver> const & ) *((CtlCountedPtr<RenDriver>*)pDirect3DDrivers_->item());

    //  pDriverSelector_->useD3dDriver( newDriver );

    // Save display driver info in registry
    //   pDriverSelector_->updateDriverRegistries();

    {
        MachGuiDropDownListBoxCreator::DropDownListBoxItem currentItem = pScaleFactorSelector_->item();
        int ScaleValue = *static_cast<const int*>(currentItem);
        SysRegistry::instance().setIntegerValue(c_ScaleFactorOptionKey, "Value", ScaleValue);
    }
}

void MachGuiCtxOptions::readFromConfig()
{
    // Used to set gui items to reflect current game settings
    pSound3d_->setChecked(
        SysRegistry::instance().queryBooleanValue("Options\\3DSound", "on", SndMixer::instance().is3dMixer()));

    musicVolume_ = DevCD::instance().volume();
    soundVolume_ = SndMixer::instance().masterSampleVolume();

    pMusicVolume_->setValue(musicVolume_);
    pSoundVolume_->setValue(soundVolume_);

    // Set resolution lock on if it the first time the game is being run
    pScreenResolutionLock_->setChecked(SysRegistry::instance().queryBooleanValue(
        "Screen Resolution",
        "Lock Resolution",
        MachGuiStartupScreens::getDefaultLockScreenResolutionValue()));
    pCursorType_->setChecked(SysRegistry::instance().queryIntegerValue("Options\\Cursor Type", "2D"));
    pReverseKeys_->setChecked(SysRegistry::instance().queryIntegerValue("Options\\Reverse UpDown Keys", "on"));
    pReverseMouse_->setChecked(SysRegistry::instance().queryIntegerValue("Options\\Reverse BackForward Mouse", "on"));

    const bool grabCursorEnabled = SysRegistry::instance().queryBooleanValue(c_GrabCursorOptionKey, "on", true);
    pGrabMouse_->setChecked(grabCursorEnabled);

    pTransitions_->setChecked(pStartupScreens_->startupData()->transitionFlicsOn());

    // Access all the boolean optimisations
    const MachPhysComplexityManager::BooleanItems& boolItems = MachPhysComplexityManager::instance().booleanItems();
    uint index = 0;
    for (MachPhysComplexityManager::BooleanItems::const_iterator it = boolItems.begin(); it != boolItems.end(); ++it)
    {
        uint id = (*it)->id();
        booleanOptimisations_[index]->setChecked((*it)->enabled());
        ++index;
    }

    index = 0;
    // Access all the choices items, their id and # of choice per id
    const MachPhysComplexityManager::ChoiceItems& chItems = MachPhysComplexityManager::instance().choiceItems();
    for (MachPhysComplexityManager::ChoiceItems::const_iterator it = chItems.begin(); it != chItems.end(); ++it)
    {
        uint id = (*it)->id();

        GuiResourceString choice(id + (*it)->choice() + 1);

        choicesOptimisations_[index]->text(choice.asString());

        ++index;
    }

    cursorType2d_ = pCursorType_->isChecked();

    {
        int scaleFactorValue = SysRegistry::instance().queryIntegerValue(c_ScaleFactorOptionKey, "Value");

        const MachGuiDropDownListBoxCreator::DropDownListBoxItems& scaleItems = pScaleFactorSelector_->items();
        std::size_t scaleItemIndex = 0;
        for (std::size_t i = 0; i < scaleItems.size(); ++i)
        {
            const int* scale = static_cast<const int*>(scaleItems.at(i));
            if (scaleFactorValue == *scale)
            {
                scaleItemIndex = i;
                break;
            }
        }
        pScaleFactorSelector_->setCurrentItem(scaleItems.at(scaleItemIndex));
    }
}

// static
void MachGuiCtxOptions::load3dSoundFiles(bool enabled)
{
    // Load the sound definition file depending on 3d sound enabled and the bit rate currently being used

    unsigned int totalMem = SysMemoryCaps::totalPhysicalMem();

    // World4d needs to know what sounds we are operating with.
    enabled ? W4dSoundManager::instance().currentSoundDimensions(Snd::THREE_D)
            : W4dSoundManager::instance().currentSoundDimensions(Snd::TWO_D);

    if (enabled)
    {
        if (totalMem > 64000000)
            W4dSoundManager::instance().readSoundDefinitionFile("sounds/snddef64.dat");
        else if (totalMem > 32000000)
            W4dSoundManager::instance().readSoundDefinitionFile("sounds/snddef32.dat");
        else
            W4dSoundManager::instance().readSoundDefinitionFile("sounds/snddef16.dat");
    }
    else
    {
        if (totalMem > 64000000)
            W4dSoundManager::instance().readSoundDefinitionFile("sounds/sd2d64.dat");
        else if (totalMem > 32000000)
            W4dSoundManager::instance().readSoundDefinitionFile("sounds/sd2d32.dat");
        else
            W4dSoundManager::instance().readSoundDefinitionFile("sounds/sd2d16.dat");
    }
}

void MachGuiCtxOptions::showDirect3DDrivers()
{
    if (!pDirectDrawDrivers_->hasItems())
        return;

    // CtlCountedPtr<RenDriver> const & newDriver = ( CtlCountedPtr<RenDriver> const & ) *(pDirectDrawDrivers_->item());
    CtlCountedPtr<RenDriver> const& newDriver
        = (CtlCountedPtr<RenDriver> const&)*((CtlCountedPtr<RenDriver>*)pDirectDrawDrivers_->item());
    pDriverSelector_->useDDrawDriver(newDriver);

    delete pDirect3DDrivers_;

    const RenDriverSelector::RenDrivers& d3dDriversList = pDriverSelector_->d3dDrivers();
    MachGuiDropDownListBoxCreator::DropDownListBoxItems d3dDrivers;
    GuiStrings d3dDriverNames;
    d3dDriverNames.reserve(4);
    d3dDrivers.reserve(4);

    for (RenDriverSelector::RenDrivers::const_iterator i = d3dDriversList.begin(); i != d3dDriversList.end(); ++i)
    {
        d3dDrivers.push_back((MachGuiDropDownListBoxCreator::DropDownListBoxItem) & (*i));
        d3dDriverNames.push_back((*i)->name());
    }
    pDirect3DDrivers_ = new MachGuiDropDownListBoxCreator(
        pStartupScreens_,
        pStartupScreens_,
        Gui::Coord(353, 159),
        153,
        d3dDriverNames,
        false,
        true);
    pDirect3DDrivers_->items(d3dDrivers);

    pDirect3DDrivers_->text(pDriverSelector_->currentD3dDriver()->name());
}

void MachGuiCtxOptions::exitFromOptions()
{
    exitFromOptions_ = true;
}

/* End CTXOPTNS.CPP *************************************************/
