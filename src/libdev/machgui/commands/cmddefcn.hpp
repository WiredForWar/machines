/*
 * C M D D E F C N . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiDefconCommand

    Implement interaction for the Construct command
*/

#ifndef _MACHGUI_CMDDEFCN_HPP
#define _MACHGUI_CMDDEFCN_HPP

#include "machgui/commands/command.hpp"

#include "base/base.hpp"
#include "machgui/gui.hpp"

#include "ctl/pvector.hpp"

// Forward refs
class MachLogAdministrator;

// orthodox canonical (revoked)
class MachGuiDefconCommand : public MachGuiCommand
{
public:
    // ctor.
    MachGuiDefconCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiDefconCommand() override;

    using Actors = ctl_pvector<MachActor>;

    // Work out which lights are on
    void update(const Actors&);

    static bool defconLow();
    static bool defconNormal();
    static bool defconHigh();

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Return new instance of this command
    std::unique_ptr<MachGuiCommand> clone() const override;

    // The resource file id of the prompt to be displayed when the cursor moves
    // over the command icon
    uint cursorPromptStringId() const override;

    // The resource file id of the prompt to be displayed when the command is clicked
    uint commandPromptStringid() const override;

    // reference to the up and down bitmap names for this command
    const std::pair<std::string, std::string>& iconNames() const override;

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

    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    // ensures that the defcon moves from NORMAL to HIGH rather than to LOW
    void resetDirectionFromDefConNormal();

private:
    // Operation deliberately revoked
    MachGuiDefconCommand(const MachGuiDefconCommand&);
    MachGuiDefconCommand& operator=(const MachGuiDefconCommand&);
    bool operator==(const MachGuiDefconCommand&);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiDefconCommand& t);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, std::string* pReason) override;

    // Execute the command as a sqaudron whose most intelligent administrator
    // is pAdministrator. If successful return true. Otherwise false, with
    // a prompt std::string indicating reason for failure in pReason.
    bool doAdminApply(MachLogAdministrator* pAdministrator, std::string* pReason) override;
    // PRE( canAdminApply() );

    /////////////////////////////////////////////////
    static bool defconLow_;
    static bool defconNormal_;
    static bool defconHigh_;

    static bool goHighFromNormal_;
};

#endif

/* End CMDDEFCN.HPP **************************************************/
