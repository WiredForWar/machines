/*
 * I N G A M E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "machgui/ingame.hpp"
#include "machgui/hwrebank.hpp"
#include "machgui/cameras.hpp"
#include "machgui/squadron.hpp"
#include "machgui/gui.hpp"
#include "machgui/guictrl.hpp"
#include "machgui/corral.hpp"
#include "machgui/commbank.hpp"
#include "machgui/worldvie.hpp"
#include "machgui/consmenu.hpp"
#include "machgui/hwremenu.hpp"
#include "machgui/promtext.hpp"
#include "machgui/igcurs2d.hpp"
#include "machgui/prodbank.hpp"
#include "machgui/squadron.hpp"
#include "machgui/bmutext.hpp"
#include "machgui/oneicon.hpp"
#include "machgui/bildmenu.hpp"
#include "machgui/machnav.hpp"
#include "machgui/map.hpp"
#include "machgui/actnamid.hpp"
#include "machgui/fstpersn.hpp"
#include "machgui/ctrladon.hpp"
#include "machgui/camscrol.hpp"
#include "machgui/controlp.hpp"
#include "machgui/maparea.hpp"
#include "machgui/chatmsgs.hpp"
#include "machgui/chatmsgd.hpp"
#include "machgui/internal/igameimp.hpp"
#include "machgui/internal/mgsndman.hpp"
#include "machgui/database.hpp"
#include "machgui/dbscenar.hpp"

// Commands
#include "machgui/commands/cmdattac.hpp"
#include "machgui/commands/cmdtrech.hpp"
#include "machgui/commands/cmdmove.hpp"
#include "machgui/commands/cmddeplo.hpp"
#include "machgui/commands/cmdconst.hpp"
#include "machgui/commands/cmdlocto.hpp"
#include "machgui/commands/cmdlmine.hpp"
#include "machgui/commands/cmdrefil.hpp"
#include "machgui/commands/cmdheal.hpp"
#include "machgui/commands/cmdpatro.hpp"
#include "machgui/commands/cmddefau.hpp"
#include "machgui/commands/cmdbuild.hpp"
#include "machgui/commands/cmdresea.hpp"
#include "machgui/commands/cmdscav.hpp"
#include "machgui/commands/cmdtrans.hpp"
#include "machgui/commands/cmdpicku.hpp"
#include "machgui/commands/cmddestr.hpp"
#include "machgui/commands/cmdfrmsq.hpp"
#include "machgui/commands/cmddecon.hpp"
#include "machgui/commands/cmdrecyc.hpp"
#include "machgui/commands/cmdrepar.hpp"
#include "machgui/commands/cmdcaptr.hpp"
#include "machgui/commands/cmdstop.hpp"
#include "machgui/commands/cmddefcn.hpp"
#include "machgui/commands/cmdstand.hpp"
#include "machgui/commands/cmdassem.hpp"
#include "machgui/commands/cmdionat.hpp"
#include "machgui/commands/cmdnukat.hpp"
#include "machgui/commands/cmdcamo.hpp"

#include "machlog/machlog.hpp"
#include "machlog/races.hpp"
#include "machlog/factory.hpp"
#include "machlog/debris.hpp"
#include "machlog/hwlab.hpp"
#include "machlog/pod.hpp"
#include "machlog/stats.hpp"
#include "machlog/vmman.hpp"
#include "machlog/network.hpp"
#include "machlog/mine.hpp"
#include "machlog/minesite.hpp"
#include "machlog/strategy.hpp"
#include "machlog/technici.hpp"
#include "machlog/oreholo.hpp"
#include "machlog/apc.hpp"
#include "machphys/techdata.hpp"
#include "machphys/apcdata.hpp"
#include "machphys/rcardata.hpp"
#include "gui/gui.hpp"
#include "gui/event.hpp"
#include "gui/manager.hpp"
#include "gui/painter.hpp"
#include "gui/restring.hpp"
#include "world4d/scenemgr.hpp"
#include "world4d/domain.hpp"
#include "world4d/soundman.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "render/capable.hpp"
#include "afx/resource.hpp"
#include "base/IProgressReporter.hpp"
#include "machgui/internal/strings.hpp"
#include "sim/manager.hpp"
#include "network/node.hpp"
#include "device/time.hpp"
#include "system/registry.hpp"

#include <algorithm>
#include <stdio.h>

MachInGameScreenImpl::MachInGameScreenImpl()
    : pSceneManager_(nullptr)
    , pRoot_(nullptr)
    , pWorldViewWindow_(nullptr)
    , pContinentMap_(nullptr)
    , pMachinesIcon_(nullptr)
    , pConstructionsIcon_(nullptr)
    , pSquadronIcon_(nullptr)
    , pMachineNavigation_(nullptr)
    , pConstructionNavigation_(nullptr)
    , pCorral_(nullptr)
    , pPromptText_(nullptr)
    , pCommandIcons_(nullptr)
    , pConstructMenu_(nullptr)
    , pBuildMenu_(nullptr)
    , pProductionBank_(nullptr)
    , pHWResearchMenu_(nullptr)
    , pHWResearchBank_(nullptr)
    , pSquadronBank_(nullptr)
    , pHighlightedActor_(nullptr)
    , pDefconCommand_(nullptr)
    , pSelfDestructCommand_(nullptr)
    , pIonAttackCommand_(nullptr)
    , commandBankNeedsUpdating_(true)
    , cancelActiveCommand_(false)
    , switchToMenus_(false)
    , switchGuiRoot_(false)
    , inFirstPerson_(false)
    , controlPanelContext_(MachGui::MAIN_MENU)
    , cursorFilter_(W4dDomain::EXCLUDE_NOT_SOLID)
    , pPromptTextActor_(nullptr)
    , pControlPanelAddOn_(nullptr)
    , controlPanelOn_(true)
    , // Control panel should be out to start with
    controlPanelXPos_(MachGui::controlPanelInXPos())
    , // Control panel starts in hidden position ( slides out when you first enter the game )
    pControlPanel_(nullptr)
    , pMapArea_(nullptr)
    , pTopCameraScrollArea_(nullptr)
    , pBottomCameraScrollArea_(nullptr)
    , pLeftCameraScrollArea_(nullptr)
    , pRightCameraScrollArea_(nullptr)
    , resolutionChanged_(true)
    , renderingScreenShot_(false)
    , corralState_(MachInGameScreen::CORRAL_EMPTY)
    , pChatMessageDisplay_(nullptr)
    , networkStuffedStartTime_(0)
    , disableFirstPerson_(false)
    , instantExit_(false)
    ,
#ifndef PRODUCTION
    showCurrentMachine_(false)
    , showNetworkPing_(false)
    , showNetworkStuffed_(false)
    ,
#endif
    redrawMapCounter_(0)
{
  // Intentionally empty
}

// MachInGameScreen GuiRoot is made very big (10000x10000) to cope with all possible screen resolutions.
MachInGameScreen::MachInGameScreen(W4dSceneManager* pSceneManager, W4dRoot* pRoot, IProgressReporter* pReporter)
    : GuiRoot(Gui::Box(
        0,
        0,
        10000 /*pSceneManager->pDevice()->windowWidth()*/,
        10000 /*pSceneManager->pDevice()->windowHeight()*/))
    , pImpl_(new MachInGameScreenImpl())
{
    CB_DEPIMPL_AUTO(inFirstPerson_);
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pFirstPerson_);
    CB_DEPIMPL_AUTO(pPromptTextActor_);
    CB_DEPIMPL_AUTO(pControlPanelAddOn_);
    CB_DEPIMPL_AUTO(pCameras_);
    CB_DEPIMPL_AUTO(pWorldViewWindow_);
    CB_DEPIMPL_AUTO(pBmuButton_);
    CB_DEPIMPL_AUTO(gameState_);
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pRoot_);
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(allCommands_);
    CB_DEPIMPL_AUTO(pStringResourceLib_);
    CB_DEPIMPL_AUTO(pMachinesIcon_);
    CB_DEPIMPL_AUTO(pConstructionsIcon_);
    CB_DEPIMPL_AUTO(pSquadronIcon_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);
    CB_DEPIMPL_AUTO(pCorral_);
    CB_DEPIMPL_AUTO(pSmallCommandIcons_);
    CB_DEPIMPL_AUTO(pCommandIcons_);
    CB_DEPIMPL_AUTO(pCursors2d_);
    CB_DEPIMPL_AUTO(pControlPanel_);
    CB_DEPIMPL_AUTO(pMapArea_);
#ifndef PRODUCTION
    CB_DEPIMPL_AUTO(showCurrentMachine_);
    CB_DEPIMPL_AUTO(showNetworkStuffed_);
    string showStuffedString;
    showNetworkStuffed_ = (nullptr != getenv("CB_SHOW_NETWORK_STUFFED"));
#endif

    // Store sceneManager and root
    pSceneManager_ = pSceneManager;
    pRoot_ = pRoot;

    // Set reasonable collection allocation size
    selectedActors_.reserve(64);
    allCommands_.reserve(32);

    // Set up area to hold map and other buttons at top of control panel
    pMapArea_
        = new MachGuiMapArea(this, Gui::Boundary(0, 0, MachGui::controlPanelOutXPos(), MachGui::mapAreaHeight()));
    pReporter->report(10, 100); // 10% done

    // Set the viewport boundary for the world view window
    RenDevice& device = *pSceneManager->pDevice();
    const int w = device.windowWidth();
    const int h = device.windowHeight();

    // Set up sliding control panel section
    pControlPanel_ = new 
        MachGuiControlPanel(this, Gui::Boundary(0, MachGui::mapAreaHeight(), MachGui::controlPanelOutXPos(), h), this);
    pReporter->report(15, 100); // 15% done

    // Create cameras
    pCameras_= std::make_unique<MachCameras>(pSceneManager, pRoot);
    pReporter->report(20, 100); // 20% done

    // Load the string table resource file
    pStringResourceLib_ = std::make_unique<AfxResourceLib>(SysPathName("machstrg.xml"));
    GuiResourceString::resource(pStringResourceLib_.get());
    pReporter->report(25, 100); // 25% done

    // Construct the set of available commands
    initialiseAllCommands();
    pReporter->report(28, 100); // 28% done

    // Create the world view window
    pWorldViewWindow_ = new MachWorldViewWindow(
        this,
        Gui::Boundary(
            MachGui::controlPanelOutXPos(),
            0,
            (w + MachGui::controlPanelOutXPos() - MachGui::controlPanelInXPos()),
            h),
        pCameras_.get());
    pReporter->report(30, 100); // 30% done

    // Construct add-on piece of control panel ( top-right )
    pControlPanelAddOn_ = new MachGuiControlPanelAddOn(this, Gui::Coord(MachGui::controlPanelOutXPos(), 0), this);
    pReporter->report(35, 100); // 35% done

    // Construct continent map
    pContinentMap_ = new MachContinentMap(pMapArea_, Gui::Coord(2, 1), pCameras_.get(), this);
    pReporter->report(40, 100); // 40% done

    // Construct buttons down side of continent map
    Gui::XCoord x = pContinentMap_->absoluteBoundary().maxCorner().x();

    pBmuButton_ = new MachGuiBmuButton(
        pMapArea_,
        Gui::Coord(x, 0),
        MachGui::getScaledImagePath("gui/navigate/bmus.bmp"),
        pContinentMap_->bmuText(),
        pContinentMap_,
        this);
    pMachinesIcon_
        = new MachMachinesIcon(pMapArea_, Gui::Coord(x, pBmuButton_->absoluteBoundary().maxCorner().y() - 1), this);
    pConstructionsIcon_ = new 
        MachConstructionsIcon(pMapArea_, Gui::Coord(x, pMachinesIcon_->absoluteBoundary().maxCorner().y() - 1), this);
    pSquadronIcon_ = new 
        MachSquadronIcon(pMapArea_, Gui::Coord(x, pConstructionsIcon_->absoluteBoundary().maxCorner().y() - 1), this);
    pBmuButton_->setLayer(GuiDisplayable::LAYER2);
    pMachinesIcon_->setLayer(GuiDisplayable::LAYER2);
    pConstructionsIcon_->setLayer(GuiDisplayable::LAYER2);
    pSquadronIcon_->setLayer(GuiDisplayable::LAYER2);
    pReporter->report(50, 100); // 50% done

    // Construct single icon corral
    pCorralSingleIcon_ = new MachGuiCorralSingleIcon(pControlPanel_, Gui::Coord(1, 0), this);

    // Construct small command icons
    Gui::Coord smallCommandCoord(2, pCorralSingleIcon_->height() + 2);
    pSmallCommandIcons_ = new MachSmallCommandIcons(pControlPanel_, smallCommandCoord, this);
    pReporter->report(55, 100); // 55% done

    // Create navigators
    setupNavigators();
    pReporter->report(60, 100); // 60% done

    // Reset the context to main menu context
    resetContext();

    // Load up the 2d cursors
    pCursors2d_ = std::make_unique<MachInGameCursors2d>(this);
    pReporter->report(65, 100); // 65% done

    // Create first person interface
    pFirstPerson_ = std::make_unique<MachGuiFirstPerson>(pSceneManager, pRoot, this);
    pReporter->report(70, 100); // 70% done

    useFastSecondDisplay(false);

#ifndef PRODUCTION
    showCurrentMachine_ = false;
#endif

    DEBUG_STREAM(DIAG_NEIL, "MachInGameScreen::CTOR exit" << std::endl);
}

