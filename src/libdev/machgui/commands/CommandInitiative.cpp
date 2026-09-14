//  Definitions of non-inline non-template methods and global functions

#include "CommandInitiative.hpp"

#include "mathex/Point3d.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/internal/strings.hpp"
#include "world4d/Scene/Domain.hpp"
#include "machlog/Actors/Actor.hpp"
#include "machlog/Actors/Machine.hpp"

// Statics
bool MachGuiInitiativeCommand::initiativeHigh_ = false;
bool MachGuiInitiativeCommand::initiativeMedium_ = false;
bool MachGuiInitiativeCommand::initiativeLow_ = false;
bool MachGuiInitiativeCommand::goLowFromMedium_ = true;

MachGuiInitiativeCommand::MachGuiInitiativeCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen, "commands-initiative"_bind)
{
    TEST_INVARIANT;
}

MachGuiInitiativeCommand::~MachGuiInitiativeCommand()
{
    TEST_INVARIANT;

    inGameScreen().setCursorFilter(W4dDomain::EXCLUDE_NOT_SOLID);
}

void MachGuiInitiativeCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiInitiativeCommand& t)
{

    o << "MachGuiInitiativeCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiInitiativeCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiInitiativeCommand::canActorEverExecute(const MachActor& actor) const
{
    // Machines can have initiative set
    return actor.objectIsMachine();
}

// virtual
bool MachGuiInitiativeCommand::isInteractionComplete() const
{
    return true;
}

// virtual
MachGui::Cursor2dType
MachGuiInitiativeCommand::cursorOnTerrain(const MexPoint3d& /*location*/, bool /*ctrlPressed*/, bool, bool)
{
    return MachGui::INVALID_CURSOR;
}

// virtual
MachGui::Cursor2dType MachGuiInitiativeCommand::cursorOnActor(MachActor* /*pActor*/, bool, bool, bool)
{
    return MachGui::INVALID_CURSOR;
}

// virtal
void MachGuiInitiativeCommand::typeData(MachLog::ObjectType /*objectType*/, int /*subType*/, uint /*level*/)
{
}

// virtual
bool MachGuiInitiativeCommand::doApply(MachActor* pActor, std::string*)
{
    MachLog::Initiative newInitiative = MachLog::INITIATIVE_MEDIUM;

    if (initiativeHigh_ && initiativeMedium_ && initiativeLow_)
        newInitiative = MachLog::INITIATIVE_LOW;
    else if (initiativeHigh_ && initiativeMedium_)
        newInitiative = MachLog::INITIATIVE_MEDIUM;
    else if (initiativeMedium_ && initiativeLow_)
        newInitiative = MachLog::INITIATIVE_LOW;
    else if (initiativeHigh_ && initiativeLow_)
        newInitiative = MachLog::INITIATIVE_LOW;
    else if (initiativeHigh_)
        newInitiative = MachLog::INITIATIVE_MEDIUM;
    else if (initiativeMedium_)
    {
        if (goLowFromMedium_)
        {
            newInitiative = MachLog::INITIATIVE_LOW;
        }
        else
        {
            newInitiative = MachLog::INITIATIVE_HIGH;
        }
    }

    DEBUG_STREAM(DIAG_NEIL, "MachGuiInitiativeCommand::doApply " << newInitiative << std::endl);

    pActor->asMachine().initiative(newInitiative);

    return true;
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiInitiativeCommand::clone() const
{
    return std::make_unique<MachGuiInitiativeCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiInitiativeCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/defco1.bmp", "gui/commands/defco1.bmp");
    return names;
}

// virtual
uint MachGuiInitiativeCommand::cursorPromptStringId() const
{
    if (initiativeHigh_)
        return IDS_INITIATIVE_HIGH_COMMAND;
    else if (initiativeMedium_)
        return IDS_INITIATIVE_MEDIUM_COMMAND;

    return IDS_INITIATIVE_LOW_COMMAND;
}

// virtual
uint MachGuiInitiativeCommand::commandPromptStringid() const
{
    if (initiativeHigh_)
        return IDS_INITIATIVE_HIGH_COMMAND;
    else if (initiativeMedium_)
        return IDS_INITIATIVE_MEDIUM_COMMAND;

    return IDS_INITIATIVE_LOW_COMMAND;
}

// virtual
bool MachGuiInitiativeCommand::canAdminApply() const
{
    return false;
}

// virtual
bool MachGuiInitiativeCommand::doAdminApply(MachLogAdministrator* /*pAdministrator*/, std::string*)
{
    PRE(canAdminApply());

    return false;
}

void MachGuiInitiativeCommand::update(const Actors& actors)
{
    // Work out which lights are on
    initiativeHigh_ = false;
    initiativeMedium_ = false;
    initiativeLow_ = false;

    for (Actors::const_iterator iter = actors.begin(); iter != actors.end(); ++iter)
    {
        if ((*iter)->objectIsMachine()) // Initiative only applies to machines
        {
            switch ((*iter)->asMachine().initiative())
            {
                case MachLog::INITIATIVE_LOW:
                    initiativeLow_ = true;
                    break;
                case MachLog::INITIATIVE_MEDIUM:
                    initiativeMedium_ = true;
                    break;
                case MachLog::INITIATIVE_HIGH:
                    initiativeHigh_ = true;
                    break;
                    DEFAULT_ASSERT_BAD_CASE((*iter)->asMachine().initiative());
            }
        }
    }

    // now may need to alter the goLowFromMedium_ movement direction flag
    if (initiativeHigh_ && ! initiativeMedium_ && ! initiativeLow_)
    {
        goLowFromMedium_ = true;
    }
    else if (initiativeLow_ && ! initiativeMedium_ && ! initiativeHigh_)
    {
        goLowFromMedium_ = false;
    }

    DEBUG_STREAM(
        DIAG_NEIL,
        "MachGuiInitiativeCommand::update " << initiativeHigh_ << " " << initiativeMedium_ << " " << initiativeLow_
                                           << std::endl);
}

// static
bool MachGuiInitiativeCommand::initiativeHigh()
{
    return initiativeHigh_;
}

// static
bool MachGuiInitiativeCommand::initiativeMedium()
{
    return initiativeMedium_;
}

// static
bool MachGuiInitiativeCommand::initiativeLow()
{
    return initiativeLow_;
}

void MachGuiInitiativeCommand::resetDirectionFromMedium()
{
    goLowFromMedium_ = true;
}
