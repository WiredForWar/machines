/*
 * C T X O P T N S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/MenuContext/CtxOptions.hpp"
#include "ctl/List.hpp"
#include "system/ConfigVariables.hpp"
#include "system/PathName.hpp"
#include "system/Registry.hpp"
#include "system/MemoryCaps.hpp"
#include "sound/Mixer.hpp"

#include "world4d/Sound/SoundManager.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "render/Font.hpp"
#include "render/Device.hpp"
#include "render/Display.hpp"
#include "render/Capabilities.hpp"
#include "gui/ResourceString.hpp"
#include "gui/Font.hpp"
#include "gui/Image.hpp"
#include "machphys/ComplexityManager.hpp"
#include "machphys/ComplexityItem.hpp"
#include "machgui/VSyncMode.hpp"
#include "machgui/InputLayout.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/MessageBoxResponder.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/StartupData.hpp"
#include "machgui/ui/SlideBar.hpp"
#include "machgui/DropDownListBoxCreator.hpp"
#include "machgui/ui/CheckBox.hpp"
#include "machgui/internal/SoundManager.hpp"
#include "machgui/internal/strings.hpp"
#include "machlog/Races.hpp"
#include "machlog/Actors/Actor.hpp"

#define OPTIONS_AREA_MINX 95
#define OPTIONS_AREA_MINY 50
#define OPTIMISATIONS_AREA_MINX OPTIONS_AREA_MINX
#define OPTIMISATIONS_AREA_MINY 239

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

    // Create buttons
    {
        const uint menuScaleFactor = MachGui::menuScaleFactor();
        const Gui::Coord okTopLeft = Gui::Coord(85, 377) * menuScaleFactor;
        const Gui::Size buttonSize = Gui::Size(206, 46) * menuScaleFactor;
        const Gui::Coord cancelTopLeft = Gui::Coord(337, 377) * menuScaleFactor;

        MachGuiMenuButton* pOkBtn = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(okTopLeft, buttonSize),
            IDS_MENUBTN_OK,
            MachGui::ButtonEvent::DUMMY_OK);
        MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(cancelTopLeft, buttonSize),
            IDS_MENUBTN_CANCEL,
            MachGui::ButtonEvent::DUMMY_EXIT);
        pCancelBtn->escapeControl(true);
        pOkBtn->defaultControl(true);
    }

    // Display large headings
    const Ren::Font& font = MachGui::Menu::font();
    GuiBmpFont smallFont = Gui::getFont(MachGui::Menu::smallFontLight());
    GuiResourceString optionsHeading(IDS_MENULB_OPTIONS);
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(OPTIONS_AREA_MINX, OPTIONS_AREA_MINY) * MachGui::menuScaleFactor(),
            Gui::Size(
                font.horizontalAdvance(optionsHeading.asString(), MachGui::Menu::menuLightTextOptions()),
                font.height() + 2 * MachGui::menuScaleFactor())),
        IDS_MENULB_OPTIONS,
        font,
        MachGui::Menu::menuLightTextOptions(),
        Gui::AlignLeft);

    GuiResourceString optimisationsHeading(IDS_MENULB_OPTIMISATIONS);
    new MachGuiMenuText(
        pStartupScreens,
        Gui::Box(
            Gui::Coord(OPTIMISATIONS_AREA_MINX, OPTIMISATIONS_AREA_MINY) * MachGui::menuScaleFactor(),
            Gui::Size(
                font.horizontalAdvance(optimisationsHeading.asString(), MachGui::Menu::menuLightTextOptions()),
                font.height() + 2 * MachGui::menuScaleFactor())),
        IDS_MENULB_OPTIMISATIONS,
        font,
        MachGui::Menu::menuLightTextOptions(),
        Gui::AlignLeft);
    // Initialise gui items

    const uint menuScaleFactor = MachGui::menuScaleFactor();
    const uint firstColumnX = 92 * menuScaleFactor;
    const uint contentY = 79 * menuScaleFactor;
    const uint firstColumnWidth = 157 * menuScaleFactor;
    const uint columnSpacing = 3 * menuScaleFactor;
    const uint secondColumnX = firstColumnX + firstColumnWidth + columnSpacing;
    const uint secondColumnTextWidth = 120 * menuScaleFactor;
    const uint secondColumnSpacing = 7 * menuScaleFactor;
    const uint secondColumnInputX = secondColumnX + secondColumnTextWidth + secondColumnSpacing;
    const uint secondColumnInputWidth = 153 * menuScaleFactor;
    const uint rowHeight = MachGuiCheckBox::implicitHeight();
    const uint verticalSpacing = 9 * menuScaleFactor;
    int itemsIn1stColumn{};
    int itemsIn2ndColumn{};

    auto addCheckBox = [&](const ResolvedUiString& label) -> MachGuiCheckBox*
    {
        const uint y = contentY + (rowHeight + verticalSpacing) * itemsIn1stColumn;
        auto* checkBox = new MachGuiCheckBox(
            pStartupScreens,
            pStartupScreens,
            Gui::Box(
                Gui::Coord(firstColumnX, y),
                Gui::Size(firstColumnWidth, rowHeight)),
            label);
        ++itemsIn1stColumn;
        return checkBox;
    };

    auto addSliderBar = [&](const ResolvedUiString& label) -> MachGuiSlideBar*
    {
        const uint y = contentY + (rowHeight + verticalSpacing) * itemsIn2ndColumn;

        new MachGuiMenuText(
            pStartupScreens,
            Gui::Box(
                Gui::Coord(secondColumnX, y),
                Gui::Size(secondColumnTextWidth, rowHeight)),
            label,
            MachGui::Menu::smallFontLight(),
            Gui::AlignRight);

        auto* slider = new MachGuiSlideBar(
            pStartupScreens,
            pStartupScreens,
            Gui::Coord(secondColumnInputX, y),
            secondColumnInputWidth);
        ++itemsIn2ndColumn;
        return slider;
    };

    auto addDropDown = [&](const ResolvedUiString& label) -> MachGuiDropDownListBoxCreator*
    {
        const uint y = contentY + (rowHeight + verticalSpacing) * itemsIn2ndColumn;

        new MachGuiMenuText(
            pStartupScreens,
            Gui::Box(
                Gui::Coord(secondColumnX, y),
                Gui::Size(secondColumnTextWidth, rowHeight)),
            label,
            MachGui::Menu::smallFontLight(),
            Gui::AlignRight);

        auto* dropDown = new MachGuiDropDownListBoxCreator(
            pStartupScreens,
            pStartupScreens,
            Gui::Coord(secondColumnInputX, y),
            secondColumnInputWidth,
            false,
            true);
        ++itemsIn2ndColumn;
        return dropDown;
    };

    // Create check boxes
    pSound3d_ = addCheckBox(IDS_MENU_3DSOUND);
    pTransitions_ = addCheckBox(IDS_MENU_SCREENTRANSITIONS);
    pCursorType_ = addCheckBox(IDS_MENU_CURSORTYPE);
    pReverseKeys_ = addCheckBox(IDS_MENU_REVERSEKEYS);
    pReverseMouse_ = addCheckBox(IDS_MENU_REVERSEMOUSE);
    pGrabMouse_ = addCheckBox(IDS_GRAB_MOUSE);
    pWasdControls_ = addCheckBox(IDS_USE_WASD);

    // Create volume sliders
    pMusicVolume_ = addSliderBar(IDS_MENU_MUSICVOL);
    pMusicVolume_->minMax(0, 100);
    pMusicVolume_->setValue(Config::musicVolume.get());
    pMusicVolume_->setValueChangedHandler([](float newValue) { Config::musicVolume.set(std::round(newValue)); });

    pSoundVolume_ = addSliderBar(IDS_MENU_SOUNDVOL);
    pSoundVolume_->minMax(0, 100);
    pSoundVolume_->setValue(Config::soundVolume.get());
    pSoundVolume_->setValueChangedHandler([](float newValue) { Config::soundVolume.set(std::round(newValue)); });

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
            && 256000000 >= 3 * mode.memoryRequired())
        {
            // Construct a string to go in the drop down list box ( e.g. "640x480" )
            char buffer[30];
            //          string resolutionStr = itoa( mode.width(), buffer, 10 );
            //          resolutionStr += itoa( mode.height(), buffer, 10 );
            snprintf(buffer, sizeof(buffer), "%dx%d %d hz", mode.width(), mode.height(), mode.refreshRate());
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

    pScreenSize_ = addDropDown(IDS_MENU_SCREENSIZE);
    pScreenSize_->setAvailText(strings);
    pScreenSize_->items(modeList);

    {
        GuiStrings itemNames = {
            ResolvedUiString(IDS_VSYNC_OPTION_AUTO),
            ResolvedUiString(IDS_VSYNC_OPTION_ENABLED),
            ResolvedUiString(IDS_VSYNC_OPTION_DISABLED),
        };

        vSyncModeDropDown_ = addDropDown(IDS_VSYNC_OPTION);
        vSyncModeDropDown_->setAvailText(itemNames);
    }

    {
        GuiStrings scaleNames = {
            ResolvedUiString(IDS_MENU_DEFAULT),
            "100%",
            "200%",
        };

        static const int scaleValues[] = {
            0,
            100,
            200,
        };

        pScaleFactorSelector_ = addDropDown(IDS_SCALE_FACTOR);
        pScaleFactorSelector_->setAvailText(scaleNames);
        auto items = MachGuiDropDownListBoxCreator::createBoxItems(scaleValues);
        pScaleFactorSelector_->items(items);
    }

    pCameraAccelerationSlider_ = addSliderBar(IDS_CONFIG_CAMERA_ACCELERATION);
    pCameraAccelerationSlider_->minMax(1, 30);
    pCameraAccelerationSlider_->setValue(Config::uiZenithCameraAcceleration.get());
    pCameraAccelerationSlider_->setValueChangedHandler([](float v)
    {
        Config::uiZenithCameraAcceleration.set(v);
        Config::uiGroundCameraAcceleration.set(v);
    });

    // Only display gamma correction slider if gamma correction is supported
    if (pDisplay_->supportsGammaCorrection())
    {
        pGammaCorrection_ = addSliderBar(IDS_MENU_GAMMACORRECTION);
        pGammaCorrection_->minMax(GAMMA_LOWER_LIMIT, GAMMA_UPPER_LIMIT);
        // Store initial value

        pGammaCorrection_->setValueChangedHandler([](float newValue) {
            W4dManager::instance().sceneManager()->pDevice()->display()->gammaCorrection(newValue);
        });

        gammaCorrection_ = W4dManager::instance().sceneManager()->pDevice()->display()->gammaCorrection();
        pGammaCorrection_->setValue(gammaCorrection_);

        // Show gamma correction image (helps get gamma setting correct)
        new GuiImage(
            pStartupScreens,
            Gui::Coord(secondColumnInputX, pGammaCorrection_->relativeBoundary().bottom() + verticalSpacing),
            MachGui::getScaledImage("gui/menu/gammacal.bmp"));
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
                Gui::Box(
                    optimizationAreaCoord + Gui::Coord(0, verticalStep * index),
                    MexSize2d(boxWidth, MachGuiCheckBox::implicitHeight())),
                item->id()));
            ++index;
        }
    }

    {
        // Access all the choices items, their id and # of choice per id
        const MachPhysComplexityManager::ChoiceItems& chItems = MachPhysComplexityManager::instance().choiceItems();
        uint index = 0;
        const auto optimizationAreaCoord
            = Gui::Coord(secondColumnX, (OPTIMISATIONS_AREA_MINY + 33) * MachGui::menuScaleFactor());
        const auto choicesBaseCoord
            = Gui::Coord(secondColumnInputX, (OPTIMISATIONS_AREA_MINY + 35) * MachGui::menuScaleFactor());
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
                // The item id is an opaque pointer-sized tag, so widen the
                // index before reinterpreting it as one.
                choiceIds.push_back(reinterpret_cast<MachGuiDropDownListBoxCreator::DropDownListBoxItem>(
                    static_cast<uintptr_t>(ch + 1)));
            }
            GuiResourceString choiceTitle(id);

            new MachGuiMenuText(
                pStartupScreens,
                Gui::Box(
                    optimizationAreaCoord + Gui::Vec(0, verticalSpacing * index),
                    Gui::Size(secondColumnTextWidth, smallFont.height() + 8 * MachGui::menuScaleFactor())),
                id,
                MachGui::Menu::smallFontLight(),
                Gui::AlignRight);

            auto* choiceDropDown = new MachGuiDropDownListBoxCreator(
                pStartupScreens,
                pStartupScreens,
                choicesBaseCoord + Gui::Coord(0, verticalSpacing * index),
                secondColumnInputWidth,
                false,
                true);
            choiceDropDown->setAvailText(choices);
            choiceDropDown->items(choiceIds);
            choicesOptimisations_.push_back(choiceDropDown);
            ++index;
        }
    }

    // Retain original settings in case user cancels subsequent settings
    musicVolume_ = Config::musicVolume.get();
    soundVolume_ = Config::soundVolume.get();
    use2DMarker_ = Config::uiUse2DSelectionMarker.get();
    grabCursor_ = Config::grabCursor.get();

    readFromConfig();

    pCursorType_->setCallback([](MachGuiCheckBox* pBox) { Config::uiUse2DSelectionMarker.set(pBox->isChecked()); });
    pGrabMouse_->setCallback([](MachGuiCheckBox* pCheckBox) { Config::grabCursor.set(pCheckBox->isChecked()); });

    pWasdControls_->setCallback([](MachGuiCheckBox* pCheckBox) {
        using InputLayout = MachGui::InputLayout;
        Config::inputBaseLayout.set(pCheckBox->isChecked() ? InputLayout::WASD : InputLayout::Legacy);
    });

    vSyncModeDropDown_->setCurrentIndexChangedCallback([this]() {
        MachGui::VSyncMode selectedMode = static_cast<MachGui::VSyncMode>(vSyncModeDropDown_->currentIndex());
        Config::gfxVSyncMode.set(selectedMode);
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
        int currentScaleFactorValue = Config::uiScaleFactor.get();

        writeToConfig();

        SysRegistry::instance().reload();

        uint idsMessage;

        bool bDisplayMessageBox = false;

        const RenDisplay::Mode* pNewMode = (const RenDisplay::Mode*)pScreenSize_->item();
        const RenDisplay::Mode& pCurrentMode
            = W4dManager::instance().sceneManager()->pDevice()->display()->currentMode();

        int newScaleFactorValue = Config::uiScaleFactor.get();

        // The chosen resolution is applied when the game starts, so a change to it
        // only takes effect after a restart.
        if ((pNewMode->width() != pCurrentMode.width()) || (pNewMode->height() != pCurrentMode.height()))
        {
            idsMessage = IDS_MENUMESSAGE_RESOLUTION;
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
        pCursorType_->setChecked(use2DMarker_);
        pGrabMouse_->setChecked(grabCursor_);
        pMusicVolume_->setValue(musicVolume_);
        pSoundVolume_->setValue(soundVolume_);
        vSyncModeDropDown_->setCurrentIndex(static_cast<int>(vsyncMode_));
        // Only restore gamma correction if gamma correction is supported
        if (pGammaCorrection_)
        {
            pGammaCorrection_->setValue(gammaCorrection_);
        }
        pCameraAccelerationSlider_->setValue(zenithCameraAcceleration_);

        // There is no explicit Ground camera control but it is possible to set different settings via config file
        Config::uiGroundCameraAcceleration.set(groundCameraAcceleration_);

        pStartupScreens_->buttonAction(MachGui::ButtonEvent::EXIT);
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
    Config::gfxResolutionWidth.set(pNewMode->width());
    Config::gfxResolutionHeight.set(pNewMode->height());
    Config::gfxRefreshRate.set(pNewMode->refreshRate());

    // Store gamma correction value
    if (pGammaCorrection_)
    {
        SysRegistry::instance().setIntegerValue(
            "Options\\Gamma Correction",
            "Value",
            static_cast<double>(pGammaCorrection_->value()) * GAMMA_REG_MULTIPLIER);
    }

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
    {
        using InputLayout = MachGui::InputLayout;
        Config::inputBaseLayout.set(pWasdControls_->isChecked() ? InputLayout::WASD : InputLayout::Legacy);
    }

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

    {
        MachGuiDropDownListBoxCreator::DropDownListBoxItem currentItem = pScaleFactorSelector_->item();
        int ScaleValue = *static_cast<const int*>(currentItem);
        Config::uiScaleFactor.set(ScaleValue);
    }
}

void MachGuiCtxOptions::readFromConfig()
{
    // Used to set gui items to reflect current game settings
    pSound3d_->setChecked(
        SysRegistry::instance().queryBooleanValue("Options\\3DSound", "on", SndMixer::instance().is3dMixer()));

    zenithCameraAcceleration_ = Config::uiZenithCameraAcceleration.get();
    groundCameraAcceleration_ = Config::uiGroundCameraAcceleration.get();

    pMusicVolume_->setValue(musicVolume_);
    pSoundVolume_->setValue(soundVolume_);

    pCursorType_->setChecked(use2DMarker_);
    pReverseKeys_->setChecked(SysRegistry::instance().queryIntegerValue("Options\\Reverse UpDown Keys", "on"));
    pReverseMouse_->setChecked(SysRegistry::instance().queryIntegerValue("Options\\Reverse BackForward Mouse", "on"));

    pGrabMouse_->setChecked(grabCursor_);
    {
        using InputLayout = MachGui::InputLayout;
        pWasdControls_->setChecked(Config::inputBaseLayout.get() == InputLayout::WASD);
    }

    pTransitions_->setChecked(pStartupScreens_->startupData()->transitionFlicsOn());

    vsyncMode_ = Config::gfxVSyncMode.get();
    vSyncModeDropDown_->setCurrentIndex(static_cast<int>(vsyncMode_));

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

        choicesOptimisations_[index]->setCurrentText(choice.asString());

        ++index;
    }

    cursorType2d_ = pCursorType_->isChecked();

    {
        int scaleFactorValue = Config::uiScaleFactor.get();

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

void MachGuiCtxOptions::exitFromOptions()
{
    exitFromOptions_ = true;
}

/* End CTXOPTNS.CPP *************************************************/