MachInGameScreen::~MachInGameScreen()
{
    DEBUG_STREAM(DIAG_NEIL, "MachInGameScreen::DTOR enter" << std::endl);

    // NB All the child GuiDisplayables are deleted automatically

    // Unload the string table resource file
    GuiResourceString::clearResource();

    // Delete implementation class
    delete pImpl_;

    DEBUG_STREAM(DIAG_NEIL, "MachInGameScreen::DTOR leave" << std::endl);
}

void MachInGameScreen::doBecomeRoot()
{
    CB_DEPIMPL_AUTO(pCursors2d_);
    CB_DEPIMPL_AUTO(pControlPanel_);
    CB_DEPIMPL_AUTO(pProductionBank_);
    CB_DEPIMPL_AUTO(pHWResearchBank_);
    CB_DEPIMPL_AUTO(resolutionChanged_);
    CB_DEPIMPL_AUTO(pChatMessageDisplay_);
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(redrawMapCounter_);
    CB_DEPIMPL_AUTO(pCameras_);

    // Reposition debug text
    pSceneManager_->pDevice()->debugTextCoords(204, 0);

    if (resolutionChanged_)
    {
        // Setup areas of screen that cause camera to scroll
        setupCameraScrollAreas();
        // Reposition prompt text
        setupPromptText();
        // Reposition control panel decals
        pControlPanel_->setupDecalCoords();
        // Reposition navigators
        setupNavigators();
        // Resize corral and reposition command icons
        setupCorralAndCommandIcons();
        // Clean up production and hw-research banks before resetting context, this
        // will ensure that they are recreated.
        delete pProductionBank_;
        pProductionBank_ = nullptr;
        delete pHWResearchBank_;
        pHWResearchBank_ = nullptr;
        // Change context to same context but get code to recreate the
        // gui controls relevant to that context.
        currentContext(currentContext(), true);

        // Reset resolution changed flag
        resolutionChanged_ = false;
    }

    // Change to menu cursor.
    pCursors2d_->forceCursor(MachGui::MENU_CURSOR);

    // Remove any previously displayed prompt text
    clearCursorPromptText();
    clearCommandPromptText();

    // Make sure that we redraw all chat messages
    if (pChatMessageDisplay_)
        pChatMessageDisplay_->forceUpdate();

    // Fully redraw map for next 3 frames
    redrawMapCounter_ = 3;

    // Configure pitch up/down keys for ground camera
    pCameras_->reversePitchUpDownKeys(SysRegistry::instance().queryIntegerValue("Options\\Reverse UpDown Keys", "on"));
}

void MachInGameScreen::doBecomeNotRoot()
{
    // Stop all playing sounds
    W4dSoundManager::instance().stopAll();
    // Clean up any gui sounds that are currently playing
    MachGuiSoundManager::instance().clearAll();
}

MachWorldViewWindow& MachInGameScreen::worldViewWindow()
{
    CB_DEPIMPL_AUTO(pWorldViewWindow_);

    PRE(pWorldViewWindow_ != nullptr);

    return *pWorldViewWindow_;
}

MachGuiCorral& MachInGameScreen::corral()
{
    CB_DEPIMPL_AUTO(pCorral_);

    return *pCorral_;
}

const MachInGameScreen::Actors& MachInGameScreen::selectedActors() const
{
    CB_DEPIMPL_AUTO(selectedActors_);

    return selectedActors_;
}

bool MachInGameScreen::isSelected(MachActor& actor) const
{
    return actor.selectionState() == MachLog::SELECTED;
}

void MachInGameScreen::select(MachActor* pActor)
{
    select(Actors({ pActor }));
}

void MachInGameScreen::select(const Actors& actors)
{
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);
    CB_DEPIMPL_AUTO(pFirstPerson_);
    CB_DEPIMPL_AUTO(commandBankNeedsUpdating_);

    DEBUG_STREAM(DIAG_NEIL, "MachInGameScreen::select( const Actors& actors )");

    for (Actors::const_iterator iter = actors.begin(); iter != actors.end(); ++iter)
    {
        MachActor* pActor = (*iter);

        // Check to see if actor is already selected!!
        ASSERT(! isSelected(*pActor), "actor already selected");

        // Mark actor as selected
        pActor->selectionState(MachLog::SELECTED);

        // Add to local collection
        selectedActors_.push_back(pActor);

        // Become an observer, in case it gets deleted etc
        pActor->attach(this);
    }

    // Add to corral
    corral().add(actors);

    updateCorralState();

    // Update single icon corral and first person
    if (selectedActors_.size() == 1)
    {
        // Get first actor
        MachActor* pActor = selectedActors_.front();

        // Update single icon corral
        pCorralSingleIcon_->setActor(pActor, true);

        // Tell first person control which actor has been selected
        pFirstPerson_->setActor(pActor);
    }
    else
    {
        pCorralSingleIcon_->clear();
        pFirstPerson_->resetActor();
    }

    // Command buttons will need processing
    commandBankNeedsUpdating_ = true;

    // Reset the self-destruct icon
    MachGuiSelfDestructCommand::resetButtonState();

    // Any active command should be cancelled - but deferred to next update
    cancelActiveCommand();

    // Selection has changed therefore we no longer have a squad
    applyCommandToSquadron(false);

    checkDismissNavigator();

    // Add/remove any associated banks
    setupActorBank();

    DEBUG_STREAM(DIAG_NEIL, "leaving MachInGameScreen::select( const Actors& actors )");
}

void MachInGameScreen::deselect(const Actors& actors)
{
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);
    CB_DEPIMPL_AUTO(pFirstPerson_);
    CB_DEPIMPL_AUTO(commandBankNeedsUpdating_);

    removeHighlightedActor();

    for (Actors::const_iterator iter = actors.begin(); iter != actors.end(); ++iter)
    {
        MachActor* pActor = (*iter);

        // Check to see if actor is selected. Cannot deselect an unselected actor!!
        ASSERT(isSelected(*pActor), "actor not selected");

        // Cease being an observer
        pActor->detach(this);

        // Mark actor as not selected
        pActor->selectionState(MachLog::NOT_SELECTED);

        // Remove from local collection
        Actors::iterator it = find(selectedActors_.begin(), selectedActors_.end(), pActor);
        ASSERT(it != selectedActors_.end(), "");

        selectedActors_.erase(it);
    }

    // Remove from corral
    corral().remove(actors);

    updateCorralState();

    // Update single icon corral and first person
    if (selectedActors_.size() == 1)
    {
        // Get first actor
        MachActor* pActor = selectedActors_.front();

        // Update single icon corral
        pCorralSingleIcon_->setActor(pActor, true);

        // Tell first person control which actor has been selected
        pFirstPerson_->setActor(pActor);
    }
    else
    {
        pCorralSingleIcon_->clear();
        pFirstPerson_->resetActor();
    }

    // Command buttons will need processing
    commandBankNeedsUpdating_ = true;

    // Reset the self-destruct icon
    MachGuiSelfDestructCommand::resetButtonState();

    // Any active command should be cancelled - but deferred to next update
    cancelActiveCommand();

    // Selection has changed therefore we no longer have a squad
    applyCommandToSquadron(false);

    checkDismissNavigator();

    // Add/remove any associated banks
    setupActorBank();

    // Reset the defcon command's "movement direction" (which way it will move from the middle setting)
    defconCommand()->resetDirectionFromDefConNormal();
}

void MachInGameScreen::deselect(MachActor* pActor)
{
    PRE(isSelected(*pActor));

    // Cease being an observer
    pActor->detach(this);

    // Do the bulk of the work
    unselect(pActor);
}

void MachInGameScreen::unselect(MachActor* pActor)
{
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);
    CB_DEPIMPL_AUTO(pFirstPerson_);
    CB_DEPIMPL_AUTO(commandBankNeedsUpdating_);

    removeHighlightedActor();

    // Mark actor as not selected
    pActor->selectionState(MachLog::NOT_SELECTED);

    // Remove from local collection
    Actors::iterator it = find(selectedActors_.begin(), selectedActors_.end(), pActor);
    ASSERT(it != selectedActors_.end(), "");

    selectedActors_.erase(it);

    // Remove from corral
    corral().remove(pActor);

    updateCorralState();

    // Update single icon corral and first person
    if (selectedActors_.size() == 1)
    {
        // Get first actor
        MachActor* pActor = selectedActors_.front();

        // Update single icon corral
        pCorralSingleIcon_->setActor(pActor, true);

        // Tell first person control which actor has been selected
        pFirstPerson_->setActor(pActor);
    }
    else
    {
        pCorralSingleIcon_->clear();
        pFirstPerson_->resetActor();

        if (selectedActors_.size() == 0)
        {
            // Reset the defcon command's "movement direction"
            defconCommand()->resetDirectionFromDefConNormal();
        }
    }

    // Command buttons will need processing
    commandBankNeedsUpdating_ = true;

    // Reset the self-destruct icon
    MachGuiSelfDestructCommand::resetButtonState();

    // Any active command should be cancelled - but deferred to next update
    cancelActiveCommand();

    // Selection has changed therefore we no longer have a squad
    applyCommandToSquadron(false);

    checkDismissNavigator();

    // Add/remove any associated banks
    setupActorBank();
}

void MachInGameScreen::checkDismissNavigator()
{
    CB_DEPIMPL_AUTO(pMachinesIcon_);
    CB_DEPIMPL_AUTO(pConstructionsIcon_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);

    // Called when actor is selected/deselected.

    // Check to see if selection should cause navigator to be dismissed
    if (! isMachineNavigationContext() || ! pMachineNavigation_->remainVisible())
    {
        if (! isConstructionNavigationContext() || ! pConstructionNavigation_->remainVisible())
        {
            pMachinesIcon_->forceUp(); // Make sure machines navigation button pops up as we are leaving navigation mode
            pConstructionsIcon_->forceUp();

            mainMenuOrSingleFactoryContext();
        }
    }
    // Any other selections will cause the navigator to dismiss unless the navigator resets the
    // remain visible flag to true
    pMachineNavigation_->remainVisible(false);
    pConstructionNavigation_->remainVisible(false);
}

void MachInGameScreen::deselectAll()
{
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(commandBankNeedsUpdating_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);
    CB_DEPIMPL_AUTO(pFirstPerson_);

    removeHighlightedActor();

    for (Actors::iterator iter = selectedActors_.begin(); iter != selectedActors_.end(); ++iter)
    {
        MachActor* pActor = *iter;

        // Cease being an observer
        pActor->detach(this);

        // Mark actor as not selected
        pActor->selectionState(MachLog::NOT_SELECTED);
    }

    // Remove from corral
    corral().remove(selectedActors_);

    // Clean up list of selected actors
    selectedActors_.erase(selectedActors_.begin(), selectedActors_.end());

    updateCorralState();

    // Command buttons will need processing
    commandBankNeedsUpdating_ = true;

    // Reset the self-destruct icon
    MachGuiSelfDestructCommand::resetButtonState();

    // Any active command should be cancelled - but deferred to next update
    cancelActiveCommand();

    // Selection has changed therefore we no longer have a squad
    applyCommandToSquadron(false);

    checkDismissNavigator();

    // Add/remove any associated banks
    setupActorBank();

    // Remove reference to selected actors from 1st person and single icon corral
    pCorralSingleIcon_->clear();
    pFirstPerson_->resetActor();
}

MachInGameScreen::Actors MachInGameScreen::getVisibleActors() const
{
    ctl_pvector<W4dEntity> entities = pImpl_->pWorldViewWindow_->getEntitiesInView();

    for (const W4dEntity* pEntity : entities)
    {
        std::cerr << pEntity->id() << " " << pEntity->name() << std::endl;
    }

    Actors actors;
    actors.reserve(std::min<size_t>(12, entities.size()));

    MachLogRaces& races = MachLogRaces::instance();
    for (const W4dEntity* pEntity : entities)
    {
        if (races.actorExists(pEntity->id()))
        {
            MachActor& actor = races.actor(pEntity->id());
            actors.push_back(&actor);
        }
    }

    return actors;
}

W4dSceneManager& MachInGameScreen::sceneManager() const
{
    CB_DEPIMPL_AUTO(pSceneManager_);

    return *pSceneManager_;
}

MachGuiDefconCommand* MachInGameScreen::defconCommand()
{
    CB_DEPIMPL_AUTO(pDefconCommand_);

    return pDefconCommand_;
}

MachGuiSelfDestructCommand* MachInGameScreen::selfDestructCommand()
{
    CB_DEPIMPL_AUTO(pSelfDestructCommand_);

    return pSelfDestructCommand_;
}

