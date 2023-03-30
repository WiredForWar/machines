/*
 * C M D D E F A U . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiDefaultCommand

    Implement interaction for the Default command.
    This includes intelligent cursor, actor selection etc.
*/

#ifndef _MACHGUI_CMDDEFAU_HPP
#define _MACHGUI_CMDDEFAU_HPP

#include "base/base.hpp"

#include "machgui/command.hpp"

// Forward refs
class MachGuiIntelligentCursorOnActor;

// orthodox canonical (revoked)
class MachGuiDefaultCommand : public MachGuiCommand
{
public:
    // ctor.
    MachGuiDefaultCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiDefaultCommand() override;

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

    // True if this command should be visible based on selected actor set.
    // Ignores actor race in determining result.
    bool actorsCanExecute() const override;

    void selectActors(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed);

    // True if this is a kind of command that actor can ever execute.
    bool canActorEverExecute(const MachActor& actor) const override;

    // True if the interaction for the command is complete
    bool isInteractionComplete() const override;

    /////////////////////////////////////////////////

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiDefaultCommand& t);

private:
    // Operation deliberately revoked
    MachGuiDefaultCommand(const MachGuiDefaultCommand&);
    MachGuiDefaultCommand& operator=(const MachGuiDefaultCommand&);
    bool operator==(const MachGuiDefaultCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;

    /////////////////////////////////////////////////

    // Data members
    // Performs intelligent cursor mapping when cursor is on an actor
    MachGuiIntelligentCursorOnActor* pIntelligentCursorOnActor_;
    MachActor* pLastSelectedActor_ = nullptr;
    double actorSelectedTime_ = 0;
};

#endif

/* End CMDDEFAU.HPP **************************************************/
