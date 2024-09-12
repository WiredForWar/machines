/*
 * C M D F R M S Q . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmdfrmsq.hpp"

#include "gui/event.hpp"
#include "machgui/ingame.hpp"
#include "machgui/internal/strings.hpp"
#include "device/butevent.hpp"
#include "machlog/actor.hpp"
#include "machlog/machlog.hpp"

MachGuiFormSquadronCommand::MachGuiFormSquadronCommand(MachInGameScreen* pInGameScreen)
    : MachGuiDefaultCommand(pInGameScreen)
{
    TEST_INVARIANT;
}

MachGuiFormSquadronCommand::~MachGuiFormSquadronCommand()
{
    TEST_INVARIANT;
}

void MachGuiFormSquadronCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiFormSquadronCommand& t)
{

    o << "MachGuiFormSquadronCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiFormSquadronCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiFormSquadronCommand::canActorEverExecute(const MachActor& actor) const
{
    // Machines can be formed into squadrons
    return actor.objectIsMachine() && inGameScreen().currentContext() == MachGui::MAIN_MENU;
}

// virtual
bool MachGuiFormSquadronCommand::isInteractionComplete() const
{
    return false;
}

// virtual
bool MachGuiFormSquadronCommand::doApply(MachActor*, std::string*)
{
    // This command interaction is all via the menu icons, so this function is not called
    return true;
}

// virtual
void MachGuiFormSquadronCommand::typeData(MachLog::ObjectType, int, uint)
{
    // Do nothing
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiFormSquadronCommand::clone() const
{
    return std::make_unique<MachGuiFormSquadronCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiFormSquadronCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/formsqad.bmp", "gui/commands/formsqad.bmp");
    return names;
}

void MachGuiFormSquadronCommand::start()
{
    MachGuiCommand::start();
    // Cache the current control panel context
    previousMenuContext_ = inGameScreen().currentContext();

    // Enter the form squadron command context
    inGameScreen().currentContext(MachGui::FORM_SQUADRON_COMMAND);
}

void MachGuiFormSquadronCommand::finish()
{
    MachGuiCommand::finish();
    // If still in the form squadron command context, restore the cached context
    if (inGameScreen().currentContext() == MachGui::FORM_SQUADRON_COMMAND)
        inGameScreen().currentContext(previousMenuContext_);
}

// virtual
uint MachGuiFormSquadronCommand::cursorPromptStringId() const
{
    return IDS_FORM_SQUADRON_COMMAND;
}

// virtual
uint MachGuiFormSquadronCommand::commandPromptStringid() const
{
    return IDS_FORM_SQUADRON_START;
}

// virtual
bool MachGuiFormSquadronCommand::processButtonEvent(const GuiKeyEvent& event)
{
    const DevButtonEvent& be = event.buttonEvent();
    if (isVisible() && be.scanCode() == Device::KeyCode::KEY_F && be.action() == DevButtonEvent::PRESS && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

// virtual
bool MachGuiFormSquadronCommand::actorsCanExecute() const
{
    return MachGuiCommand::actorsCanExecute();
}

/* End CMDFRMSQ.CPP **************************************************/
