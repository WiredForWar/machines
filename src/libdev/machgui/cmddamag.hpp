/*
 * C M D L M I N E . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiDamageCommand

    Implement interaction for the Damage command
*/

#ifndef _MACHGUI_CMDDAMAGE_HPP
#define _MACHGUI_CMDDAMAGE_HPP

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "mathex/point2d.hpp"

#include "machgui/command.hpp"

// Forward refs
class MachActor;

// orthodox canonical (revoked)
class MachGuiDamageCommand : public MachGuiCommand
{
public:
    // ctor.
    MachGuiDamageCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiDamageCommand() override;

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

    // True iff there is an administer version of the command for squadrons
    // with an administrator. Default implementation returns false.
    bool canAdminApply() const override;

    // True if the interaction for the command is complete
    bool isInteractionComplete() const override;

    /////////////////////////////////////////////////

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachGuiDamageCommand& t);

private:
    // Operation deliberately revoked
    MachGuiDamageCommand(const MachGuiDamageCommand&);
    MachGuiDamageCommand& operator=(const MachGuiDamageCommand&);
    bool operator==(const MachGuiDamageCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;

    /////////////////////////////////////////////////

    using Path = ctl_vector<MexPoint2d>;

    // Data members
    Path path_; // collection of points to drop mines on
    bool hadFinalPick_; // True when the final destination pick has been received
};

#endif

/* End CMDLOCTO.HPP **************************************************/
