/*
 * C M D T R A N S . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiTransportCommand

    Implement interaction for the Transport command
*/

#ifndef _MACHGUI_CMDTRANS_HPP
#define _MACHGUI_CMDTRANS_HPP

#include "base/base.hpp"

#include "ctl/pvector.hpp"

#include "mathex/point3d.hpp"

#include "world4d/observer.hpp"

#include "machlog/rescarr.hpp"

#include "machgui/command.hpp"

// Forward refs
class MachActor;
class MachLogSmelter;

// orthodox canonical (revoked)
class MachGuiTransportCommand
    : public MachGuiCommand
    , public W4dObserver
{
public:
    // ctor.
    MachGuiTransportCommand(MachInGameScreen* pInGameScreen, bool explicitOrder = true);

    // dtor
    ~MachGuiTransportCommand() override;

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

    // already have this construction in our vector
    bool supplierIsDuplicate(const MachLogConstruction* pCandidateConstruction) const;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    void domainDeleted(W4dDomain* pDomain) override;

    void clearAndDetachFromAllSuppliers();

    /////////////////////////////////////////////////

    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachGuiTransportCommand& t);

private:
    // Operation deliberately revoked
    MachGuiTransportCommand(const MachGuiTransportCommand&);
    MachGuiTransportCommand& operator=(const MachGuiTransportCommand&);
    bool operator==(const MachGuiTransportCommand&);

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;

    /////////////////////////////////////////////////

    enum Action
    {
        PICKUP_AND_SMELT
    };

    // Data members
    Action action_; // What to do
    bool hadFinalPick_; // True when the final destination pick has been received
    MachLogResourceCarrier::Suppliers suppliers_; // The list of suppliers to send the resource carrier to
    MachLogConstruction* pSmeltingBuilding_;
    bool explicitOrder_; // if the thing was issued as a direct command rather than by
                         // means of an intelligent click.
};

#endif

/* End CMDTRANS.HPP **************************************************/
