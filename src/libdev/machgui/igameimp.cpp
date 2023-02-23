/*
 * I G A M E I M P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/internal/igameimp.hpp"
#include "machgui/gui.hpp"

MachInGameScreenImpl::MachInGameScreenImpl()
    : pSceneManager_(nullptr)
    , pRoot_(nullptr)
    , pWorldViewWindow_(nullptr)
    , pDefaultCommand_(nullptr)
    , pActiveCommand_(nullptr)
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
    , pCursors2d_(nullptr)
    , pStringResourceLib_(nullptr)
    , pHighlightedActor_(nullptr)
    , pDefconCommand_(nullptr)
    , pSelfDestructCommand_(nullptr)
    , pIonAttackCommand_(nullptr)
    , pNukeAttackCommand_(nullptr)
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

/* End IGAMEIMP.CPP *************************************************/
