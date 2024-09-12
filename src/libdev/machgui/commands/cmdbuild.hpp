/*
 * C M D B U I L D . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiBuildCommand

    Implement interaction for the Build command
*/

#ifndef _MACHGUI_CMDBUILD_HPP
#define _MACHGUI_CMDBUILD_HPP

#include "machgui/commands/command.hpp"

#include "base/base.hpp"
#include "mathex/point3d.hpp"
#include "machgui/gui.hpp"

// Forward refs
class MachActor;

// orthodox canonical (revoked)
class MachGuiBuildCommand : public MachGuiCommand
{
public:
    // ctor.
    MachGuiBuildCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiBuildCommand() override;

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Return new instance of this command
    std::unique_ptr<MachGuiCommand> clone() const override;

    // The resource file id of the prompt to be displayed when the cursor moves
    // over the command icon
    uint cursorPromptStringId() const override;

    // The resource file id of the prompt to be displayed when the command is clicked
    uint commandPromptStringid() const override;

    void start() override;
    void finish() override;

    // reference to the up and down bitmap names for this command
    const std::pair<std::string, std::string>& iconNames() const override;

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
    bool processButtonEvent(const GuiKeyEvent& event) override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiBuildCommand& t);

private:
    // Operation deliberately revoked
    MachGuiBuildCommand(const MachGuiBuildCommand&);
    MachGuiBuildCommand& operator=(const MachGuiBuildCommand&);
    bool operator==(const MachGuiBuildCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, std::string* pReason) override;

    /////////////////////////////////////////////////

    // Data members
    MachGui::ControlPanelContext previousMenuContext_;
};

#endif

/* End CMDBUILD.HPP **************************************************/
