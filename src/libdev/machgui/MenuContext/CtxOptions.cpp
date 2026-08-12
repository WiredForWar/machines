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
#include "render/RenderVariables.hpp"
#include "render/WindowMode.hpp"
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

namespace
{

// A percentage, or zero to leave the scale to the game.
constexpr int AllScaleFactorValues[] = {
    0,
    100,
    200,
};

} // namespace

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

    {
        GuiStrings windowModeNames = {
            ResolvedUiString(IDS_WINDOW_MODE_FULLSCREEN),
            ResolvedUiString(IDS_WINDOW_MODE_BORDERLESS),
            ResolvedUiString(IDS_WINDOW_MODE_WINDOWED),
        };

        pWindowMode_ = addDropDown(IDS_WINDOW_MODE);
        pWindowMode_->setAvailText(windowModeNames);
    }

    pScreenSize_ = addDropDown(IDS_MENU_SCREENSIZE);
    pRefreshRate_ = addDropDown(IDS_REFRESH_RATE);

    {
        GuiStrings itemNames = {
            ResolvedUiString(IDS_VSYNC_OPTION_AUTO),
            ResolvedUiString(IDS_VSYNC_OPTION_ENABLED),
            ResolvedUiString(IDS_VSYNC_OPTION_DISABLED),
        };

        vSyncModeDropDown_ = addDropDown(IDS_VSYNC_OPTION);
        vSyncModeDropDown_->setAvailText(itemNames);
    }

    pScaleFactorSelector_ = addDropDown(IDS_SCALE_FACTOR);

    pCameraAccelerationSlider_ = addSliderBar(IDS_CONFIG_CAMERA_ACCELERATION);
    pCameraAccelerationSlider_->minMax(1, 30);
    pCameraAccelerationSlider_->setValue(Config::uiZenithCameraAcceleration.get());
    pCameraAccelerationSlider_->setValueChangedHandler([](float v)
    {
        Config::uiZenithCameraAcceleration.set(v);
        Config::uiGroundCameraAcceleration.set(v);
    });

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
        // The label and the control beside it start on the line the check boxes in the
        // column to the left of them start on, as they do in the options above.
        const int optimisationsY = (OPTIMISATIONS_AREA_MINY + 33) * MachGui::menuScaleFactor();
        const auto optimizationAreaCoord = Gui::Coord(secondColumnX, optimisationsY);
        const auto choicesBaseCoord = Gui::Coord(secondColumnInputX, optimisationsY);
        for (MachPhysComplexityManager::ChoiceItems::const_iterator it = chItems.begin(); it != chItems.end(); ++it)
        {
            uint id = (*it)->id();
            uint nch = (*it)->nChoices();

            GuiStrings choices;
            choices.reserve(nch);

            for (uint ch = 0; ch < nch; ++ch)
            {
                GuiResourceString choice(ch + id + 1);
                choices.push_back(choice.asString());
            }
            GuiResourceString choiceTitle(id);

            new MachGuiMenuText(
                pStartupScreens,
                Gui::Box(
                    optimizationAreaCoord + Gui::Vec(0, verticalSpacing * index),
                    Gui::Size(secondColumnTextWidth, rowHeight)),
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

    pWindowMode_->setCurrentIndexChangedCallback([this]() { updateDisplayControls(); });
    pScreenSize_->setCurrentIndexChangedCallback([this]() {
        updateRefreshRates();
        updateScaleFactors();
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

        const RenDisplay* pDisplay = W4dManager::instance().sceneManager()->pDevice()->display();
        const RenDisplay::Mode& currentMode = pDisplay->currentMode();

        int newScaleFactorValue = Config::uiScaleFactor.get();

        // The display settings are applied when the game starts, so a change to any
        // of them only takes effect after a restart.
        if (selectedResolution() != currentMode.size() || selectedWindowMode() != pDisplay->windowMode())
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
        setDisplaySettings(windowMode_, resolution_, refreshRate_);
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

    // Store the new display settings in the registry
    {
        const Ren::Size resolution = selectedResolution();
        Config::gfxWindowMode.set(selectedWindowMode());
        Config::gfxResolutionWidth.set(resolution.width);
        Config::gfxResolutionHeight.set(resolution.height);
        Config::gfxRefreshRate.set(selectedRefreshRate());
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
        const int choice = choicesOptimisations_[index]->currentIndex();
        if (choice >= 0)
            MachPhysComplexityManager::instance().changeChoiceItem(id, choice);
        ++index;
    }

    Config::uiScaleFactor.set(selectedScaleFactor());
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

    {
        const RenDisplay* pDisplay = W4dManager::instance().sceneManager()->pDevice()->display();

        windowMode_ = pDisplay->windowMode();
        resolution_ = pDisplay->currentMode().size();
        refreshRate_ = pDisplay->currentMode().refreshRate();

        setDisplaySettings(windowMode_, resolution_, refreshRate_);
    }

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
        choicesOptimisations_[index]->setCurrentIndex(static_cast<int>((*it)->choice()));

        ++index;
    }

    cursorType2d_ = pCursorType_->isChecked();

    setScaleFactor(Config::uiScaleFactor.get());
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

Ren::WindowMode MachGuiCtxOptions::selectedWindowMode() const
{
    const int index = pWindowMode_->currentIndex();
    if (index < 0 || static_cast<std::size_t>(index) >= std::size(Ren::AllWindowModes))
        return Ren::WindowMode::Fullscreen;

    return Ren::AllWindowModes[index];
}

Ren::Size MachGuiCtxOptions::selectedResolution() const
{
    const int index = pScreenSize_->currentIndex();
    if (index < 0 || static_cast<std::size_t>(index) >= resolutions_.size())
        return {};

    return resolutions_[index];
}

int MachGuiCtxOptions::selectedRefreshRate() const
{
    const int index = pRefreshRate_->currentIndex();
    if (index < 0 || static_cast<std::size_t>(index) >= refreshRates_.size())
        return 0;

    return refreshRates_[index];
}

void MachGuiCtxOptions::updateDisplayControls()
{
    if (updatingDisplayControls_)
        return;

    updatingDisplayControls_ = true;

    // Only exclusive fullscreen puts the display into a mode of our choosing. The
    // other two take the resolution the display is already in, so there is nothing
    // to offer beyond saying what that is.
    const bool ownsTheMode = selectedWindowMode() == Ren::WindowMode::Fullscreen;
    const bool ownsTheSize = ownsTheMode || selectedWindowMode() == Ren::WindowMode::Windowed;

    updateResolutions();
    pScreenSize_->setEnabled(ownsTheSize);

    updatingDisplayControls_ = false;

    updateRefreshRates();
    updateScaleFactors();
}

void MachGuiCtxOptions::updateResolutions()
{
    const RenDisplay* pDisplay = W4dManager::instance().sceneManager()->pDevice()->display();
    const Ren::Size desktop = pDisplay->getDesktopDisplayMode().size();
    const Ren::Size wanted = selectedResolution();

    resolutions_.clear();
    GuiStrings labels;

    if (selectedWindowMode() == Ren::WindowMode::Borderless)
    {
        // The display stays in the mode it is already in.
        resolutions_.push_back(desktop);
    }
    else
    {
        // A window larger than the display cannot be shown in full, and the menus
        // are drawn for 640x480 upwards.
        for (const Ren::Size& resolution : pDisplay->modeCatalogue().resolutions())
        {
            const bool fits = desktop.isNull()
                || (resolution.width <= desktop.width && resolution.height <= desktop.height);

            if (fits && resolution.width >= 640 && resolution.height >= 480)
                resolutions_.push_back(resolution);
        }
    }

    if (resolutions_.empty())
        resolutions_.push_back(desktop.isNull() ? Ren::Size(640, 480) : desktop);

    labels.reserve(resolutions_.size());
    for (const Ren::Size& resolution : resolutions_)
    {
        char buffer[30];
        snprintf(buffer, sizeof(buffer), "%dx%d", resolution.width, resolution.height);
        labels.push_back(buffer);
    }

    pScreenSize_->setAvailText(labels);
    pScreenSize_->setCurrentIndex(indexOfResolution(wanted));
}

void MachGuiCtxOptions::updateScaleFactors()
{
    if (updatingDisplayControls_)
        return;

    const int wanted = selectedScaleFactor();
    const Ren::Size resolution = selectedResolution();

    scaleFactors_.clear();
    GuiStrings labels;

    for (const int percents : AllScaleFactorValues)
    {
        if (! MachGui::scaleFactorFits(percents, resolution))
            continue;

        scaleFactors_.push_back(percents);
        if (percents == 0)
        {
            labels.push_back(ResolvedUiString(IDS_MENU_DEFAULT));
        }
        else
        {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%d%%", percents);
            labels.push_back(buffer);
        }
    }

    pScaleFactorSelector_->setAvailText(labels);

    // Keep the scale that was chosen where the resolution still leaves room for
    // it, and fall back to letting the game decide where it does not.
    int index = 0;
    for (std::size_t i = 0; i < scaleFactors_.size(); ++i)
    {
        if (scaleFactors_[i] == wanted)
        {
            index = static_cast<int>(i);
            break;
        }
    }
    pScaleFactorSelector_->setCurrentIndex(index);
}

void MachGuiCtxOptions::setScaleFactor(int scaleFactorPercents)
{
    for (std::size_t i = 0; i < scaleFactors_.size(); ++i)
    {
        if (scaleFactors_[i] == scaleFactorPercents)
        {
            pScaleFactorSelector_->setCurrentIndex(static_cast<int>(i));
            return;
        }
    }

    // The resolution leaves no room for the scale that was asked for, so leave it
    // to the game.
    pScaleFactorSelector_->setCurrentIndex(0);
}

int MachGuiCtxOptions::selectedScaleFactor() const
{
    const int index = pScaleFactorSelector_->currentIndex();
    if (index < 0 || static_cast<std::size_t>(index) >= scaleFactors_.size())
        return 0;

    return scaleFactors_[index];
}

void MachGuiCtxOptions::updateRefreshRates()
{
    if (updatingDisplayControls_)
        return;

    updatingDisplayControls_ = true;

    const RenDisplay* pDisplay = W4dManager::instance().sceneManager()->pDevice()->display();
    const int wanted = selectedRefreshRate();

    refreshRates_.clear();

    if (selectedWindowMode() == Ren::WindowMode::Fullscreen)
    {
        const std::vector<int>& offered = pDisplay->modeCatalogue().refreshRates(selectedResolution());
        refreshRates_.assign(offered.begin(), offered.end());
    }
    else
    {
        // The rate belongs to the mode the display is already in.
        refreshRates_.push_back(pDisplay->getDesktopDisplayMode().refreshRate());
    }

    if (refreshRates_.empty())
        refreshRates_.push_back(pDisplay->currentMode().refreshRate());

    GuiStrings labels;
    labels.reserve(refreshRates_.size());
    for (const int rate : refreshRates_)
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d Hz", rate);
        labels.push_back(buffer);
    }

    pRefreshRate_->setAvailText(labels);

    // Keep the rate that was chosen when the resolution still offers it; the
    // highest one leads the list otherwise.
    int index = 0;
    for (std::size_t i = 0; i < refreshRates_.size(); ++i)
    {
        if (refreshRates_[i] == wanted)
        {
            index = static_cast<int>(i);
            break;
        }
    }
    pRefreshRate_->setCurrentIndex(index);

    // There is a rate to choose only where the mode is ours to set and the
    // resolution is offered at more than one of them. Settled here rather than
    // alongside the other two controls because it takes the length of the list,
    // which changes with the resolution the player lands on.
    pRefreshRate_->setEnabled(selectedWindowMode() == Ren::WindowMode::Fullscreen && refreshRates_.size() > 1);

    updatingDisplayControls_ = false;
}

void MachGuiCtxOptions::setDisplaySettings(Ren::WindowMode windowMode, Ren::Size resolution, int refreshRate)
{
    for (std::size_t i = 0; i < std::size(Ren::AllWindowModes); ++i)
    {
        if (Ren::AllWindowModes[i] == windowMode)
        {
            pWindowMode_->setCurrentIndex(static_cast<int>(i));
            break;
        }
    }

    // Each list depends on the choice above it, so all three are settled in turn
    // rather than left to the change handlers, which are not in place yet the first
    // time round.
    updateDisplayControls();

    pScreenSize_->setCurrentIndex(indexOfResolution(resolution));
    updateRefreshRates();
    updateScaleFactors();

    for (std::size_t i = 0; i < refreshRates_.size(); ++i)
    {
        if (refreshRates_[i] == refreshRate)
        {
            pRefreshRate_->setCurrentIndex(static_cast<int>(i));
            break;
        }
    }
}

int MachGuiCtxOptions::indexOfResolution(Ren::Size resolution) const
{
    for (std::size_t i = 0; i < resolutions_.size(); ++i)
    {
        if (resolutions_[i] == resolution)
            return static_cast<int>(i);
    }

    return 0;
}

/* End CTXOPTNS.CPP *************************************************/