void MachInGameScreen::initialiseAllCommands()
{
    CB_DEPIMPL_AUTO(pDefaultCommand_);
    CB_DEPIMPL_AUTO(pDefconCommand_);
    CB_DEPIMPL_AUTO(pSelfDestructCommand_);
    CB_DEPIMPL_AUTO(pIonAttackCommand_);
    CB_DEPIMPL_AUTO(pNukeAttackCommand_);
    CB_DEPIMPL_AUTO(allCommands_);

    DEBUG_STREAM(DIAG_NEIL, "MachInGameScreen::initialiseAllCommands" << std::endl << std::flush);
    pDefaultCommand_ = std::make_unique<MachGuiDefaultCommand>(this);

    // Commands with special icon processing
    pSelfDestructCommand_ = new MachGuiSelfDestructCommand(this);
    pIonAttackCommand_ = new MachGuiIonAttackCommand(this);
    pNukeAttackCommand_ = new MachGuiNukeAttackCommand(this);
    pDefconCommand_ = new MachGuiDefconCommand(this);

    allCommands_.emplace_back(pSelfDestructCommand_);
    allCommands_.emplace_back(std::make_unique<MachGuiMoveCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiStopCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiAttackCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiConstructCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiRepairCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiCaptureCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiDeconstructCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiRecycleCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiLocateToCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiDropLandMineCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiRefillLandMineCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiDeployCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiPatrolCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiTransportCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiPickUpCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiScavengeCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiTreacheryCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiFormSquadronCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiHealCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiStandGroundCommand>(this));
    allCommands_.emplace_back(std::make_unique<MachGuiAssemblyPointCommand>(this));
    allCommands_.emplace_back(pIonAttackCommand_);
    allCommands_.emplace_back(pNukeAttackCommand_);
    allCommands_.emplace_back(pDefconCommand_);
    allCommands_.emplace_back(std::make_unique<MachGuiCamouflageCommand>(this));

    DEBUG_STREAM(DIAG_NEIL, "allcommands.size " << allCommands_.size() << std::endl << std::flush);
}

const MachInGameScreen::Commands& MachInGameScreen::allCommands() const
{
    CB_DEPIMPL_AUTO(allCommands_);

    return allCommands_;
}

MachGuiCommand& MachInGameScreen::defaultCommand() const
{
    CB_DEPIMPL_AUTO(pDefaultCommand_);

    PRE(pDefaultCommand_ != nullptr);

    return *pDefaultCommand_;
}

void MachInGameScreen::activateDefaultCommand()
{
    CB_DEPIMPL_AUTO(pActiveCommand_);
    CB_DEPIMPL_AUTO(cancelActiveCommand_);

    // Cancel any existing command
    if (pActiveCommand_ != nullptr)
    {
        pActiveCommand_->finish();
        pActiveCommand_.reset();
    }

    // Ensure the command prompt text is cleared
    clearCommandPromptText();

    // Any outstanding request hereby fulfilled
    cancelActiveCommand_ = false;
}

void MachInGameScreen::activeCommand(const MachGuiCommand& command)
{
    PRE(command.isVisible());

    CB_DEPIMPL_AUTO(pActiveCommand_);

    // Can't issue commands if game is paused or network is busy
    if (SimManager::instance().isSuspended() || isNetworkStuffed())
    {
        MachGuiSoundManager::instance().playSound("gui/sounds/clickbad.wav");
    }
    else
    {
        // Make a fresh copy of the required command
        std::unique_ptr<MachGuiCommand> pNewCommand = command.clone();

        // Cancel any existing command
        if (pActiveCommand_ != nullptr)
        {
            pActiveCommand_->finish();
        }

        // Store copy of the required command
        pActiveCommand_ = std::move(pNewCommand);

        // Load the string for the command
        GuiResourceString::Id id = pActiveCommand_->commandPromptStringid();
        GuiResourceString prompt(id);

        // Set the command prompt
        commandPromptText(prompt.asString());

        // Do start processing
        pActiveCommand_->start();
    }
}

MachGuiCommand& MachInGameScreen::activeCommand() const
{
    CB_DEPIMPL_AUTO(pActiveCommand_);
    CB_DEPIMPL_AUTO(pDefaultCommand_);

    PRE(pActiveCommand_ != nullptr || pDefaultCommand_ != nullptr);
    return (pActiveCommand_ != nullptr ? *pActiveCommand_ : *pDefaultCommand_);
}

void MachInGameScreen::setCursorFilter(int filter)
{
    CB_DEPIMPL(W4dDomain::EntityFilter, cursorFilter_);

    // cursorFilter_ = filter;
    cursorFilter_ = static_cast<W4dDomain ::EntityFilter>(filter);
}

int MachInGameScreen::cursorFilter() const
{
    CB_DEPIMPL(W4dDomain::EntityFilter, cursorFilter_);

    return cursorFilter_;
}

void MachInGameScreen::highlightActor(MachActor* pHighlightActor)
{
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(pHighlightedActor_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);

    if (selectedActors_.empty())
    {
        pHighlightedActor_ = pHighlightActor;
        pCorralSingleIcon_->setActor(pHighlightedActor_);
    }
}

bool MachInGameScreen::addPromptTextMachineInfo(MachActor* pActor, GuiString& prompt)
{
    bool processed = false;
    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

    if (pActor->objectIsMachine() && pActor->race() == playerRace)
    {
        MachLogMachine& machine = pActor->asMachine();
        char buffer[20];

        // Get HP and Armour info
        //      GuiResourceString hpText( IDS_HEALTHPOINTS, GuiString( itoa( machine.hp(), buffer, 10 ) ) );
        //      GuiResourceString apText( IDS_ARMOURPOINTS, GuiString( itoa( machine.armour(), buffer, 10 ) ) );
        sprintf(buffer, "%d", machine.hp());
        GuiResourceString hpText(IDS_HEALTHPOINTS, GuiString(buffer));
        sprintf(buffer, "%d", machine.armour());
        GuiResourceString apText(IDS_ARMOURPOINTS, GuiString(buffer));

        prompt += "\n" + hpText.asString() + " " + apText.asString();

        // For technicians add on how clever they are onto the prompt text
        if (pActor->objectType() == MachLog::TECHNICIAN)
        {
            //          GuiResourceString rpText( IDS_RESEARCHPOINTS, GuiString( itoa(
            //          pActor->asTechnician().data().researchRate(), buffer, 10 ) ) );
            sprintf(buffer, "%d", pActor->asTechnician().data().researchRate());
            GuiResourceString rpText(IDS_RESEARCHPOINTS, GuiString(buffer));
            prompt += " " + rpText.asString();
        }
        // Resource carrier info
        else if (
            pActor->objectType() == MachLog::RESOURCE_CARRIER && pActor->asResourceCarrier().isNormalResourceCarrier())
        {
            char buffer2[20];
            //          itoa( pActor->asResourceCarrier().data().capacity(), buffer, 10 );
            //          itoa( pActor->asResourceCarrier().amountCarried(), buffer2, 10 );
            sprintf(buffer, "%d", pActor->asResourceCarrier().data().capacity());
            sprintf(buffer2, "%d", pActor->asResourceCarrier().amountCarried());

            GuiResourceString bmuText(IDS_BMUPOINTS, buffer2);
            prompt += " " + bmuText.asString();
            prompt += "/";
            prompt += buffer;
        }
        // For APC add on contents
        else if (pActor->objectType() == MachLog::APC)
        {
            GuiResourceString apcStartText(IDS_APCINFO_START);
            GuiResourceString apcEndText(IDS_APCINFO_END);
            GuiResourceString administratorText(IDS_APCINFO_ADMINISTRATOR);
            GuiResourceString aggressorText(IDS_APCINFO_AGGRESSOR);
            GuiResourceString civilianText(IDS_APCINFO_CONSTRUCTOR);
            GuiResourceString spaceText(IDS_APCINFO_SPACE);

            bool first = true;
            int count = 0;
            MachLogAPC& apc = pActor->asAPC();

            for (MachLogAPC::Machines::const_iterator iter = apc.machines().begin();
                 iter != apc.machines().end() && count < 10;
                 ++iter)
            {
                if (first)
                {
                    prompt += " " + apcStartText.asString();
                    first = false;
                }

                switch ((*iter)->objectType())
                {
                    case MachLog::ADMINISTRATOR:
                        prompt += administratorText.asString();
                        break;
                    case MachLog::AGGRESSOR:
                        prompt += aggressorText.asString();
                        break;
                    case MachLog::APC:
                        ASSERT_FAIL("APCs are not allowed inside APCs");
                        break;
                    default:
                        prompt += civilianText.asString();
                        break;
                }

                ++count;
                // If more than 10 machines in APC then end with ... to indicate more machines.
                if (count == 10 && count != apc.machines().size())
                {
                    prompt += spaceText.asString();
                }
            }

            if (! first)
            {
                prompt += apcEndText.asString();
            }
        }

        // Add on a string describing the machines current operation
        if (! pActor->isIdle())
        {
            GuiResourceString opString(
                9000 /*IDS_xxx values for ops start at 9000*/ + pActor->strategy().currentOperationType());
            prompt += " " + opString.asString();
        }

        processed = true;
    }
    return processed;
}

bool MachInGameScreen::addPromptTextConstructionInfo(MachActor* pActor, GuiString& prompt)
{
    bool processed = false;
    char buffer[20];
    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

    // Display incomplete construction info
    if (pActor->objectIsConstruction() && ! pActor->asConstruction().isComplete() && pActor->race() == playerRace)
    {
        // Special case for incomplete mines, they still show number of BMUs in mine.
        // Add BMUs left in mine onto prompt text
        if (pActor->objectType() == MachLog::MINE)
        {
            MachPhys::BuildingMaterialUnits bmusLeft = 0;

            if (pActor->asMine().hasMineralSite())
            {
                bmusLeft = _CONST_CAST(const MachLogMine&, pActor->asMine()).mineralSite().amountOfOre();
            }

            // Add BMU info
            //          GuiResourceString bmuText( IDS_BMUPOINTS, GuiString( itoa( bmusLeft, buffer, 10 ) ) );
            sprintf(buffer, "%d", bmusLeft);
            GuiResourceString bmuText(IDS_BMUPOINTS, GuiString(buffer));
            prompt += " " + bmuText.asString();
        }

        // Percentage complete info
        //      GuiResourceString percentCompleteText( IDS_PERCENTAGECOMPLETE, GuiString( itoa(
        //      pActor->asConstruction().percentageComplete(), buffer, 10 ) ) );
        sprintf(buffer, "%d", pActor->asConstruction().percentageComplete());
        GuiResourceString percentCompleteText(IDS_PERCENTAGECOMPLETE, GuiString(buffer));
        prompt += ", " + percentCompleteText.asString();

        processed = true;
    }
    // Display HP + AP for fristd::endly constructions
    else if (pActor->objectIsConstruction() && pActor->race() == playerRace)
    {
        // Get HP and Armour info
        //      GuiResourceString hpText( IDS_HEALTHPOINTS, GuiString( itoa( pActor->hp(), buffer, 10 ) ) );
        //      GuiResourceString apText( IDS_ARMOURPOINTS, GuiString( itoa( pActor->armour(), buffer, 10 ) ) );
        sprintf(buffer, "%d", pActor->hp());
        GuiResourceString hpText(IDS_HEALTHPOINTS, GuiString(buffer));
        sprintf(buffer, "%d", pActor->armour());
        GuiResourceString apText(IDS_ARMOURPOINTS, GuiString(buffer));

        prompt += "\n" + hpText.asString() + " " + apText.asString();

        // Add BMU's left in mine onto prompt text
        if (pActor->objectType() == MachLog::MINE)
        {
            MachPhys::BuildingMaterialUnits bmusLeft = 0;

            if (pActor->asMine().hasMineralSite())
            {
                bmusLeft = _CONST_CAST(const MachLogMine&, pActor->asMine()).mineralSite().amountOfOre();
            }

            // Add BMU info
            //          GuiResourceString bmuText( IDS_BMUPOINTS, GuiString( itoa( bmusLeft, buffer, 10 ) ) );
            //          GuiResourceString bmuMinedText( IDS_BMUPOINTSMINED, GuiString( itoa( pActor->asMine().ore(),
            //          buffer, 10 ) ) );
            sprintf(buffer, "%d", bmusLeft);
            GuiResourceString bmuText(IDS_BMUPOINTS, GuiString(buffer));
            sprintf(buffer, "%d", pActor->asMine().ore());
            GuiResourceString bmuMinedText(IDS_BMUPOINTSMINED, GuiString(buffer));
            prompt += " " + bmuText.asString();
            prompt += " " + bmuMinedText.asString();
        }
        // Add Research Rate info for Hardware Labs
        else if (pActor->objectType() == MachLog::HARDWARE_LAB)
        {
            // Add Research Rate info
            //          GuiResourceString rpText( IDS_RESEARCHPOINTS, GuiString( itoa(
            //          pActor->asHardwareLab().totalResearchRate(), buffer, 10 ) ) );
            sprintf(buffer, "%.0f", pActor->asHardwareLab().totalResearchRate());
            GuiResourceString rpText(IDS_RESEARCHPOINTS, GuiString(buffer));
            prompt += " " + rpText.asString();
        }

        // Add on a string describing the current operation
        if (! pActor->isIdle())
        {
            GuiResourceString opString(
                9000 /*IDS_xxx values for ops start at 9000*/ + pActor->strategy().currentOperationType());
            prompt += " " + opString.asString();
        }

        processed = true;
    }

    return processed;
}

bool MachInGameScreen::addPromptTextArtefactInfo(MachActor* pActor, GuiString& prompt)
{
    bool processed = false;

    if (pActor->objectIsArtefact())
    {
        char buffer[20];

        // Get HP and Armour info
        //      GuiResourceString hpText( IDS_HEALTHPOINTS, GuiString( itoa( pActor->hp(), buffer, 10 ) ) );
        //      GuiResourceString apText( IDS_ARMOURPOINTS, GuiString( itoa( pActor->armour(), buffer, 10 ) ) );
        sprintf(buffer, "%d", pActor->hp());
        GuiResourceString hpText(IDS_HEALTHPOINTS, GuiString(buffer));
        sprintf(buffer, "%d", pActor->armour());
        GuiResourceString apText(IDS_ARMOURPOINTS, GuiString(buffer));
        prompt += " " + hpText.asString() + " " + apText.asString();

        processed = true;
    }

    return processed;
}

