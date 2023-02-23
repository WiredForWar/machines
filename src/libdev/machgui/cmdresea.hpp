/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiResearchCommand

    Implement interaction for the ResearchLevel command
*/

#ifndef _MACHGUI_CMDRESEA_HPP
#define _MACHGUI_CMDRESEA_HPP

#include "base/base.hpp"
#include "mathex/point3d.hpp"

#include "machgui/command.hpp"
#include "machgui/gui.hpp"

// Forward refs
class MachActor;

// orthodox canonical (revoked)
class MachGuiResearchCommand : public MachGuiCommand
{
public:
    // ctor.
    MachGuiResearchCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiResearchCommand() override;

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

    // Respond to pick at location on the terrain.
    // ctrl/shift/altPressed indicate which modifier keys were pressed at the time.
    void pickOnTerrain(const MexPoint3d& location, bool ctrlPressed, bool shiftPressed, bool altPressed) override;

    // respond to pick on pActor in the world view window.
    // ctrl/shift/altPressed indicate which modifier keys were pressed at the time.
    void pickOnActor(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed) override;

    // Respond to cursor at location on the terrain.
    // ctrl/shift/altPressed indicate which modifier keys are pressed.
    // Returns 2d cursor to be displayed.
    MachGui::Cursor2dType
    cursorOnTerrain(const MexPoint3d& location, bool ctrlPressed, bool shiftPressed, bool altPressed) override;

    // Respond to cursor on pActor in the world view window.
    // ctrl/shift/altPressed indicate which modifier keys are pressed.
    // Returns 2d cursor to be displayed.
    MachGui::Cursor2dType
    cursorOnActor(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed) override;

    // Set the type data associated with the command.
    void typeData(MachLog::ObjectType objectType, int subType, uint level) override;

    // True if this is a kind of command that actor can ever execute.
    bool canActorEverExecute(const MachActor& actor) const override;

    // True if this command should be visible based on selected actor set.
    // Ignores actor race in determining result.
    bool actorsCanExecute() const override;

    // True if the interaction for the command is complete
    bool isInteractionComplete() const override;

    /////////////////////////////////////////////////
    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachGuiResearchCommand& t);

private:
    // Operation deliberately revoked
    MachGuiResearchCommand(const MachGuiResearchCommand&);
    MachGuiResearchCommand& operator=(const MachGuiResearchCommand&);
    bool operator==(const MachGuiResearchCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;

    /////////////////////////////////////////////////

    // Data members
    MachGui::ControlPanelContext previousMenuContext_;
};

#endif

/* End **************************************************/
