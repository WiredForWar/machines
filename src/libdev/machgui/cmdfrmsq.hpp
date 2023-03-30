/*
 * C M D F R M S Q . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiFormSquadronCommand

    Implement interaction for the FormSquadron command
*/

#ifndef _MACHGUI_CMDFRMSQ_HPP
#define _MACHGUI_CMDFRMSQ_HPP

#include "base/base.hpp"
#include "mathex/point3d.hpp"
#include "machgui/cmddefau.hpp"
#include "machgui/gui.hpp"

// Forward refs
class MachActor;

// orthodox canonical (revoked)
class MachGuiFormSquadronCommand : public MachGuiDefaultCommand
{
public:
    // ctor.
    MachGuiFormSquadronCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiFormSquadronCommand() override;

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Return new instance of this command
    MachGuiCommand* clone() const override;

    // The resource file id of the prompt to be displayed when the cursor moves
    // over the command icon
    uint cursorPromptStringId() const override;

    // The resource file id of the prompt to be displayed when the command is clicked
    uint commandPromptStringid() const override;

    // Called when the command becomes active
    // Default override does nothing
    void start() override;

    // Called when the command ceases to be active
    // Default override does nothing
    void finish() override;

    // reference to the up and down bitmap names for this command
    const std::pair<string, string>& iconNames() const override;

    // Set the type data associated with the command.
    void typeData(MachLog::ObjectType objectType, int subType, uint level) override;

    // True if this is a kind of command that actor can ever execute.
    bool canActorEverExecute(const MachActor& actor) const override;

    // True if the interaction for the command is complete
    bool isInteractionComplete() const override;

    bool actorsCanExecute() const override;

    /////////////////////////////////////////////////
    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiFormSquadronCommand& t);

private:
    // Operation deliberately revoked
    MachGuiFormSquadronCommand(const MachGuiFormSquadronCommand&);
    MachGuiFormSquadronCommand& operator=(const MachGuiFormSquadronCommand&);
    bool operator==(const MachGuiFormSquadronCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;

    /////////////////////////////////////////////////

    // Data members
    MachGui::ControlPanelContext previousMenuContext_;
};

#endif

/* End CMDFRMSQ.HPP **************************************************/