bool MachInGameScreen::addPromptTextDebrisInfo(MachActor* pActor, GuiString& prompt)
{
    bool processed = false;

    if (pActor->objectIsDebris())
    {
        char buffer[20];

        // Display BMU value for Debris
        //      GuiResourceString bmuText( IDS_BMUPOINTS, GuiString( itoa( pActor->asDebris().quantity(), buffer, 10 ) )
        //      );
        sprintf(buffer, "%d", pActor->asDebris().quantity());
        GuiResourceString bmuText(IDS_BMUPOINTS, GuiString(buffer));
        prompt += " " + bmuText.asString();

        processed = true;
    }

    return processed;
}

bool MachInGameScreen::addPromptTextOreHolographInfo(MachActor* pActor, GuiString& prompt)
{
    bool processed = false;

    if (pActor->objectIsOreHolograph())
    {
        char buffer[20];

        // BMU's in mineral site
        //      GuiResourceString bmuText( IDS_BMUPOINTS, GuiString( itoa(
        //      pActor->asOreHolograph().mineralSite().amountOfOre(), buffer, 10 ) ) );
        sprintf(buffer, "%d", pActor->asOreHolograph().mineralSite().amountOfOre());
        GuiResourceString bmuText(IDS_BMUPOINTS, GuiString(buffer));

        prompt += " " + bmuText.asString();

        processed = true;
    }

    return processed;
}

void MachInGameScreen::displayActorPromptText(MachActor* pActor)
{
    CB_DEPIMPL_AUTO(pPromptTextActor_);

    GuiResourceString::Id stringId = 0;
    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();
    GuiString weaponName;

    if (pActor->race() == playerRace && MachLogActorStringIdRestorer::isExceptionToRule(pActor, &stringId))
    {
    }
    else
    {
        stringId = MachLogActorStringIdRestorer::stringId(pActor);

        // Get weapon name, if any ( must belong to player to see weapon info ).
        if (pActor->objectIsCanAttack() && pActor->race() == playerRace)
        {
            // If actor is POD then we must check that the ion cannon is active.
            if ((pActor->objectType() == MachLog::POD && pActor->asPod().hasIonCannon())
                || pActor->objectType() != MachLog::POD)
            {
                MachLogCanAttack& canAttack = pActor->asCanAttack();
                GuiResourceString weapon(MachLogActorStringIdRestorer::weaponStringId(canAttack.weaponCombo()));
                weaponName = weapon.asString();
            }
        }
    }

    // Load the resource string
    GuiResourceString name(stringId);

    GuiString prompt;

    if (weaponName.length() == 0)
    {
        GuiResourceString text(IDS_HIGHLIGHT_PROMPT, name.asString());
        prompt = text.asString();
    }
    else
    {
        GuiStrings strings;
        strings.reserve(2);
        strings.push_back(name.asString());
        strings.push_back(weaponName);
        GuiResourceString text(IDS_HIGHLIGHT_WITH_WEAPON_PROMPT, strings);
        prompt = text.asString();
    }

    // Add aditional prompt text info based on actor type
    if (addPromptTextMachineInfo(pActor, prompt))
    {
    }
    else if (addPromptTextConstructionInfo(pActor, prompt))
    {
    }
    else if (addPromptTextArtefactInfo(pActor, prompt))
    {
    }
    else if (addPromptTextDebrisInfo(pActor, prompt))
    {
    }
    else if (addPromptTextOreHolographInfo(pActor, prompt))
    {
    }

    // Set the cursor prompt
    setCursorPromptText(prompt, !(pPromptTextActor_ == pActor));

    pPromptTextActor_ = pActor;
}

void MachInGameScreen::removeHighlightedActor()
{
    CB_DEPIMPL_AUTO(pHighlightedActor_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);

    if (pHighlightedActor_)
    {
        pHighlightedActor_ = nullptr;
        pCorralSingleIcon_->clear();
    }
}

void MachInGameScreen::asynchronousUpdate()
{
    CB_DEPIMPL_AUTO(pPromptText_);
    CB_DEPIMPL_AUTO(pControlPanelAddOn_);
    CB_DEPIMPL_AUTO(pWorldViewWindow_);
    CB_DEPIMPL_AUTO(cancelActiveCommand_);
    CB_DEPIMPL_AUTO(commandBankNeedsUpdating_);
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(allCommands_);
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(pProductionBank_);
    CB_DEPIMPL_AUTO(pHWResearchBank_);
    CB_DEPIMPL_AUTO(pMachinesIcon_);
    CB_DEPIMPL_AUTO(pConstructionsIcon_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pActiveCommand_);
    CB_DEPIMPL_AUTO(controlPanelOn_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);
    CB_DEPIMPL_AUTO(pControlPanel_);
    CB_DEPIMPL_AUTO(pMapArea_);
    CB_DEPIMPL_AUTO(redrawMapCounter_);
    CB_DEPIMPL_AUTO(networkStuffedStartTime_);

#ifndef PRODUCTION
    CB_DEPIMPL_AUTO(showCurrentMachine_);
    CB_DEPIMPL_AUTO(showNetworkPing_);
    CB_DEPIMPL_AUTO(showNetworkStuffed_);
#endif

    if (actualGameState() != PLAYING)
    {
        pSceneManager_->pDevice()->display()->useCursor(
            nullptr); // Shouldn't really use NULL but there's no other interface
        SimManager::instance().suspend();
    }

    // If we have been requested to cancel the active command, do so now
    if (cancelActiveCommand_)
        activateDefaultCommand();

    // If we have a non-default active command, and its interaction is complete,
    // execute it and restore default command.
    if (pActiveCommand_ != nullptr && pActiveCommand_->isInteractionComplete())
    {
        // If not done so, execute it
        if (! pActiveCommand_->isExecuted())
        {
            pActiveCommand_->apply();
            commandBankNeedsUpdating_ = true;
        }

        activateDefaultCommand();
    }

    // If required, update the command bank
    if (commandBankNeedsUpdating_)
    {
        // Ensure don't do it again next time
        commandBankNeedsUpdating_ = false;

        // Run along the commands, setting the visibility flag.
        // However, if no selections, or first is foreign race, then no command should
        // be visible.
        MachLogRaces& races = MachLogRaces::instance();
        bool forceInvisible = selectedActors_.size() == 0 || ! races.hasPCRace()
            || selectedActors_.front()->race() != races.playerRace();

        for (const auto& pCommand : allCommands_)
        {
            pCommand->isVisible(forceInvisible ? false : pCommand->actorsCanExecute());
        }

        // Update the command bank if displayed
        updateCommandIcons();

        // Display command text for single factory and research contexts
        if (controlPanelContext_ == MachGui::SINGLE_FACTORY)
        {
            GuiResourceString promptString(IDS_BUILD_START);
            commandPromptText(promptString.asString());
        }
        else if (controlPanelContext_ == MachGui::HARDWARE_RESEARCH)
        {
            GuiResourceString promptString(IDS_RESEARCHLEVEL_START);
            commandPromptText(promptString.asString());
        }
    }

    // If we have a production bank, update it
    if (pProductionBank_ != nullptr)
        pProductionBank_->updateProgress();

    // If we have a hw research menu, update it
    if (pHWResearchBank_ != nullptr)
        pHWResearchBank_->updateProgress();

    // Update some things only every 20 frames
    static size_t frameCounter = 19;
    frameCounter = (++frameCounter) % 20;
    switch (frameCounter)
    {
        case 0:
            MachLogVoiceMailManager::instance().update();
            break;
        case 3:
            pMachinesIcon_->refresh();
            break;
        case 7:
            pConstructionsIcon_->refresh();
            break;
        case 11:
            pMachineNavigation_->update();
            break;
        case 15:
            pConstructionNavigation_->update();
            break;
        case 19:
            updateGameState();
            break;
    }

    // Refresh the continent map. This is not done every frame. When it is redrawn is
    // controlled internally by the continent map.
    if (redrawMapCounter_ != 0)
    {
        pContinentMap_->forceUpdate();
        --redrawMapCounter_;
    }
    else
    {
        pContinentMap_->refresh();
    }

    // Make sure new gui sounds are played
    MachGuiSoundManager::instance().update();

    // Update time network has been stuffed for
    if (MachLogNetwork::instance().isNetworkGame())
    {
        if (! NetNetwork::instance().imStuffed())
        {
            networkStuffedStartTime_ = DevTime::instance().time();
        }
    }

#ifndef PRODUCTION
    if (showNetworkStuffed_)
    {
        NetNetwork::instance().outputCurrentStatistics(pSceneManager_->out());
        if (NetNetwork::instance().imStuffed())
            NetNetwork::instance().outputStuffedInfo(pSceneManager_->out());
    }
    if (showCurrentMachine_ && selectedActors().size() > 0)
        pSceneManager_->out() << *selectedActors().front() << std::endl;
    if (showNetworkPing_ && MachLogNetwork::instance().isNetworkGame())
    {
        const NetNode& node = MachLogNetwork::instance().node();
        /*      const NetNode::Ping& pingInfo = node.pingInformation();
        for( NetNode::Ping::const_iterator i = pingInfo.begin(); i != pingInfo.end(); ++i )
        {
            if( (*i).first != node.nodeUid() )
            {
                pSceneManager_->out() << "Current ping : " << (*i).first.nodeName() << " " << (*i).second << std::endl;
            }
        }*/
    }
#endif
}

// virtual
void MachInGameScreen::update()
{
    CB_DEPIMPL_AUTO(pPromptText_);
    CB_DEPIMPL_AUTO(pControlPanelAddOn_);
    CB_DEPIMPL_AUTO(pWorldViewWindow_);
    CB_DEPIMPL_AUTO(cancelActiveCommand_);
    CB_DEPIMPL_AUTO(commandBankNeedsUpdating_);
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(allCommands_);
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(pProductionBank_);
    CB_DEPIMPL_AUTO(pHWResearchBank_);
    CB_DEPIMPL_AUTO(pMachinesIcon_);
    CB_DEPIMPL_AUTO(pConstructionsIcon_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pActiveCommand_);
    CB_DEPIMPL_AUTO(controlPanelOn_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);
    CB_DEPIMPL_AUTO(pControlPanel_);
    CB_DEPIMPL_AUTO(pMapArea_);
    CB_DEPIMPL_AUTO(redrawMapCounter_);
    CB_DEPIMPL_AUTO(networkStuffedStartTime_);

#ifndef PRODUCTION
    CB_DEPIMPL_AUTO(showCurrentMachine_);
    CB_DEPIMPL_AUTO(showNetworkPing_);
    CB_DEPIMPL_AUTO(showNetworkStuffed_);
#endif

    bool controlPanelMoved = false;
    // Scroll control panel...
    if ((controlPanelOn_ || displayControlPanel()) && controlPanelXPos_ != MachGui::controlPanelOutXPos())
    {
        // Trigger sound if control panel about to slide out
        if (controlPanelXPos_ == MachGui::controlPanelInXPos())
            MachGuiSoundManager::instance().playSound("gui/sounds/ctrlpon.wav");

        controlPanelXPos_ += MachGui::controlPanelSlideOutSpeed();
        controlPanelXPos_ = std::min(controlPanelXPos_, MachGui::controlPanelOutXPos());

        controlPanelMoved = true;
    }
    else if (! controlPanelOn_ && ! displayControlPanel() && controlPanelXPos_ != MachGui::controlPanelInXPos())
    {
        // Trigger sound if control panel about to slide in
        if (controlPanelXPos_ == MachGui::controlPanelOutXPos())
            MachGuiSoundManager::instance().playSound("gui/sounds/ctrlpoff.wav");

        controlPanelXPos_ -= MachGui::controlPanelSlideInSpeed();
        controlPanelXPos_ = std::max(controlPanelXPos_, MachGui::controlPanelInXPos());

        controlPanelMoved = true;
    }

    //  if ( controlPanelMoved ) //TODO: now panel is redrawn every frame
    {
        // Move control panel
        Gui::Coord newTopLeft = controlPanel().relativeCoord();
        newTopLeft.x(controlPanelXPos_ - MachGui::controlPanelOutXPos());
        positionChildRelative(pControlPanel_, newTopLeft);
        pControlPanel_->changed();

        // Move prompt text
        Gui::Coord promptNewTopLeft = pPromptText_->relativeCoord();
        promptNewTopLeft.x(controlPanelXPos_ + MachGui::promptTextXOffset());
        positionChildRelative(pPromptText_, promptNewTopLeft);

        // Move WorldViewWindow
        positionChildRelative(pWorldViewWindow_, Gui::Coord(controlPanelXPos_, 0));
    }

    // Tell map area to update every frame
    pMapArea_->controlPanelSliding(controlPanelXPos_ != MachGui::controlPanelOutXPos());
    pMapArea_->changed(); // Force redraw of map area

    // World view window is responsible for drawing rubber band when
    // selecting large numbers of actors.
    pWorldViewWindow_->update();

    // Update the visibility of any child GuiDisplayables
    updateChildVisible();

    // Any control panel sections that have been invalidated will be
    // redrawn now before child GuiDisplayables are draw.
    pControlPanel_->redrawAllAreas();
}

MachCameras* MachInGameScreen::cameras()
{
    CB_DEPIMPL_AUTO(pCameras_);

    PRE(pCameras_);

    return pCameras_.get();
}

