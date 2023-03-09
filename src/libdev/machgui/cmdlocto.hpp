/*
 * C M D L O C T O . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiLocateToCommand

    Implement interaction for the Attack command
*/

#ifndef _MACHGUI_CMDLOCTO_HPP
#define _MACHGUI_CMDLOCTO_HPP

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "mathex/point2d.hpp"

#include "machgui/command.hpp"

// Forward refs
class MachActor;

// orthodox canonical (revoked)
class MachGuiLocateToCommand : public MachGuiCommand
{
public:
    // ctor.
    MachGuiLocateToCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiLocateToCommand() override;

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

    // True iff there is an administer version of the command for squadrons
    // with an administrator. Default implementation returns false.
    bool canAdminApply() const override;

    // True if the interaction for the command is complete
    bool isInteractionComplete() const override;

    /////////////////////////////////////////////////
    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachGuiLocateToCommand& t);

private:
    // Operation deliberately revoked
    MachGuiLocateToCommand(const MachGuiLocateToCommand&);
    MachGuiLocateToCommand& operator=(const MachGuiLocateToCommand&);
    bool operator==(const MachGuiLocateToCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;

    // Execute the command as a sqaudron whose most intelligent administrator
    // is pAdministrator. If successful return true. Otherwise false, with
    // a prompt string indicating reason for failure in pReason.
    bool doAdminApply(MachLogAdministrator* pAdministrator, string* pReason) override;
    /////////////////////////////////////////////////

    using Path = ctl_vector<MexPoint2d>;

    // Data members
    Path path_; // collection of points to locate to
    bool hadFinalPick_; // True when the final destination pick has been received
};

#endif

/* End CMDLOCTO.HPP **************************************************/
