/*
 * C M D S C A V . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiScavengeCommand

    Implement interaction for the Scavenge command
*/

#ifndef _MACHGUI_CMDSCAV_HPP
#define _MACHGUI_CMDSCAV_HPP

#include "base/base.hpp"

#include "ctl/pvector.hpp"

#include "mathex/point3d.hpp"

#include "world4d/observer.hpp"

#include "machgui/command.hpp"

// Forward refs
class MachActor;
class MachLogDebris;

// orthodox canonical (revoked)
class MachGuiScavengeCommand
    : public MachGuiCommand
    , public W4dObserver
{
public:
    // ctor.
    MachGuiScavengeCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiScavengeCommand() override;

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

    // already have this debris in our vector
    bool debrisIsDuplicate(const MachLogDebris* pCandidateDebris) const;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    void domainDeleted(W4dDomain* pDomain) override;

    /////////////////////////////////////////////////
    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiScavengeCommand& t);

private:
    // Operation deliberately revoked
    MachGuiScavengeCommand(const MachGuiScavengeCommand&);
    MachGuiScavengeCommand& operator=(const MachGuiScavengeCommand&);
    bool operator==(const MachGuiScavengeCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;

    /////////////////////////////////////////////////

    enum Action
    {
        SCAVENGE
    };

    // Apply the actions
    bool applyScavengeObject(MachActor* pActor, string* pReason);

    using Suppliers = ctl_pvector<MachLogDebris>;
    // Data members
    Action action_; // What to do
    bool hadFinalPick_; // True when the final destination pick has been received
    Suppliers suppliers_; // The list of suppliers to send the resource carrier to
};

#endif

/* End CMDATTAC.HPP **************************************************/