// virtual
bool MachInGameScreen::beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData)
{
    CB_DEPIMPL_AUTO(commandBankNeedsUpdating_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);
    CB_DEPIMPL_AUTO(pCorral_);

    bool cancelSelection = false;

    // We want to deselect the actor if being deleted, or entering an APC
    switch (event)
    {
        case W4dSubject::DELETED:
            cancelSelection = true;
            break;

        case W4dSubject::CLIENT_SPECIFIC:
            if (clientData == MachLog::TELEPORTED_OUT_OF_WORLD || clientData == MachLog::ENTERED_WORLD
                || clientData == MachLog::RACE_CHANGED || clientData == MachLog::CHANGED_CAMOUFLAGE_STATUS
                || clientData == MachLog::CHANGED_MINERALS_CARRIED)
            {
                // Need to grey out or show the icon in the corral when leaving/entering APC
                pCorralSingleIcon_->setActor(selectedActors().front(), true);
                pCorral_->changed();
                commandBankNeedsUpdating_ = true;
            }
            else if (clientData == MachLog::WEAPON_AVAILABILITY_CHANGED)
            {
                commandBankNeedsUpdating_ = true;
            }
            break;
    }

    // Cancel the selection if required
    if (cancelSelection)
    {
        // Since we only observe MachActors, this cast, although unfortunate,
        // should be safe.
        MachActor* pActor = _STATIC_CAST(MachActor*, pSubject);

        // Stop the navigator from being dismissed just because we are unselecting
        // a deleted actor
        if (isMachineNavigationContext())
            pMachineNavigation_->remainVisible(true);

        if (isConstructionNavigationContext())
            pConstructionNavigation_->remainVisible(true);

        // Do the work, but don't cancel the observer relation - let caller do that
        unselect(pActor);
    }

    return ! cancelSelection;
}

bool MachInGameScreen::applyCommandToSquadron()
{
    CB_DEPIMPL_AUTO(applyCommandToSquadron_);

    return applyCommandToSquadron_;
}

void MachInGameScreen::applyCommandToSquadron(bool apply)
{
    CB_DEPIMPL_AUTO(applyCommandToSquadron_);

    applyCommandToSquadron_ = apply;
}

// virtual
void MachInGameScreen::domainDeleted(W4dDomain*)
{
    // Do nothing
}

// virtual
bool MachInGameScreen::doHandleRightClickEvent(const GuiMouseEvent& event)
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(Gui::Coord, rightClickMousePos_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(pContinentMap_);

    static double pressReleaseTimer;

    if (event.rightButton() == Gui::PRESSED)
    {
        pressReleaseTimer = DevTime::instance().time();
        rightClickMousePos_ = event.coord();
    }
    else
    {
        bool shouldDeselectActors = false;

        // If we release the right mouse button in under 0.5sec and we're not
        // over the map then deselect all selected actors, reset the intelligent
        // cursors etc.
        if (DevTime::instance().time() - pressReleaseTimer < 0.5
            && ! pContinentMap_->absoluteBoundary().contains(event.coord()))
        {
            shouldDeselectActors = true;
        }

        if (shouldDeselectActors)
        {
            // First check if constructor menu is displayed. If so, dismiss this instead of
            // cleaning out corral
            if (isConstructCommandContext())
            {
                mainMenuContext();
                activateDefaultCommand();
            }
            else
            {
                // Navigators are not dismissed via a right click.
                pMachineNavigation_->remainVisible(true);
                pConstructionNavigation_->remainVisible(true);
                // Remove green selection from around any nav buttons.
                pMachineNavigation_->deselectAll();
                pConstructionNavigation_->deselectAll();

                deselectAll();

                // Revert to default intelligent command
                activateDefaultCommand();
            }
        }
    }

    return true;
}

bool MachInGameScreen::isMainMenuContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::MAIN_MENU;
}

bool MachInGameScreen::isSquadronContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::SQUADRON_MENU;
}

bool MachInGameScreen::isConstructCommandContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::CONSTRUCT_COMMAND;
}

bool MachInGameScreen::isBuildCommandContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::BUILD_COMMAND;
}

bool MachInGameScreen::isHardwareResearchContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::HARDWARE_RESEARCH;
}

bool MachInGameScreen::isSoftwareResearchContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::SOFTWARE_RESEARCH;
}

bool MachInGameScreen::isSingleFactoryContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::SINGLE_FACTORY;
}

void MachInGameScreen::mainMenuContext()
{
    currentContext(MachGui::MAIN_MENU);
}

void MachInGameScreen::squadronContext()
{
    currentContext(MachGui::SQUADRON_MENU);
}

void MachInGameScreen::constructCommandContext()
{
    currentContext(MachGui::CONSTRUCT_COMMAND);
}

void MachInGameScreen::buildCommandContext()
{
    currentContext(MachGui::BUILD_COMMAND);
}

void MachInGameScreen::hardwareResearchContext()
{
    currentContext(MachGui::HARDWARE_RESEARCH);
}

void MachInGameScreen::softwareResearchContext()
{
    currentContext(MachGui::SOFTWARE_RESEARCH);
}

void MachInGameScreen::singleFactoryContext()
{
    currentContext(MachGui::SINGLE_FACTORY);
}

MachGui::ControlPanelContext MachInGameScreen::currentContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_;
}

void MachInGameScreen::mainMenuOrSingleFactoryContext()
{
    // Check to see if we have one selected factory. If we do then we switch straight into
    // build menu.
    const MachInGameScreen::Actors& selectionSet = selectedActors();
    MachLogRaces& races = MachLogRaces::instance();
    if (selectionSet.size() == 1 && // Only consider when one actor selected
        races.hasPCRace() && // PC race must exist
        selectionSet.front()->race() == races.playerRace()) // factory must belong to PC race
    {
        if (selectionSet.front()->objectType() == MachLog::FACTORY) // actor is factory
        {
            singleFactoryContext();
        }
        else if (selectionSet.front()->objectType() == MachLog::HARDWARE_LAB)
        {
            hardwareResearchContext();
        }
        else
        {
            mainMenuContext();
        }
    }
    else
    {
        mainMenuContext();
    }
}

void MachInGameScreen::currentContext(MachGui::ControlPanelContext newContext, bool forceChange /*= false*/)
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(commandBankNeedsUpdating_);
    CB_DEPIMPL_AUTO(pMachinesIcon_);
    CB_DEPIMPL_AUTO(pConstructionsIcon_);
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pConstructMenu_);
    CB_DEPIMPL_AUTO(pBuildMenu_);
    CB_DEPIMPL_AUTO(pSmallCommandIcons_);
    CB_DEPIMPL_AUTO(pProductionBank_);
    CB_DEPIMPL_AUTO(pHWResearchBank_);
    CB_DEPIMPL_AUTO(pHWResearchMenu_);
    CB_DEPIMPL_AUTO(pControlPanel_);

    // Check not already in it
    if (controlPanelContext_ != newContext || forceChange)
    {
        // Reset all other contexts
        MachGui::ControlPanelContext oldContext = controlPanelContext_;
        resetContext();
        controlPanelContext_ = newContext;
        commandBankNeedsUpdating_ = true;
        setupActorBank();

        // Make navigator buttons popup if we are switching to a different context
        if (oldContext == MachGui::MACHINE_NAVIGATION_MENU)
            pMachinesIcon_->forceUp();
        else if (oldContext == MachGui::CONSTRUCTION_NAVIGATION_MENU)
            pConstructionsIcon_->forceUp();

        // Do any context specific menu creations etc
        switch (newContext)
        {
            case MachGui::CONSTRUCT_COMMAND:
                {
                    // Construct the construction selection menu
                    pConstructMenu_ = new MachConstructMenu(pControlPanel_, Gui::Coord(1, 0), this);
                    pConstructMenu_->initialise();
                    break;
                }

            case MachGui::SINGLE_FACTORY:
                {
                    // Construct the build menu
                    Gui::Coord sciCoord = pSmallCommandIcons_->relativeBoundary(*pControlPanel_).maxCorner();

                    size_t yPos = sciCoord.y() + 2;

                    pBuildMenu_ = new MachBuildMenu(pControlPanel_, pProductionBank_, Gui::Coord(1, yPos), this);
                    pBuildMenu_->initialise();

                    GuiResourceString promptString(IDS_BUILD_START);
                    commandPromptText(promptString.asString());
                    clearCursorPromptText();

                    break;
                }

            case MachGui::HARDWARE_RESEARCH:
                {
                    // Construct the hw research menu
                    Gui::Coord sciCoord = pSmallCommandIcons_->relativeBoundary(*pControlPanel_).maxCorner();

                    size_t yPos = sciCoord.y() + 2;

                    pHWResearchMenu_
                        = new MachHWResearchMenu(pControlPanel_, pHWResearchBank_, Gui::Coord(1, yPos), this);
                    pHWResearchMenu_->initialise();

                    GuiResourceString promptString(IDS_RESEARCHLEVEL_START);
                    commandPromptText(promptString.asString());
                    clearCursorPromptText();

                    break;
                }
        }
    }
}

void MachInGameScreen::resetContext()
{
    CB_DEPIMPL_AUTO(pConstructMenu_);
    CB_DEPIMPL_AUTO(pBuildMenu_);
    CB_DEPIMPL_AUTO(pHWResearchMenu_);
    CB_DEPIMPL_AUTO(pControlPanel_);

    DEBUG_STREAM(DIAG_NEIL, "MachInGameScreen::resetContext");

    // Delete temporary menus
    if (pConstructMenu_ != nullptr)
    {
        pControlPanel_->redrawArea(*pConstructMenu_);
        delete pConstructMenu_;
        pConstructMenu_ = nullptr;
    }

    if (pBuildMenu_ != nullptr)
    {
        pControlPanel_->redrawArea(*pBuildMenu_);
        delete pBuildMenu_;
        pBuildMenu_ = nullptr;
    }

    if (pHWResearchMenu_ != nullptr)
    {
        pControlPanel_->redrawArea(*pHWResearchMenu_);
        delete pHWResearchMenu_;
        pHWResearchMenu_ = nullptr;
    }
}

void MachInGameScreen::updateCommandIcons()
{
    CB_DEPIMPL_AUTO(pCommandIcons_);
    CB_DEPIMPL_AUTO(pSmallCommandIcons_);
    CB_DEPIMPL_AUTO(selectedActors_);

    if (pCommandIcons_ != nullptr)
        pCommandIcons_->change();

    if (pSmallCommandIcons_ != nullptr)
        pSmallCommandIcons_->change();

    // Update special command icons
    defconCommand()->update(selectedActors_);
    MachGuiSelfDestructCommand::update(selectedActors_);
    MachGuiIonAttackCommand::update(selectedActors_);
    MachGuiNukeAttackCommand::update(selectedActors_);
}

bool MachInGameScreen::isCorralVisible() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    bool visible = controlPanelContext_ == MachGui::MAIN_MENU || controlPanelContext_ == MachGui::SQUADRON_MENU
        || controlPanelContext_ == MachGui::FORM_SQUADRON_COMMAND
        || controlPanelContext_ == MachGui::MACHINE_NAVIGATION_MENU
        || controlPanelContext_ == MachGui::CONSTRUCTION_NAVIGATION_MENU;

    // Only visible if there is more than one actor selected
    visible &= (selectedActors_.size() > 1);

    // Only visible if control panel is not in the process of sliding in/out
    visible &= controlPanelXPos_ == MachGui::controlPanelOutXPos();

    return visible;
}

bool MachInGameScreen::isCorralSingleIconVisible() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(pHighlightedActor_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    bool visible = controlPanelContext_ == MachGui::MAIN_MENU || controlPanelContext_ == MachGui::SQUADRON_MENU
        || controlPanelContext_ == MachGui::FORM_SQUADRON_COMMAND
        || controlPanelContext_ == MachGui::MACHINE_NAVIGATION_MENU
        || controlPanelContext_ == MachGui::CONSTRUCTION_NAVIGATION_MENU
        || controlPanelContext_ == MachGui::SINGLE_FACTORY || controlPanelContext_ == MachGui::HARDWARE_RESEARCH;

    visible &= (selectedActors_.size() == 1 || (pHighlightedActor_ && selectedActors_.empty()));

    // Only visible if control panel is not in the process of sliding in/out
    visible &= controlPanelXPos_ == MachGui::controlPanelOutXPos();

    return visible;
}

bool MachInGameScreen::isCommandIconsVisible() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    bool visible = controlPanelContext_ == MachGui::MAIN_MENU;

    // Only visible if control panel is not in the process of sliding in/out
    visible &= controlPanelXPos_ == MachGui::controlPanelOutXPos();

    return visible;
}

bool MachInGameScreen::isSmallCommandIconsVisible() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    bool visible
        = (controlPanelContext_ == MachGui::SINGLE_FACTORY || controlPanelContext_ == MachGui::HARDWARE_RESEARCH);

    // Only visible if control panel is not in the process of sliding in/out
    visible &= controlPanelXPos_ == MachGui::controlPanelOutXPos();

    return visible;
}

bool MachInGameScreen::isMachineNavigationVisible() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    bool visible = controlPanelContext_ == MachGui::MACHINE_NAVIGATION_MENU;

    // Only visible if control panel is not in the process of sliding in/out
    visible &= controlPanelXPos_ == MachGui::controlPanelOutXPos();

    return visible;
}

bool MachInGameScreen::isConstructionNavigationVisible() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    bool visible = controlPanelContext_ == MachGui::CONSTRUCTION_NAVIGATION_MENU;

    // Only visible if control panel is not in the process of sliding in/out
    visible &= controlPanelXPos_ == MachGui::controlPanelOutXPos();

    return visible;
}

