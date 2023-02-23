/*
 * C M D D E P L O . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiDeployCommand

    Implement interaction for the Deploy command
*/

#ifndef _MACHGUI_CMDDEPLO_HPP
#define _MACHGUI_CMDDEPLO_HPP

#include "base/base.hpp"
#include "mathex/point3d.hpp"

#include "machgui/command.hpp"

// Forward refs
class MachActor;

// orthodox canonical (revoked)
class MachGuiDeployCommand : public MachGuiCommand
{
public:
    // ctor.
    MachGuiDeployCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiDeployCommand() override;

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Return new instance of this command
    MachGuiCommand* clone() const override;

    // The resource file id of the prompt to be displayed when the cursor moves
    // over the command icon
    uint cursorPromptStringId() const override;

    // The resource file id of the prompt to be displayed when the command is clicked
    uint commandPromptStringid() const override;

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

    // True if the interaction for the command is complete
    bool isInteractionComplete() const override;

    /////////////////////////////////////////////////
    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachGuiDeployCommand& t);

private:
    // Operation deliberately revoked
    MachGuiDeployCommand(const MachGuiDeployCommand&);
    MachGuiDeployCommand& operator=(const MachGuiDeployCommand&);
    bool operator==(const MachGuiDeployCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;

    /////////////////////////////////////////////////

    // Data members
    MexPoint3d location_; // Location to deploy the contents
    bool hadFinalPick_; // true when the location has been picked
};

#endif

/* End CMDDEPLO.HPP **************************************************/
