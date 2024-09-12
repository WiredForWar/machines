/*
 * C M D D E F A U  . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmddefau.hpp"

#include "gui/event.hpp"
#include "mathex/point3d.hpp"
#include "machgui/intcurac.hpp"
#include "machgui/commands/cmdattac.hpp"
#include "machgui/commands/cmdmove.hpp"
#include "machgui/commands/cmdconst.hpp"
#include "machgui/commands/cmdionat.hpp"
#include "machgui/commands/cmdlocto.hpp"
#include "machgui/commands/cmdpicku.hpp"
#include "machgui/commands/cmdtrans.hpp"
#include "machgui/commands/cmdheal.hpp"
#include "machgui/commands/cmdrepar.hpp"
#include "machgui/commands/cmdscav.hpp"
#include "machgui/commands/cmdcaptr.hpp"
#include "machgui/commands/cmddeplo.hpp"
#include "machgui/commands/cmdassem.hpp"
#include "machgui/commands/cmdrecyc.hpp"
#include "machgui/commands/cmdtrech.hpp"
#include "machgui/commands/cmddecon.hpp"
#include "machgui/ingame.hpp"
#include "machgui/cameras.hpp"

#include "machlog/actor.hpp"
#include "machlog/canattac.hpp"
#include "machlog/races.hpp"
#include "machlog/planet.hpp"
#include "machlog/machine.hpp"

#include "phys/cspace2.hpp"

MachGuiDefaultCommand::MachGuiDefaultCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
    , pIntelligentCursorOnActor_(new MachGuiIntelligentCursorOnActor)
{
    TEST_INVARIANT;
}

MachGuiDefaultCommand::~MachGuiDefaultCommand()
{
    TEST_INVARIANT;
    delete pIntelligentCursorOnActor_;
}

void MachGuiDefaultCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDefaultCommand& t)
{

    o << "MachGuiDefaultCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDefaultCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiDefaultCommand::pickOnTerrain(
    const MexPoint3d& location,
    bool ctrlPressed,
    bool shiftPressed,
    bool altPressed)
{
    // Check the intelligent cursor
    MachGui::Cursor2dType cursor = cursorOnTerrain(location, ctrlPressed, shiftPressed, altPressed);

    MachGuiCommand* pCommand = nullptr;

    if (cursor == MachGui::MOVETO_CURSOR)
    {
        // Do intelligent move
        pCommand = new MachGuiMoveCommand(&inGameScreen());
    }
    else if (cursor == MachGui::LOCATETO_CURSOR)
    {
        // Do intelligent locate
        pCommand = new MachGuiLocateToCommand(&inGameScreen());
    }
    else if (cursor == MachGui::ION_ATTACK_CURSOR)
    {
        // Do ion attack
        pCommand = new MachGuiIonAttackCommand(&inGameScreen());
    }
    else if (cursor == MachGui::DEPLOY_CURSOR)
    {
        // APC deploy
        pCommand = new MachGuiDeployCommand(&inGameScreen());
    }
    else if (cursor == MachGui::ASSEMBLEPOINT_CURSOR)
    {
        // Assemble At for factories
        pCommand = new MachGuiAssemblyPointCommand(&inGameScreen());
    }

    if (pCommand != nullptr)
    {
        // Pass on the pick
        pCommand->pickOnTerrain(location, ctrlPressed, shiftPressed, altPressed);

        // If this gives us a complete command, do it
        if (pCommand->isInteractionComplete())
            pCommand->apply();

        // Tidy up
        delete pCommand;
    }
}

// virtual
void MachGuiDefaultCommand::pickOnActor(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    MachGuiCommand* pCommand = nullptr;

    // Get the intelligent cursor
    MachGui::Cursor2dType cursor = cursorOnActor(pActor, ctrlPressed, shiftPressed, altPressed);
    switch (cursor)
    {
        case MachGui::MOVETO_CURSOR:
        case MachGui::FOLLOW_CURSOR:
            {
                pCommand = new MachGuiMoveCommand(&inGameScreen());
                break;
            }

        case MachGui::ATTACK_CURSOR:
            {
                pCommand = new MachGuiAttackCommand(&inGameScreen());
                break;
            }

        case MachGui::ION_ATTACK_CURSOR:
            {
                pCommand = new MachGuiIonAttackCommand(&inGameScreen());
                break;
            }

        case MachGui::CONSTRUCT_CURSOR:
        case MachGui::JOINCONSTRUCT_CURSOR:
            {
                pCommand = new MachGuiConstructCommand(&inGameScreen());
                break;
            }

        case MachGui::LOCATETO_CURSOR:
            {
                pCommand = new MachGuiLocateToCommand(&inGameScreen());
                break;
            }

        case MachGui::PICKUP_CURSOR:
            {
                pCommand = new MachGuiPickUpCommand(&inGameScreen());
                break;
            }

        case MachGui::SCAVENGE_CURSOR:
            {
                pCommand = new MachGuiScavengeCommand(&inGameScreen());
                break;
            }

        case MachGui::TRANSPORT_CURSOR:
            {
                pCommand = new MachGuiTransportCommand(&inGameScreen(), false);
                break;
            }

        case MachGui::ENTER_BUILDING_CURSOR:
            {
                pCommand = new MachGuiMoveCommand(&inGameScreen());
                break;
            }

        case MachGui::ENTER_APC_CURSOR:
            {
                pCommand = new MachGuiMoveCommand(&inGameScreen());
                break;
            }

        case MachGui::HEAL_CURSOR:
            {
                pCommand = new MachGuiHealCommand(&inGameScreen());
                break;
            }

        case MachGui::TREACHERY_CURSOR:
            {
                pCommand = new MachGuiTreacheryCommand(&inGameScreen());
                break;
            }

        case MachGui::REPAIR_CURSOR:
            {
                pCommand = new MachGuiRepairCommand(&inGameScreen());
                break;
            }

        case MachGui::CAPTURE_CURSOR:
            {
                pCommand = new MachGuiCaptureCommand(&inGameScreen());
                break;
            }

        case MachGui::DECONSTRUCT_CURSOR:
            {
                pCommand = new MachGuiDeconstructCommand(&inGameScreen());
                break;
            }

        case MachGui::RECYCLE_CURSOR:
            {
                pCommand = new MachGuiRecycleCommand(&inGameScreen());
                break;
            }

        case MachGui::SELECT_CURSOR:
            {
                selectActors(pActor, ctrlPressed, shiftPressed, altPressed);
                break;
            }
    }

    // Deal with an intelligent command
    if (pCommand != nullptr)
    {
        // Pass on the pick
        pCommand->pickOnActor(pActor, ctrlPressed, shiftPressed, altPressed);

        // If this gives us a complete command, do it
        if (pCommand->isInteractionComplete())
            pCommand->apply();

        // Tidy up
        delete pCommand;
    }
}

void MachGuiDefaultCommand::selectActors(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    ASSERT(
        pActor->selectableType() == MachLog::FULLY_SELECTABLE,
        "Attempting to select into the corral an invalid actor");
    // Add the actor to the list, if a machine
    if (ctrlPressed)
    {
        bool select = !inGameScreen().isSelected(*pActor);
        if (select)
        {
            const ctl_pvector<MachActor> visibleActors = inGameScreen().getVisibleActors();
            ctl_pvector<MachActor> actorsToAdd;
            std::copy_if(
                visibleActors.begin(),
                visibleActors.end(),
                std::back_inserter(actorsToAdd),
                [pActor](MachActor* pReferenceActor) {
                    if (pActor->race() != pReferenceActor->race())
                        return false;
                    if (!MachActor::IsSameActorType(pActor, pReferenceActor))
                        return false;
                    if (pReferenceActor->selectionState() == MachLog::SELECTED)
                        return false;

                    return true;
                });
            ASSERT(!actorsToAdd.empty(), "The must be at least one actor (the picked one)");
            inGameScreen().select(actorsToAdd);
        }
        else
        {
            const ctl_pvector<MachActor> currentSelection = inGameScreen().selectedActors();
            ctl_pvector<MachActor> actorsToRemove;
            std::copy_if(
                currentSelection.begin(),
                currentSelection.end(),
                std::back_inserter(actorsToRemove),
                [pActor](MachActor* pActorInSelection) {
                    return MachActor::IsSameActorType(pActor, pActorInSelection);
                });
            ASSERT(!actorsToRemove.empty(), "The must be at least one actor (the picked one)");
            inGameScreen().deselect(actorsToRemove);
        }
    }
    else if (shiftPressed)
    {
        // See if already selected: toggle state
        if (inGameScreen().isSelected(*pActor))
            inGameScreen().deselect(pActor);
        else
            inGameScreen().select(pActor);
    }
    else
    {
        double currentT = DevTime::instance().time();
        if (currentT - actorSelectedTime_ < MachGui::doubleClickInterval())
        {
            actorSelectedTime_ = 0;
            if (pActor == pLastSelectedActor_)
            {
                inGameScreen().deselectAll();

                // The actor is double-clicked
                const ctl_pvector<MachActor> visibleActors = inGameScreen().getVisibleActors();
                ctl_pvector<MachActor> actorsToAdd;
                std::copy_if(
                    visibleActors.begin(),
                    visibleActors.end(),
                    std::back_inserter(actorsToAdd),
                    [pActor](MachActor* pReferenceActor) {
                        if (pActor->race() != pReferenceActor->race())
                            return false;
                        if (!MachActor::IsSameActorType(pActor, pReferenceActor))
                            return false;

                        return true;
                    });
                ASSERT(!actorsToAdd.empty(), "The must be at least one actor (the picked one)");
                inGameScreen().select(actorsToAdd);
                return;
            }
        }
        actorSelectedTime_ = currentT;
        pLastSelectedActor_ = pActor;

        inGameScreen().deselectAll();
        inGameScreen().select(pActor);
    }
}

// virtual
bool MachGuiDefaultCommand::canActorEverExecute(const MachActor&) const
{
    return true;
}

// virtual
bool MachGuiDefaultCommand::isInteractionComplete() const
{
    return true;
}

// virtual
bool MachGuiDefaultCommand::doApply(MachActor*, std::string*)
{
    return true;
}

// virtual
MachGui::Cursor2dType MachGuiDefaultCommand::cursorOnTerrain(const MexPoint3d& location, bool, bool, bool altPressed)
{
    MachGui::Cursor2dType cursor = MachGui::MENU_CURSOR;

    // Check for a legal move position with at least one fristd::endly machine selected

    // Get player race
    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

    // Check for any selected actors
    const MachInGameScreen::Actors& selectedActors = inGameScreen().selectedActors();
    if (selectedActors.size() != 0)
    {
        // Get the actor and the intelligent cursor it implies for the operand
        MachActor* pSelectedActor = nullptr;

        for (MachInGameScreen::Actors::const_iterator it = selectedActors.begin(); it != selectedActors.end(); ++it)
        {
            // Find an actor in corral that ain't in an APC
            if (!((*it)->objectIsMachine() && (*it)->asMachine().insideAPC()))
            {
                pSelectedActor = *it;
                break;
            }
        }

        if (pSelectedActor)
        {
            if (pSelectedActor->objectIsMachine() && pSelectedActor->race() == playerRace)
            {
                // If we are outside a building then we need to check if we are trying to move
                // to a valid domain
                if (! inGameScreen().cameras()->currentCamera()->insideConstruction())
                {
                    if (cursorInFogOfWar() || isPointValidOnTerrain(location, IGNORE_SELECTED_ACTOR_OBSTACLES))
                    {
                        // Locators default to Locate, move is alternative
                        if (pSelectedActor->objectType() == MachLog::GEO_LOCATOR && ! altPressed)
                        {
                            cursor = MachGui::LOCATETO_CURSOR;
                        }
                        // APC default to move, deploy is alternative
                        else if (pSelectedActor->objectType() == MachLog::APC && altPressed)
                        {
                            cursor = MachGui::DEPLOY_CURSOR;
                        }
                        // All other machines move to terrain position
                        else
                        {
                            cursor = MachGui::MOVETO_CURSOR;
                        }
                    }
                }
                else
                {
                    cursor = MachGui::MOVETO_CURSOR;
                }
            }
            // Factories get the "assemble at" cursor as an alternative
            else if (
                pSelectedActor->objectType() == MachLog::FACTORY && pSelectedActor->race() == playerRace
                && altPressed)
            {
                if (cursorInFogOfWar() || isPointValidOnTerrain(location, IGNORE_SELECTED_ACTOR_OBSTACLES))
                {
                    cursor = MachGui::ASSEMBLEPOINT_CURSOR;
                }
            }
        }
    }

    return cursor;
}

// virtual
MachGui::Cursor2dType
MachGuiDefaultCommand::cursorOnActor(MachActor* pCursorActor, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    MachGui::Cursor2dType cursor = MachGui::SELECT_CURSOR;

    // Get player race
    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

    // Check for any selected actor
    const MachInGameScreen::Actors& selectedActors = inGameScreen().selectedActors();
    if (selectedActors.size() != 0)
    {
        // Get the actor and the intelligent cursor it implies for the operand
        MachActor* pSelectedActor = nullptr;

        for (MachInGameScreen::Actors::const_iterator it = selectedActors.begin(); it != selectedActors.end(); ++it)
        {
            // Find an actor in corral that ain't in an APC
            if (!((*it)->objectIsMachine() && (*it)->asMachine().insideAPC()))
            {
                pSelectedActor = *it;
                break;
            }
        }

        if (pSelectedActor)
        {
            cursor = pIntelligentCursorOnActor_->cursorType(
                pSelectedActor,
                pCursorActor,
                &inGameScreen(),
                ctrlPressed,
                shiftPressed,
                altPressed);
        }
        else if (pCursorActor->selectableType() != MachLog::FULLY_SELECTABLE)
        {
            cursor = MachGui::MENU_CURSOR;
        }
    }
    else if (pCursorActor->selectableType() != MachLog::FULLY_SELECTABLE)
        cursor = MachGui::MENU_CURSOR;

    return cursor;
}

// virtual
void MachGuiDefaultCommand::typeData(MachLog::ObjectType, int, uint)
{
    // Do nothing
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiDefaultCommand::clone() const
{
    return std::make_unique<MachGuiDefaultCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiDefaultCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("", "");
    return names;
}

// virtual
bool MachGuiDefaultCommand::actorsCanExecute() const
{
    return true;
}

// virtual
uint MachGuiDefaultCommand::cursorPromptStringId() const
{
    return 0;
}

// virtual
uint MachGuiDefaultCommand::commandPromptStringid() const
{
    return 0;
}
/* End CMDDEFAU.CPP **************************************************/