bool MachInGameScreen::isSquadronBankVisible() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    bool visible
        = controlPanelContext_ == MachGui::FORM_SQUADRON_COMMAND || controlPanelContext_ == MachGui::SQUADRON_MENU;

    // Only visible if control panel is not in the process of sliding in/out
    visible &= controlPanelXPos_ == MachGui::controlPanelOutXPos();

    return visible;
}

// virtual
void MachInGameScreen::doDisplay()
{
}

void MachInGameScreen::cancelActiveCommand()
{
    CB_DEPIMPL_AUTO(cancelActiveCommand_);
    cancelActiveCommand_ = true;
}

void MachInGameScreen::setCursorPromptText(const string& prompt)
{
    CB_DEPIMPL_AUTO(pPromptText_);

    PRE(pPromptText_ != nullptr);

    pPromptText_->setCursorPromptText(prompt);
}

void MachInGameScreen::setCursorPromptText(const string& prompt, bool restartScroll)
{
    CB_DEPIMPL_AUTO(pPromptText_);

    PRE(pPromptText_ != nullptr);
    pPromptText_->setCursorPromptText(prompt, restartScroll);
}

const string& MachInGameScreen::cursorPromptText() const
{
    CB_DEPIMPL_AUTO(pPromptText_);

    PRE(pPromptText_ != nullptr);
    return pPromptText_->cursorPromptText();
}

void MachInGameScreen::clearCursorPromptText()
{
    CB_DEPIMPL_AUTO(pPromptText_);

    PRE(pPromptText_ != nullptr);
    pPromptText_->clearCursorPromptText();
}

void MachInGameScreen::commandPromptText(const string& prompt)
{
    CB_DEPIMPL_AUTO(pPromptText_);

    PRE(pPromptText_ != nullptr);
    pPromptText_->setCommandPromptText(prompt);
}

const string& MachInGameScreen::commandPromptText() const
{
    CB_DEPIMPL_AUTO(pPromptText_);

    PRE(pPromptText_ != nullptr);
    return pPromptText_->commandPromptText();
}

void MachInGameScreen::clearCommandPromptText()
{
    CB_DEPIMPL_AUTO(pPromptText_);

    PRE(pPromptText_ != nullptr);
    pPromptText_->clearCommandPromptText();
}

MachPromptText& MachInGameScreen::promptTextWindow()
{
    CB_DEPIMPL_AUTO(pPromptText_);

    PRE(pPromptText_ != nullptr);
    return *pPromptText_;
}

void MachInGameScreen::machineNavigationContext()
{
    currentContext(MachGui::MACHINE_NAVIGATION_MENU);
}

bool MachInGameScreen::isMachineNavigationContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::MACHINE_NAVIGATION_MENU;
}

void MachInGameScreen::constructionNavigationContext()
{
    currentContext(MachGui::CONSTRUCTION_NAVIGATION_MENU);
}

bool MachInGameScreen::isConstructionNavigationContext() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    return controlPanelContext_ == MachGui::CONSTRUCTION_NAVIGATION_MENU;
}

void MachInGameScreen::cursor2d(MachGui::Cursor2dType type)
{
    CB_DEPIMPL_AUTO(pCursors2d_);

    PRE(pCursors2d_ != nullptr);

    if (actualGameState() == PLAYING)
    {
        pCursors2d_->cursorSize(MachInGameCursors2d::LARGECURSORS);
        pCursors2d_->cursor(type);
    }
}

void MachInGameScreen::cursor2d(MachGui::Cursor2dType type, MachInGameCursors2d::CursorSize curSize)
{
    CB_DEPIMPL_AUTO(pCursors2d_);

    PRE(pCursors2d_ != nullptr);

    if (actualGameState() == PLAYING)
    {
        pCursors2d_->cursorSize(curSize);
        pCursors2d_->cursor(type);
    }
}

void MachInGameScreen::setupActorBank()
{
    CB_DEPIMPL_AUTO(controlPanelContext_);
    CB_DEPIMPL_AUTO(selectedActors_);
    CB_DEPIMPL_AUTO(pProductionBank_);
    CB_DEPIMPL_AUTO(pHWResearchBank_);
    CB_DEPIMPL_AUTO(pControlPanel_);

    // Get the fristd::endly race id
    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

    // check for a single selected actor, and get its pointer
    MachActor* pSelectedActor = (selectedActors_.size() == 1 ? selectedActors_.front() : nullptr);
    MachLogFactory* pFactory = nullptr;
    MachLogHardwareLab* pHardwareLab = nullptr;

    // Use flags to indicate whether a given bank should be displayed
    bool showProductionBank = false;
    bool showHWResearchBank = false;

    if (pSelectedActor != nullptr)
    {
        // See if the factory production bank is appropriate.
        // Must have a fristd::endly, complete factory, and be in the main or build command menus
        if (pSelectedActor->objectType() == MachLog::FACTORY)
            pFactory = &pSelectedActor->asFactory();

        showProductionBank = pFactory != nullptr
            && (controlPanelContext_ == MachGui::MAIN_MENU || controlPanelContext_ == MachGui::SINGLE_FACTORY)
            && pFactory->race() == playerRace;

        // See if the hw research bank is appropriate.
        // Must have a fristd::endly, complete hw lab, and be in the main or hw research menus
        if (pSelectedActor->objectType() == MachLog::HARDWARE_LAB)
            pHardwareLab = &pSelectedActor->asHardwareLab();

        showHWResearchBank = pHardwareLab != nullptr
            && (controlPanelContext_ == MachGui::MAIN_MENU || controlPanelContext_ == MachGui::HARDWARE_RESEARCH)
            && pHardwareLab->race() == playerRace;
    }

    // Delete any unwanted banks, also watching for banks for an old actor
    if (pProductionBank_ != nullptr && ((! showProductionBank) || (pFactory != &pProductionBank_->factory())))
    {
        pControlPanel_->redrawArea(*pProductionBank_);
        delete pProductionBank_;
        pProductionBank_ = nullptr;

        DEBUG_STREAM(DIAG_NEIL, "Deleting production bank" << std::endl << std::flush);
    }

    if (pHWResearchBank_ != nullptr
        && ((! showHWResearchBank) || (pHardwareLab != &pHWResearchBank_->hardwareLab())))
    {
        pControlPanel_->redrawArea(*pHWResearchBank_);
        delete pHWResearchBank_;
        pHWResearchBank_ = nullptr;

        DEBUG_STREAM(DIAG_NEIL, "Deleting research bank" << std::endl << std::flush);
    }

    // Construct any required banks
    if (showProductionBank && pProductionBank_ == nullptr)
    {
        Gui::Boundary bankArea(
            1,
            pControlPanel_->getVisibleHeight() - MachProductionBank::height(),
            pControlPanel_->width(),
            pControlPanel_->getVisibleHeight());
        pProductionBank_ = new MachProductionBank(pControlPanel_, bankArea, pFactory, this);

        DEBUG_STREAM(DIAG_NEIL, "Creating production bank" << std::endl << std::flush);
    }
    else if (showHWResearchBank && pHWResearchBank_ == nullptr)
    {
        Gui::Boundary bankArea(
            1,
            pControlPanel_->getVisibleHeight() - MachHWResearchBank::reqHeight(),
            pControlPanel_->width(),
            pControlPanel_->getVisibleHeight());
        pHWResearchBank_ = new MachHWResearchBank(pControlPanel_, bankArea, pHardwareLab, this);

        DEBUG_STREAM(DIAG_NEIL, "Creating research bank" << std::endl << std::flush);
    }
}

void MachInGameScreen::updateCameras()
{
    CB_DEPIMPL_AUTO(pCameras_);
    pCameras_->updateCameras();
}

void MachInGameScreen::updateChildVisible()
{
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(pCorralSingleIcon_);
    CB_DEPIMPL_AUTO(pCorral_);
    CB_DEPIMPL_AUTO(pSquadronBank_);
    CB_DEPIMPL_AUTO(pCommandIcons_);
    CB_DEPIMPL_AUTO(pSmallCommandIcons_);
    CB_DEPIMPL_AUTO(pControlPanel_);
    CB_DEPIMPL_AUTO(pProductionBank_);
    CB_DEPIMPL_AUTO(pHWResearchBank_);
    CB_DEPIMPL_AUTO(pConstructMenu_);
    CB_DEPIMPL_AUTO(pBuildMenu_);
    CB_DEPIMPL_AUTO(pHWResearchMenu_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    // Make navigator visible/invisible
    if (! isMachineNavigationVisible() && pMachineNavigation_->isVisible())
    {
        pMachineNavigation_->makeVisible(false);
        pControlPanel_->redrawArea(*pMachineNavigation_);
    }
    else if (isMachineNavigationVisible() && ! pMachineNavigation_->isVisible())
    {
        pMachineNavigation_->makeVisible(true);
    }

    // Make construction navigation visible/invisble
    if (! isConstructionNavigationVisible() && pConstructionNavigation_->isVisible())
    {
        pConstructionNavigation_->makeVisible(false);
        pControlPanel_->redrawArea(*pConstructionNavigation_);
    }
    else if (isConstructionNavigationVisible() && ! pConstructionNavigation_->isVisible())
    {
        pConstructionNavigation_->makeVisible(true);
    }

    // Make single icon corral visible/invisible
    if (! isCorralSingleIconVisible() && pCorralSingleIcon_->isVisible())
    {
        pCorralSingleIcon_->isVisible(false);
        pControlPanel_->redrawArea(*pCorralSingleIcon_);
    }
    else if (isCorralSingleIconVisible() && ! pCorralSingleIcon_->isVisible())
    {
        pCorralSingleIcon_->isVisible(true);
    }

    // Make multi-icon corral visible/invisible
    if (! isCorralVisible() && pCorral_->isVisible())
    {
        pCorral_->isVisible(false);
        pControlPanel_->redrawArea(*pCorral_);
    }
    else if (isCorralVisible() && ! pCorral_->isVisible())
    {
        pCorral_->isVisible(true);
    }

    // Make squadron bank visible/invisible
    if (! isSquadronBankVisible() && pSquadronBank_->isVisible())
    {
        pSquadronBank_->isVisible(false);
        pControlPanel_->redrawArea(*pSquadronBank_);
    }
    else if (isSquadronBankVisible() && ! pSquadronBank_->isVisible())
    {
        pSquadronBank_->isVisible(true);
    }

    // Make command icons visible/invisible
    if (! isCommandIconsVisible() && pCommandIcons_->isVisible())
    {
        pCommandIcons_->isVisible(false);
        pControlPanel_->redrawArea(*pCommandIcons_);
    }
    else if (isCommandIconsVisible() && ! pCommandIcons_->isVisible())
    {
        pCommandIcons_->isVisible(true);
    }

    // Make small command icons visible/invisible
    if (! isSmallCommandIconsVisible() && pSmallCommandIcons_->isVisible())
    {
        pSmallCommandIcons_->isVisible(false);
        pControlPanel_->redrawArea(*pSmallCommandIcons_);
    }
    else if (isSmallCommandIconsVisible() && ! pSmallCommandIcons_->isVisible())
    {
        pSmallCommandIcons_->isVisible(true);
    }

    // Make various bits and bobs visible/invisible
    // Only visible if control panel is not in the process of sliding in/out
    bool visible = controlPanelXPos_ == MachGui::controlPanelOutXPos();

    // Make production bank visible/invisible
    if (pProductionBank_)
    {
        if (visible != pProductionBank_->isVisible())
        {
            pProductionBank_->isVisible(visible);
        }
    }
    // Make hardware research bank visible/invisible
    if (pHWResearchBank_)
    {
        if (visible != pHWResearchBank_->isVisible())
        {
            pHWResearchBank_->isVisible(visible);
        }
    }
    // Make construction menu visible/invisible
    if (pConstructMenu_)
    {
        if (visible != pConstructMenu_->isVisible())
        {
            pConstructMenu_->isVisible(visible);
        }
    }
    // Make build menu visible/invisible
    if (pBuildMenu_)
    {
        if (visible != pBuildMenu_->isVisible())
        {
            pBuildMenu_->isVisible(visible);
        }
    }
    // Make research menu visible/invisible
    if (pHWResearchMenu_)
    {
        if (visible != pHWResearchMenu_->isVisible())
        {
            pHWResearchMenu_->isVisible(visible);
        }
    }
}

// virtual
void MachInGameScreen::doHandleContainsMouseEvent(const GuiMouseEvent& /*event*/)
{
}

void MachInGameScreen::setGuiViewport()
{
    CB_DEPIMPL_AUTO(pSceneManager_);

    // Set the viewport boundary to the entire screen.
    RenDevice& device = *pSceneManager_->pDevice();
    const int w = device.windowWidth();
    const int h = device.windowHeight();
    device.setViewport(0, 0, w, h);
}

void MachInGameScreen::setWorldViewViewport()
{
    CB_DEPIMPL_AUTO(inFirstPerson_);
    CB_DEPIMPL_AUTO(pFirstPerson_);
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pControlPanel_);

    if (inFirstPerson_)
    {
        pFirstPerson_->setFirstPerson3DViewport();
    }
    else
    {
        // Set the viewport boundary for the world view window
        Gui::Coord topLeftCoord = controlPanel().relativeCoord();

        RenDevice& device = *pSceneManager_->pDevice();
        const int w = device.windowWidth();
        const int h = device.windowHeight();
        device.setViewport(
            pControlPanel_->width() + topLeftCoord.x(),
            0,
            w - (pControlPanel_->width() + topLeftCoord.x()),
            h);
    }
}

