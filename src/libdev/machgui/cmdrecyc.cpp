/*
 * C M D R E C Y C . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/cmdrecyc.hpp"
#include "machgui/ingame.hpp"
#include "machgui/internal/strings.hpp"
#include "world4d/domain.hpp"
#include "machlog/actor.hpp"
#include "machlog/machvman.hpp"
#include "machlog/smelter.hpp"
#include "machlog/oprecycl.hpp"
#include "device/butevent.hpp"

MachGuiRecycleCommand::MachGuiRecycleCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
    , hadFinalPick_(false)
    , pSmeltingBuilding_(nullptr)
{
    TEST_INVARIANT;
}

MachGuiRecycleCommand::~MachGuiRecycleCommand()
{
    TEST_INVARIANT;

    inGameScreen().setCursorFilter(W4dDomain::EXCLUDE_NOT_SOLID);
}

void MachGuiRecycleCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiRecycleCommand& t)
{

    o << "MachGuiRecycleCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiRecycleCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiRecycleCommand::pickOnActor(MachActor* pActor, bool, bool, bool)
{
    // Check for a pick on construction
    // note that use of static cast relies on short-circuiting of AND operator
    if (pActor->objectIsCanSmelt() && pActor->asConstruction().isComplete())
    {
        hadFinalPick_ = true;
        pSmeltingBuilding_ = &pActor->asConstruction();
    }
    else
        pSmeltingBuilding_ = nullptr;
}

// virtual
bool MachGuiRecycleCommand::canActorEverExecute(const MachActor& actor) const
{
    // All machine types can be recycled
    return actor.objectIsMachine();
}

// virtual
bool MachGuiRecycleCommand::isInteractionComplete() const
{
    return hadFinalPick_;
}

// virtual
MachGui::Cursor2dType
MachGuiRecycleCommand::cursorOnTerrain(const MexPoint3d& /*location*/, bool /*ctrlPressed*/, bool, bool)
{
    return MachGui::MENU_CURSOR;
}

// virtual
MachGui::Cursor2dType MachGuiRecycleCommand::cursorOnActor(MachActor* pActor, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    if (pActor->objectIsCanSmelt() && (_STATIC_CAST(MachLogConstruction*, pActor))->isComplete())
    {
        cursor = MachGui::RECYCLE_CURSOR;
    }

    return cursor;
}

// virtal
void MachGuiRecycleCommand::typeData(MachLog::ObjectType /*objectType*/, int /*subType*/, uint /*level*/)
{
}

// virtual
bool MachGuiRecycleCommand::doApply(MachActor* pActor, string*)
{
    ASSERT(
        pSmeltingBuilding_ != nullptr,
        "pSmeltingBuilding_ should not have a NULL assignment if MachGuiRecycleCommand::doApply is called");

    MachLogRecycleOperation* pOp = new MachLogRecycleOperation(&pActor->asMachine(), pSmeltingBuilding_);

    pActor->newOperation(pOp);

    if (! hasPlayedVoiceMail())
    {
        MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::RECYCLE);
        hasPlayedVoiceMail(true);
    }

    return true;
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiRecycleCommand::clone() const
{
    return std::make_unique<MachGuiRecycleCommand>(&inGameScreen());
}

// virtual
const std::pair<string, string>& MachGuiRecycleCommand::iconNames() const
{
    static std::pair<string, string> names("gui/commands/recycle.bmp", "gui/commands/recycle.bmp");
    return names;
}

// virtual
uint MachGuiRecycleCommand::cursorPromptStringId() const
{
    return IDS_RECYCLE_COMMAND;
}

// virtual
uint MachGuiRecycleCommand::commandPromptStringid() const
{
    return IDS_RECYCLE_START;
}

// virtual
bool MachGuiRecycleCommand::canAdminApply() const
{
    return false;
}

// virtual
bool MachGuiRecycleCommand::doAdminApply(MachLogAdministrator* /*pAdministrator*/, string*)
{
    PRE(canAdminApply());

    return false;
}

// virtual
bool MachGuiRecycleCommand::processButtonEvent(const DevButtonEvent& be)
{
    if (isVisible() && be.scanCode() == DevKey::KEY_Q && be.action() == DevButtonEvent::PRESS && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

/* End CMDRECYC.CPP **************************************************/
