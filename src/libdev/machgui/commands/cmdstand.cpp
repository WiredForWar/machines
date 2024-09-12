/*
 * C M D S T A N D  . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmdstand.hpp"

#include "machgui/ingame.hpp"
#include "machgui/internal/strings.hpp"

#include "device/butevent.hpp"

#include "world4d/domain.hpp"

#include "machlog/actor.hpp"
#include "machlog/machine.hpp"
#include "machlog/machvman.hpp"
#include "machlog/opstandg.hpp"

MachGuiStandGroundCommand::MachGuiStandGroundCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
{
    TEST_INVARIANT;
}

MachGuiStandGroundCommand::~MachGuiStandGroundCommand()
{
    TEST_INVARIANT;

    inGameScreen().setCursorFilter(W4dDomain::EXCLUDE_NOT_SOLID);
}

void MachGuiStandGroundCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiStandGroundCommand& t)
{

    o << "MachGuiStandGroundCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiStandGroundCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiStandGroundCommand::canActorEverExecute(const MachActor& actor) const
{
    // All machines can stand ground
    return actor.objectIsMachine();
}

// virtual
bool MachGuiStandGroundCommand::isInteractionComplete() const
{
    return true;
}

// virtual
MachGui::Cursor2dType
MachGuiStandGroundCommand::cursorOnTerrain(const MexPoint3d& /*location*/, bool /*ctrlPressed*/, bool, bool)
{
    return MachGui::MENU_CURSOR;
}

// virtual
MachGui::Cursor2dType MachGuiStandGroundCommand::cursorOnActor(MachActor* /*pActor*/, bool, bool, bool)
{
    return MachGui::MENU_CURSOR;
}

// virtal
void MachGuiStandGroundCommand::typeData(MachLog::ObjectType /*objectType*/, int /*subType*/, uint /*level*/)
{
}

// virtual
bool MachGuiStandGroundCommand::doApply(MachActor* pActor, string*)
{
    ASSERT(pActor->objectIsMachine(), "Tried to give stand ground operation to non-machine actor!");

    bool result = false;
    MachLogMachine& machine = pActor->asMachine();

    if (! machine.isStandingGround())
    {
        // Create a stand ground operation for the machine
        MachLogStandGroundOperation* pOp = new MachLogStandGroundOperation(&machine);

        machine.newOperation(pOp);

        if (! hasPlayedVoiceMail())
        {
            MachLogMachineVoiceMailManager::instance().postNewMail(machine, MachineVoiceMailEventID::TASKED);
            hasPlayedVoiceMail(true);
        }

        result = true;
    }

    return result;
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiStandGroundCommand::clone() const
{
    return std::make_unique<MachGuiStandGroundCommand>(&inGameScreen());
}

// virtual
const std::pair<string, string>& MachGuiStandGroundCommand::iconNames() const
{
    static std::pair<string, string> names("gui/commands/stand.bmp", "gui/commands/stand.bmp");
    return names;
}

// virtual
uint MachGuiStandGroundCommand::cursorPromptStringId() const
{
    return IDS_STANDGROUND_COMMAND;
}

// virtual
uint MachGuiStandGroundCommand::commandPromptStringid() const
{
    return IDS_STANDGROUND_START;
}

// virtual
bool MachGuiStandGroundCommand::canAdminApply() const
{
    return false;
}

// virtual
bool MachGuiStandGroundCommand::doAdminApply(MachLogAdministrator* /*pAdministrator*/, string*)
{
    ASSERT(false, "There is no admin stand ground operation");

    return true;
}

// virtual
bool MachGuiStandGroundCommand::processButtonEvent(const DevButtonEvent& be)
{
    if (isVisible() && be.scanCode() == Device::KeyCode::KEY_W && be.action() == DevButtonEvent::PRESS && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

/* End CMDDECON.CPP **************************************************/