bool MachInGameScreen::rubberBandSelectionHappening() const
{
    CB_DEPIMPL_AUTO(pWorldViewWindow_);

    return pWorldViewWindow_->rubberBandSelectionHappening();
}

void MachInGameScreen::loadGame(const string& planet)
{
    CB_DEPIMPL_AUTO(pCameras_);
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pWorldViewWindow_);
    CB_DEPIMPL_AUTO(pSquadronBank_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(pBmuButton_);
    CB_DEPIMPL(MachInGameScreen::GameState, gameState_);
    CB_DEPIMPL_AUTO(pMachinesIcon_);
    CB_DEPIMPL_AUTO(pConstructionsIcon_);
    CB_DEPIMPL_AUTO(pSquadronIcon_);
    CB_DEPIMPL_AUTO(pChatMessageDisplay_);
    CB_DEPIMPL_AUTO(pControlPanel_);

    // Don't allow any of the button setting below to trigger their sounds
    MachGuiSoundManager::instance().delaySounds();

    pCameras_->loadGame();
    pContinentMap_->loadGame(planet);
    pWorldViewWindow_->loadGame();
    pSquadronBank_->loadGame();
    pMachineNavigation_->loadGame();
    pConstructionNavigation_->loadGame();
    pMachinesIcon_->loadGame();
    pConstructionsIcon_->loadGame();
    pSquadronIcon_->loadGame();
    pControlPanel_->setupDecalBitmaps();

    pBmuButton_->setDepressed(true); // button should start depressed.

    // Switch to main menu context.
    mainMenuContext();

    if (MachLogNetwork::instance().isNetworkGame())
    {
        const int chatMessagesX = 202 * Gui::uiScaleFactor();
        const int chatMessagesY = 0 * Gui::uiScaleFactor();

        pChatMessageDisplay_ = new MachGuiInGameChatMessagesDisplay(
            this,
            Gui::Box(
                Gui::Coord(chatMessagesX, chatMessagesY),
                MachGuiInGameChatMessages::reqWidth(),
                MachGuiInGameChatMessages::reqHeight()),
            &worldViewWindow());
    }

    gameState_ = PLAYING;
}

void MachInGameScreen::unloadGame()
{
    CB_DEPIMPL_AUTO(pCameras_);
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pWorldViewWindow_);
    CB_DEPIMPL_AUTO(pSquadronBank_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(inFirstPerson_);
    CB_DEPIMPL_AUTO(pChatMessageDisplay_);

    // Unselect any actors
    deselectAll();

    // Clear any current command. Otherwise we can get crashes -eg the construct command has
    // a physical building in the world which it deletes on destruction, so we must delete it
    // in advance of deleting the world to avoid double delete.
    activateDefaultCommand();

    pWorldViewWindow_->unloadGame();
    pCameras_->unloadGame();
    pContinentMap_->unloadGame();
    pSquadronBank_->unloadGame();
    pMachineNavigation_->unloadGame();
    pConstructionNavigation_->unloadGame();
    inFirstPerson_ = false;

    // Clean up chat message area
    delete pChatMessageDisplay_;
    pChatMessageDisplay_ = nullptr;
}

void MachInGameScreen::updateGameState()
{
    CB_DEPIMPL(MachInGameScreen::GameState, gameState_);
    CB_DEPIMPL_AUTO(gameStateTimer_);

    if (gameState_ == PLAYING) // Only interested in switching the state if the game has not yet been won/lost
    {
        MachPhys::Race playerRace = MachLogRaces::instance().playerRace();
        if (MachLogRaces::instance().hasLost(playerRace))
        {
            // Keep host working on until ctrl is pressed TODO: another elegant solution?
            if (MachLogNetwork::instance().isNetworkGame() && MachLogNetwork::instance().isNodeLogicalHost()
                && ! DevKeyboard::instance().ctrlPressed())
                return;
            gameState_ = LOST;
            gameStateTimer_ = Phys::time();
            W4dSoundManager::instance().stopAll();
        }
        else if (MachLogRaces::instance().hasWon(playerRace))
        {
            gameState_ = WON;
            gameStateTimer_ = Phys::time();
            W4dSoundManager::instance().stopAll();
        }
    }
}

MachInGameScreen::GameState MachInGameScreen::gameState() const
{
    CB_DEPIMPL(MachInGameScreen::GameState, gameState_);
    CB_DEPIMPL_AUTO(gameStateTimer_);

    GameState retVal = PLAYING;

    if (gameState_ != PLAYING)
    {
        PhysAbsoluteTime now = Phys::time();
        // Only return that game has been lost/won after it has been in this state for a few seconds.
        // This enables pod destruction animation time to run and music to play.
        if (MachGuiDatabase::instance().currentScenario().isTrainingScenario() && now - gameStateTimer_ > 5.0)
        {
            retVal = gameState_;
        }
        else if (gameState_ == WON && now - gameStateTimer_ > 19.0)
        {
            retVal = gameState_;
        }
        else if (gameState_ == LOST && now - gameStateTimer_ > 14.0)
        {
            retVal = gameState_;
        }
    }

    return retVal;
}

void MachInGameScreen::activate()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL_AUTO(inFirstPerson_);
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pFirstPerson_);

    if (! inFirstPerson_)
    {
        pContinentMap_->forceUpdate();
        changed();
    }
    else
    {
        pFirstPerson_->activate();
    }
}

void MachInGameScreen::fogOfWarOn(bool fog)
{
    CB_DEPIMPL_AUTO(pContinentMap_);

    PRE(pContinentMap_);

    pContinentMap_->fogOfWarOn(fog);
}

bool MachInGameScreen::switchToMenus() const
{
    CB_DEPIMPL_AUTO(switchToMenus_);
    CB_DEPIMPL_AUTO(pFirstPerson_);

    return switchToMenus_ || pFirstPerson_->switchToMenus();
}

void MachInGameScreen::resetSwitchToMenus()
{
    CB_DEPIMPL_AUTO(switchToMenus_);
    CB_DEPIMPL_AUTO(pFirstPerson_);

    switchToMenus_ = false;
    pFirstPerson_->resetSwitchToMenus();
}

void MachInGameScreen::switchToInHead()
{
    CB_DEPIMPL_AUTO(switchGuiRoot_);

    switchGuiRoot_ = true;
}

void MachInGameScreen::checkSwitchGuiRoot()
{
    CB_DEPIMPL_AUTO(inFirstPerson_);
    CB_DEPIMPL_AUTO(pFirstPerson_);
    CB_DEPIMPL_AUTO(switchGuiRoot_);
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pWorldViewWindow_);

    if (inFirstPerson_)
    {
        // Switch to ingame gui?
        if (pFirstPerson_->switchToInGameRequested())
        {
            inFirstPerson_ = false;
            pFirstPerson_->resetSwitchToInGame();

            // Make inGameScreen Gui root.
            becomeRoot();

            // Give inGameScreen keyboard focus.
            GuiManager::instance().keyboardFocus(this);

            // Redraw all gui elements.
            activate();
        }
    }
    else
    {
        // Switch to first person?
        if (switchGuiRoot_)
        {
            // Reset request flag.
            switchGuiRoot_ = false;

            // Is the selected actor a valid 1st person actor?
            if (pFirstPerson_->okayToSwitchTo1stPerson())
            {
                // Switch Gui root
                pFirstPerson_->becomeRoot();

                // Give 1st person the keyboard focus.
                GuiManager::instance().keyboardFocus(pFirstPerson_.get());

                // Remove the menu cursor.
                pSceneManager_->pDevice()->display()->useCursor(nullptr);

                // Remove blue selection box from highlighted actor before switching to 1st person
                if (pWorldViewWindow_->haveHighlightedActor())
                {
                    if (MachLogRaces::instance().actorExists(pWorldViewWindow_->highlightedActorId()))
                    {
                        MachActor& highlightedActor
                            = MachLogRaces::instance().actor(pWorldViewWindow_->highlightedActorId());

                        if (highlightedActor.selectionState() == MachLog::HIGHLIGHTED)
                        {
                            highlightedActor.selectionState(MachLog::NOT_SELECTED);
                        }
                    }
                }

                inFirstPerson_ = true;
            }
        }
    }
}

void MachInGameScreen::switchBackToInGame()
{
    CB_DEPIMPL_AUTO(inFirstPerson_);
    CB_DEPIMPL_AUTO(pFirstPerson_);
    CB_DEPIMPL_AUTO(pSceneManager_);

    // Were we in first person? Is it still valid to switch to first person?
    // In a multiplayer game the game continues whilst the user is in the menu
    // screens therefore it is possible that the actor that was embodied has been
    // destroyed in the mean time.
    if (inFirstPerson_ && pFirstPerson_->okayToSwitchTo1stPerson())
    {
        // Switch Gui root.
        pFirstPerson_->becomeRoot();

        // Give 1st person the keyboard focus.
        GuiManager::instance().keyboardFocus(pFirstPerson_.get());

        // Remove the menu cursor.
        pSceneManager_->pDevice()->display()->useCursor(nullptr);
    }
    else
    {
        inFirstPerson_ = false;

        // Make inGameScreen Gui root.
        becomeRoot();

        // Give inGameScreen keyboard focus.
        GuiManager::instance().keyboardFocus(this);

        // Redraw all gui elements.
        activate();
    }
}

bool MachInGameScreen::inFirstPerson() const
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL_AUTO(inFirstPerson_);

    return inFirstPerson_;
}

const Gui::Coord& MachInGameScreen::rightClickMousePos() const
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL(Gui::Coord, rightClickMousePos_);

    return rightClickMousePos_;
}

void MachInGameScreen::updateWhilstInFirstPerson()
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(networkStuffedStartTime_);

    if (actualGameState() != PLAYING)
    {
        SimManager::instance().suspend();
    }

    pContinentMap_->refresh();

    // Update some things only every 20 frames
    static size_t frameCounter = 19;
    frameCounter = (++frameCounter) % 20;
    if (frameCounter == 0)
    {
        MachLogVoiceMailManager::instance().update();
        updateGameState();
    }

    // Update time network has been stuffed for
    if (MachLogNetwork::instance().isNetworkGame())
    {
        if (! NetNetwork::instance().imStuffed())
        {
            networkStuffedStartTime_ = DevTime::instance().time();
        }
    }
}

void MachInGameScreen::switchToMenus(bool switchTo)
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL_AUTO(switchToMenus_);

    switchToMenus_ = switchTo;
}

void MachInGameScreen::saveGame(PerOstream& outStream)
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pCameras_);

    pCameras_->saveGame(outStream);
    pContinentMap_->saveGame(outStream);
}

void MachInGameScreen::loadSavedGame(const string& planet, PerIstream& inStream)
{
    // De-pImpl_ variables used within this function.
    CB_DEPIMPL_AUTO(pContinentMap_);
    CB_DEPIMPL_AUTO(pCameras_);
    CB_DEPIMPL_AUTO(pWorldViewWindow_);
    CB_DEPIMPL_AUTO(pSquadronBank_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(pBmuButton_);
    CB_DEPIMPL_AUTO(gameState_);
    CB_DEPIMPL_AUTO(pMachinesIcon_);
    CB_DEPIMPL_AUTO(pConstructionsIcon_);
    CB_DEPIMPL_AUTO(pSquadronIcon_);
    CB_DEPIMPL_AUTO(pChatMessageDisplay_);
    CB_DEPIMPL_AUTO(pControlPanel_);

    // Don't allow any of the button setting below to trigger their sounds
    MachGuiSoundManager::instance().delaySounds();

    pCameras_->loadSavedGame(inStream);
    pContinentMap_->loadSavedGame(planet, inStream);
    pWorldViewWindow_->loadGame();
    pSquadronBank_->loadGame();
    pMachineNavigation_->loadGame();
    pConstructionNavigation_->loadGame();
    pMachinesIcon_->loadGame();
    pConstructionsIcon_->loadGame();
    pSquadronIcon_->loadGame();
    pControlPanel_->setupDecalBitmaps();

    // button should start depressed.
    pBmuButton_->setDepressed(true);

    // Switch to main menu context.
    mainMenuContext();

    if (MachLogNetwork::instance().isNetworkGame())
    {
        pChatMessageDisplay_ = new MachGuiInGameChatMessagesDisplay(this, Gui::Box(202, 0, 640, 37));
    }

    gameState_ = PLAYING;
}

MachGuiControlPanel& MachInGameScreen::controlPanel()
{
    CB_DEPIMPL_AUTO(pControlPanel_);

    PRE(pControlPanel_);

    return *pControlPanel_;
}

bool MachInGameScreen::displayControlPanel() const
{
    CB_DEPIMPL_AUTO(controlPanelContext_);

    bool returnVal = controlPanelContext_ == MachGui::SQUADRON_MENU
        || controlPanelContext_ == MachGui::FORM_SQUADRON_COMMAND
        || controlPanelContext_ == MachGui::MACHINE_NAVIGATION_MENU
        || controlPanelContext_ == MachGui::CONSTRUCTION_NAVIGATION_MENU
        || controlPanelContext_ == MachGui::SINGLE_FACTORY || controlPanelContext_ == MachGui::HARDWARE_RESEARCH
        || controlPanelContext_ == MachGui::CONSTRUCT_COMMAND;

    if (! returnVal)
    {
        if (corralState() & CORRAL_ALLCONSTRUCTORS && corralState() & CORRAL_ALLMACHINES
            && corralState() & CORRAL_SOMEFRIENDLY)
        {
            returnVal = true;
        }
    }

    return returnVal;
}

void MachInGameScreen::setupCameraScrollAreas()
{
    CB_DEPIMPL_AUTO(pTopCameraScrollArea_);
    CB_DEPIMPL_AUTO(pBottomCameraScrollArea_);
    CB_DEPIMPL_AUTO(pLeftCameraScrollArea_);
    CB_DEPIMPL_AUTO(pRightCameraScrollArea_);
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pCameras_);

    delete pTopCameraScrollArea_;
    delete pBottomCameraScrollArea_;
    delete pLeftCameraScrollArea_;
    delete pRightCameraScrollArea_;

    RenDevice& device = *pSceneManager_->pDevice();
    const int w = device.windowWidth();
    const int h = device.windowHeight();

    pTopCameraScrollArea_
        = new MachGuiCameraScrollArea(this, Gui::Box(0, 0, w, 1), MachGuiCameraScrollArea::Top, pCameras_.get(), this);
    pBottomCameraScrollArea_ = new 
        MachGuiCameraScrollArea(this, Gui::Box(0, h - 1, w, h), MachGuiCameraScrollArea::Bottom, pCameras_.get(), this);
    pLeftCameraScrollArea_
        = new MachGuiCameraScrollArea(this, Gui::Box(0, 0, 1, h), MachGuiCameraScrollArea::Left, pCameras_.get(), this);
    pRightCameraScrollArea_ = new 
        MachGuiCameraScrollArea(this, Gui::Box(w - 1, 0, w, h), MachGuiCameraScrollArea::Right, pCameras_.get(), this);
}

