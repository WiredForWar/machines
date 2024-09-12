/*
 * C M D R E F I L L  . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmddamag.hpp"

#include "gui/event.hpp"
#include "gui/event.hpp"
#include "mathex/point3d.hpp"
#include "machgui/ingame.hpp"
#include "machgui/internal/strings.hpp"
#include "device/butevent.hpp"
#include "machlog/actor.hpp"

MachGuiDamageCommand::MachGuiDamageCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
    , hadFinalPick_(true)
{
    TEST_INVARIANT;
}

MachGuiDamageCommand::~MachGuiDamageCommand()
{
    TEST_INVARIANT;
}

void MachGuiDamageCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDamageCommand& t)
{

    o << "MachGuiDamageCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDamageCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiDamageCommand::canActorEverExecute(const MachActor& actor) const
{
    return actor.objectIsMachine() || actor.objectIsConstruction();
}

// virtual
bool MachGuiDamageCommand::isInteractionComplete() const
{
    return hadFinalPick_;
}

// virtual
bool MachGuiDamageCommand::doApply(MachActor* pActor, std::string* /*pReason*/)
{
    PRE(pActor->objectIsMachine() || pActor->objectIsConstruction());

    // Check locator type
    bool canDo = true;
    pActor->beHit((pActor->hp() / 2));

    return canDo;
}

// virtual
MachGui::Cursor2dType MachGuiDamageCommand::cursorOnTerrain(const MexPoint3d& /*location*/, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    return cursor;
}

// virtual
MachGui::Cursor2dType MachGuiDamageCommand::cursorOnActor(MachActor*, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    return cursor;
}

// virtual
void MachGuiDamageCommand::typeData(MachLog::ObjectType, int, uint)
{
    // Do nothing
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiDamageCommand::clone() const
{
    return std::make_unique<MachGuiDamageCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiDamageCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/damage.bmp", "gui/commands/damage.bmp");
    return names;
}

// virtual
uint MachGuiDamageCommand::cursorPromptStringId() const
{
    return IDS_SELF_DESTRUCT_COMMAND;
}

// virtual
uint MachGuiDamageCommand::commandPromptStringid() const
{
    return IDS_SELF_DESTRUCT_START;
}

// virtual
bool MachGuiDamageCommand::canAdminApply() const
{
    return false;
}

/* End CMDLOCTO.CPP **************************************************/
