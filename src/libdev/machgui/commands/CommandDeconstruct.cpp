/*
 * C M D D E C O N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "CommandDeconstruct.hpp"

#include "gui/font.hpp"
#include "gui/restring.hpp"

#include "mathex/point3d.hpp"

#include "system/pathname.hpp"

#include "world4d/domain.hpp"

#include "machlog/actor.hpp"
#include "machlog/administ.hpp"
#include "machlog/machvman.hpp"
#include "machlog/opadsupc.hpp"
#include "machlog/opsupcon.hpp"
#include "machlog/races.hpp"
#include "machlog/squad.hpp"
#include "machlog/stats.hpp"

#include "machgui/internal/strings.hpp"
#include "machgui/internal/SoundManager.hpp"
#include "machgui/InGameScreen.hpp"

MachGuiDeconstructCommand::MachGuiDeconstructCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen, "commands-deconstruct"_bind)
    , hadFinalPick_(false)
{
    TEST_INVARIANT;
}

MachGuiDeconstructCommand::~MachGuiDeconstructCommand()
{
    TEST_INVARIANT;

    inGameScreen().setCursorFilter(W4dDomain::EXCLUDE_NOT_SOLID);

    while (! constructions_.empty())
    {
        constructions_.back()->detach(this);
        constructions_.pop_back();
    }
}

void MachGuiDeconstructCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDeconstructCommand& t)
{

    o << "MachGuiDeconstructCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDeconstructCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiDeconstructCommand::pickOnActor(MachActor* pActor, bool, bool shiftPressed, bool)
{
    // Check for a pick on construction
    if (pActor->objectIsConstruction())
    {
        MachLogConstruction* pCandidateConstruction = &pActor->asConstruction();

        if (! constructionIsDuplicate(pCandidateConstruction))
        {
            // Add to list of constructions to deconstruct
            constructions_.push_back(pCandidateConstruction);

            pCandidateConstruction->attach(this);
        }

        if (! shiftPressed)
        {
            hadFinalPick_ = true;
        }
        else
        {
            // Waypoint click (i.e. not final click)
            MachGuiSoundManager::instance().playSound("gui/sounds/waypoint.wav");
        }
    }
}

bool MachGuiDeconstructCommand::constructionIsDuplicate(const MachLogConstruction* pCandidateConstruction) const
{
    bool result = false;

    ctl_pvector<MachLogConstruction>::const_iterator i
        = find(constructions_.begin(), constructions_.end(), pCandidateConstruction);

    if (i != constructions_.end())
        result = true;

    return result;
}

// virtual
bool MachGuiDeconstructCommand::canActorEverExecute(const MachActor& actor) const
{
    // Constructions can be deconstructed
    MachLog::ObjectType objectType = actor.objectType();
    return objectType == MachLog::CONSTRUCTOR;
}

// virtual
bool MachGuiDeconstructCommand::isInteractionComplete() const
{
    return hadFinalPick_;
}

// virtual
MachGui::Cursor2dType
MachGuiDeconstructCommand::cursorOnTerrain(const MexPoint3d& /*location*/, bool /*ctrlPressed*/, bool, bool)
{
    return MachGui::MENU_CURSOR;
}

// virtual
MachGui::Cursor2dType MachGuiDeconstructCommand::cursorOnActor(MachActor* pActor, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    if (pActor->objectIsConstruction())
    {
        cursor = MachGui::DECONSTRUCT_CURSOR;
    }

    return cursor;
}

// virtal
void MachGuiDeconstructCommand::typeData(MachLog::ObjectType /*objectType*/, int /*subType*/, uint /*level*/)
{
}