void MachInGameScreen::setupPromptText()
{
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pCameras_);
    CB_DEPIMPL_AUTO(pPromptText_);
    CB_DEPIMPL_AUTO(controlPanelXPos_);

    RenDevice& device = *pSceneManager_->pDevice();
    const int w = device.windowWidth();
    const int h = device.windowHeight();

    // Construct the prompt text displayable
    if (pPromptText_ == nullptr)
    {
        pPromptText_ = new MachPromptText(
            this,
            Gui::Boundary(controlPanelXPos_, h + MachGui::promptTextYOffset(), w, h),
            pCameras_.get(),
            &worldViewWindow());
    }

    // Move prompt text
    Gui::Coord promptNewTopLeft(controlPanelXPos_ + MachGui::promptTextXOffset(), h + MachGui::promptTextYOffset());
    positionChildRelative(pPromptText_, promptNewTopLeft);
}

void MachInGameScreen::setupNavigators()
{
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pMachineNavigation_);
    CB_DEPIMPL_AUTO(pConstructionNavigation_);
    CB_DEPIMPL_AUTO(pSquadronBank_);
    CB_DEPIMPL_AUTO(pSquadronIcon_);
    CB_DEPIMPL_AUTO(pControlPanel_);

    if (! pMachineNavigation_)
    {
        // Work out relative coord for all navigators
        Gui::Coord navTopLeft(1, pControlPanel_->getVisibleHeight() - MachGuiNavigatorBase::reqHeight());

        // Construct machine navigator
        pMachineNavigation_ = new MachGuiMachineNavigator(pControlPanel_, navTopLeft, this);

        // Construct construction navigator
        pConstructionNavigation_ = new MachGuiConstructionNavigator(pControlPanel_, navTopLeft, this);

        // Construct squadron bank
        pSquadronBank_ = new MachGuiSquadronBank(pControlPanel_, navTopLeft, pSquadronIcon_, this);
    }
    else
    {
        pControlPanel_->repositionNavigators(pMachineNavigation_, pConstructionNavigation_, pSquadronBank_);
    }
}

void MachInGameScreen::setupCorralAndCommandIcons()
{
    CB_DEPIMPL_AUTO(pCorral_);
    CB_DEPIMPL_AUTO(pCommandIcons_);
    CB_DEPIMPL_AUTO(pControlPanel_);
    CB_DEPIMPL_AUTO(selectedActors_);

    delete pCorral_;
    delete pCommandIcons_;

    // Construct corral
    pCorral_ = new MachGuiCorral(pControlPanel_, Gui::Coord(1, 0), this);
    pCorral_->initialise();
    pCorral_->add(selectedActors_);

    // Construct command icons
    Gui::Coord commandCoord(2, pCorral_->height() + 2);
    pCommandIcons_ = new MachCommandIcons(pControlPanel_, commandCoord, this);
}

void MachInGameScreen::resolutionChange()
{
    CB_DEPIMPL_AUTO(resolutionChanged_);
    CB_DEPIMPL_AUTO(pFirstPerson_);

    resolutionChanged_ = true;
    pFirstPerson_->resolutionChange();
}

MachInGameScreen::CorralStateBitfield MachInGameScreen::corralState() const
{
    CB_DEPIMPL_AUTO(corralState_);

    return corralState_;
}

void MachInGameScreen::updateCorralState()
{
    CB_DEPIMPL_AUTO(corralState_);
    CB_DEPIMPL_AUTO(selectedActors_);

    corralState_ = CORRAL_EMPTY;

    NEIL_STREAM("MachInGameScreen::updateCorralState()" << std::endl);
    NEIL_STREAM("selectedActors_.size() " << selectedActors_.size() << std::endl);

    if (selectedActors_.size() > 0)
    {
        bool allMachines = true;
        bool allConstructions = true;
        bool allFactories = true;
        bool allConstructors = true;
        bool allAPCs = true;
        bool someMachines = false;
        bool someConstructions = false;
        bool someFriendly = false;
        bool someEnemy = false;

        MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

        if (selectedActors_.size() == 1)
        {
            corralState_ = CORRAL_ONEENTRY;
            NEIL_STREAM("   CORRAL_ONEENTRY" << std::endl);
        }

        for (Actors::iterator iter = selectedActors_.begin(); iter != selectedActors_.end(); ++iter)
        {
            MachActor* pActor = *iter;

            // Work out if actor is enemy or fristd::endly
            if (pActor->race() == playerRace)
            {
                someFriendly = true;
            }
            else
            {
                someEnemy = true;
            }

            if (pActor->objectIsMachine())
            {
                NEIL_STREAM("   pActor->objectIsMachine()" << std::endl);

                allConstructions = false;
                someMachines = true;

                if (pActor->objectType() != MachLog::APC)
                {
                    allAPCs = false;
                }
                if (pActor->objectType() != MachLog::CONSTRUCTOR)
                {
                    allConstructors = false;
                }
            }
            else if (pActor->objectIsConstruction())
            {
                NEIL_STREAM("   pActor->objectIsConstruction()" << std::endl);

                allMachines = false;
                someConstructions = true;

                if (pActor->objectType() != MachLog::FACTORY)
                {
                    allFactories = false;
                }
            }
        }

        if (allMachines)
        {
            corralState_ |= CORRAL_ALLMACHINES;
            NEIL_STREAM("   CORRAL_ALLMACHINES" << std::endl);
        }
        if (allConstructions)
        {
            corralState_ |= CORRAL_ALLCONSTRUCTIONS;
            NEIL_STREAM("   CORRAL_ALLCONSTRUCTIONS" << std::endl);
        }
        if (allFactories && someConstructions)
        {
            corralState_ |= CORRAL_ALLFACTORIES;
            NEIL_STREAM("   CORRAL_ALLFACTORIES" << std::endl);
        }
        if (allConstructors && someMachines)
        {
            corralState_ |= CORRAL_ALLCONSTRUCTORS;
            NEIL_STREAM("   CORRAL_ALLCONSTRUCTORS" << std::endl);
        }
        if (allAPCs && someMachines)
        {
            corralState_ |= CORRAL_ALLAPCS;
            NEIL_STREAM("   CORRAL_ALLAPCS" << std::endl);
        }
        if (someMachines)
        {
            corralState_ |= CORRAL_SOMEMACHINES;
            NEIL_STREAM("   CORRAL_SOMEMACHINES" << std::endl);
        }
        if (someConstructions)
        {
            corralState_ |= CORRAL_SOMECONSTRUCTIONS;
            NEIL_STREAM("   CORRAL_SOMECONSTRUCTIONS" << std::endl);
        }
        if (someEnemy)
        {
            corralState_ |= CORRAL_SOMEENEMY;
            NEIL_STREAM("   CORRAL_SOMEENEMY" << std::endl);
        }
        if (someFriendly)
        {
            corralState_ |= CORRAL_SOMEFRIENDLY;
            NEIL_STREAM("   CORRAL_SOMEFRIENDLY" << std::endl);
        }
    }

    NEIL_STREAM("corralState_ " << corralState_ << std::endl);
}

bool MachInGameScreen::controlPanelOn() const
{
    CB_DEPIMPL_AUTO(controlPanelOn_);

    return controlPanelOn_;
}

void MachInGameScreen::controlPanelOn(bool on)
{
    CB_DEPIMPL_AUTO(controlPanelOn_);

    controlPanelOn_ = on;
}

void MachInGameScreen::initiateScreenShot()
{
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(renderingScreenShot_);

    RenDevice& device = *pSceneManager_->pDevice();

    // It's quite possible that other high-quality rendering options could
    // be turned on, in addition to anti-aliasing.
    if (device.capabilities().supportsEdgeAntiAliasing() && !device.antiAliasingOn())
    {
        device.antiAliasingOn(true);
        renderingScreenShot_ = true;
    }
    else
    {
        saveScreenShot();
    }
}

bool MachInGameScreen::isRenderingScreenShot() const
{
    return pImpl_->renderingScreenShot_;
}

void MachInGameScreen::finalizeScreenShot()
{
    PRE(isRenderingScreenShot());
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(renderingScreenShot_);

    saveScreenShot();

    // This carefully avoids turning anti-aliasing off when some other client
    // of RenDevice has turned it on (perhaps at the player's behest).
    RenDevice& device = *pSceneManager_->pDevice();
    device.antiAliasingOn(false);
    renderingScreenShot_ = false;

    POST(!isRenderingScreenShot());
}

// Can't use Gui::writeScreenAsBmp() because we need to save the front buffer.
void MachInGameScreen::saveScreenShot()
{
    CB_DEPIMPL_AUTO(pSceneManager_);

    bool gotPathName = false;
    SysPathName pathName;
    size_t count = 0;

    // Search for the first numbered file which doesn't exist already.
    while (!gotPathName)
    {
        char buffer[20];

        sprintf(buffer, "%04ld", count);

        pathName = SysPathName(string("mach") + buffer + ".png");

        if (! pathName.existsAsFile())
            gotPathName = true;

        ++count;
    }

    RenDevice& device = *pSceneManager_->pDevice();
    RenSurface front = device.frontSurface();
    front.saveAsPng(pathName);
}

Gui::Box MachInGameScreen::getWorldViewWindowVisibleArea() const
{
    CB_DEPIMPL_AUTO(pSceneManager_);
    CB_DEPIMPL_AUTO(pControlPanel_);

    Gui::Coord topLeftCoord = pControlPanel_->relativeCoord();

    RenDevice& device = *pSceneManager_->pDevice();
    const int w = device.windowWidth();
    const int h = device.windowHeight();
    Gui::Box visibleArea(pControlPanel_->width() + topLeftCoord.x(), 0, w, h);

    return visibleArea;
}

MachGuiIonAttackCommand* MachInGameScreen::ionAttackCommand()
{
    CB_DEPIMPL_AUTO(pIonAttackCommand_);

    return pIonAttackCommand_;
}

MachGuiNukeAttackCommand* MachInGameScreen::nukeAttackCommand()
{
    CB_DEPIMPL_AUTO(pNukeAttackCommand_);

    return pNukeAttackCommand_;
}

MachInGameScreen::GameState MachInGameScreen::actualGameState() const
{
    CB_DEPIMPL(MachInGameScreen::GameState, gameState_);
    CB_DEPIMPL_AUTO(gameStateTimer_);

    GameState retVal = PLAYING;

    if (gameState_ != PLAYING)
    {
        PhysAbsoluteTime now = Phys::time();
        // Only return that game has been lost/won after it has been in this state for a few seconds.
        // This enables pod destruction animation time to run and music to play.
        if (now - gameStateTimer_ > 2.0)
        {
            retVal = gameState_;
        }
    }

    return retVal;
}

bool MachInGameScreen::isNetworkStuffed() const
{
    CB_DEPIMPL_AUTO(networkStuffedStartTime_);

    bool retValue = false;

    if (MachLogNetwork::instance().isNetworkGame())
    {
        double now = DevTime::instance().time();

        retValue = now - networkStuffedStartTime_ > 2.0; // Network has been stuffed for over two seconds
    }

    return retValue;
}

void MachInGameScreen::disableFirstPerson(bool newValue)
{
    CB_DEPIMPL_AUTO(disableFirstPerson_);

    disableFirstPerson_ = newValue;
}

bool MachInGameScreen::isFirstPersonDisabled() const
{
    CB_DEPIMPL_AUTO(disableFirstPerson_);

    bool retValue = false;

    if (MachLogNetwork::instance().isNetworkGame() && disableFirstPerson_)
    {
        retValue = true;
    }

    return retValue;
}

void MachInGameScreen::instantExit(bool newValue)
{
    CB_DEPIMPL_AUTO(instantExit_);

    instantExit_ = newValue;
}

bool MachInGameScreen::instantExit() const
{
    CB_DEPIMPL_AUTO(instantExit_);

    return instantExit_;
}

/* End INGAME.CPP ***************************************************/