// virtual
bool MachGuiDeconstructCommand::doApply(MachActor* pActor, std::string*)
{
    // Create a superconstruct operation for the constructor
    auto op = std::make_unique<MachLogSuperConstructOperation>(
        &pActor->asConstructor(),
        constructions_,
        MachLogOperation::DECONSTRUCT_OPERATION);

    pActor->newOperation(std::move(op));

    ASSERT(pActor->objectIsMachine(), "Hey! That actor should have been a machine!");
    pActor->asMachine().manualCommandIssued();

    if (! hasPlayedVoiceMail())
    {
        MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::MOVE_TO_SITE);
        hasPlayedVoiceMail(true);
    }

    return true;
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiDeconstructCommand::clone() const
{
    return std::make_unique<MachGuiDeconstructCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiDeconstructCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/deconst.bmp", "gui/commands/deconst.bmp");
    return names;
}

// virtual
uint MachGuiDeconstructCommand::cursorPromptStringId() const
{
    return IDS_DECONSTRUCT_COMMAND;
}

// virtual
uint MachGuiDeconstructCommand::commandPromptStringid() const
{
    return IDS_DECONSTRUCT_START;
}

bool MachGuiDeconstructCommand::addPromptTextCommandInfo(const MachActor* pActor, std::string& prompt) const
{
    if (pActor->objectIsConstruction())
    {
        const MachLogConstruction &construction = pActor->asConstruction();
        MachPhys::BuildingMaterialUnits totalBMUs = construction.bmuValueOfHitPoints(construction.hp());
        MachPhys::BuildingMaterialUnits secondHandBMUs
            = totalBMUs * MachLogRaces::instance().stats().secondhandRefundablePercentage();

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%c%d", GuiBmpFont::bmuPointsIndex(), secondHandBMUs);

        GuiResourceString bmuText(IDS_BMUPOINTS_ON_DECONSTRUCTION, buffer);
        prompt += ", " + bmuText.asString();
        return true;
    }

    return false;
}

// virtual
bool MachGuiDeconstructCommand::canAdminApply() const
{
    return true;
}

// virtual
bool MachGuiDeconstructCommand::doAdminApply(MachLogAdministrator* pAdministrator, std::string*)
{
    PRE(canAdminApply());
    ;

    // Create an admin superconstruct operation for the administrator
    auto op = std::make_unique <MachLogAdminSuperConstructOperation>(
                  pAdministrator,
                  constructions_,
                  MachLogOperation::DECONSTRUCT_OPERATION);

    pAdministrator->newOperation(std::move(op));
    ASSERT(pAdministrator->squadron(), "Administrator didn't have a squadron!");
    pAdministrator->squadron()->manualCommandIssuedToSquadron();

    MachActor* pFirstConstructor = nullptr;

    bool found = false;
    for (MachInGameScreen::Actors::const_iterator i = inGameScreen().selectedActors().begin();
         ! found && i != inGameScreen().selectedActors().end();
         ++i)
        if ((*i)->objectType() == MachLog::CONSTRUCTOR)
        {
            found = true;
            pFirstConstructor = (*i);
        }

    ASSERT(found, "No constructor found in corral!");

    // give out voicemail
    MachLogMachineVoiceMailManager::instance().postNewMail(*pFirstConstructor, MachineVoiceMailEventID::MOVE_TO_SITE);

    return true;
}

// virtual
bool MachGuiDeconstructCommand::beNotified(
    W4dSubject* pSubject,
    W4dSubject::NotificationEvent event,
    int /*clientData*/)
{
    bool stayAttached = true;

    switch (event)
    {
        case W4dSubject::DELETED:
            {
                ctl_pvector<MachLogConstruction>::iterator i
                    = find(constructions_.begin(), constructions_.end(), pSubject);
                if (i != constructions_.end())
                {
                    // one of our constructions has been destroyed
                    stayAttached = false;
                    constructions_.erase(i);
                }
            }
            break;

        default:;
    }

    return stayAttached;
}

// virtual
void MachGuiDeconstructCommand::domainDeleted(W4dDomain*)
{
    // intentionally empty...override as necessary
}

/* End CMDDECON.CPP **************************************************/
