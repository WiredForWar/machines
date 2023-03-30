/*
 * I N T C U R A C . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiIntelligentCursorOnActor

    Works out what the intelligent cursor id should be for a given selected actor,
    when the cursor is over a second actor.
*/

#ifndef _MACHGUI_INTCURAC_HPP
#define _MACHGUI_INTCURAC_HPP

#include "base/base.hpp"
#include "machgui/restorer.hpp"
#include "machgui/gui.hpp"

// Forward refs
class MachActor;
class MachInGameScreen;

// orthodox canonical (revoked)
class MachGuiIntelligentCursorOnActor : public MachLogTypeRestorer
{
public:
    // ctor.
    MachGuiIntelligentCursorOnActor();

    // dtor.
    ~MachGuiIntelligentCursorOnActor() override;

    // returns the intelligent cursor to be displayed for the action to be performed
    // by pSelectedActor if a click is made on pActorUnderCursor.
    MachGui::Cursor2dType cursorType(
        MachActor* pSelectedActor,
        MachActor* pActorUnderCursor,
        MachInGameScreen* pInGameScreen,
        bool ctrlPressed,
        bool shiftPressed,
        bool altPressed);

protected:
    ////////////////////////////////////////////////////////////
    // Inherited from MachGuiTypeRestorer

    ProcessAsSubtype doProcessConstruction(MachLogConstruction*) override;
    ProcessAsSubtype doProcessMachine(MachLogMachine*) override;
    ProcessAsSubtype doDefaultProcess(MachActor*) override;

    ProcessAsSubtype doProcessAdministrator(MachLogAdministrator*) override;
    ProcessAsSubtype doProcessAggressor(MachLogAggressor*) override;
    ProcessAsSubtype doProcessConstructor(MachLogConstructor*) override;
    ProcessAsSubtype doProcessGeoLocator(MachLogGeoLocator*) override;
    ProcessAsSubtype doProcessSpyLocator(MachLogSpyLocator*) override;
    ProcessAsSubtype doProcessTechnician(MachLogTechnician*) override;
    ProcessAsSubtype doProcessPersonnelCarrier(MachLogAPC*) override;
    ProcessAsSubtype doProcessResourceCarrier(MachLogResourceCarrier*) override;

    ProcessAsSubtype doProcessLocator(MachLogMachine*) override;
    ProcessAsSubtype doProcessTransporter(MachLogMachine*) override;

    ////////////////////////////////////////////////////////////

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiIntelligentCursorOnActor& t);

private:
    // Operations deliberately revoked
    MachGuiIntelligentCursorOnActor(const MachGuiIntelligentCursorOnActor&);
    MachGuiIntelligentCursorOnActor& operator=(const MachGuiIntelligentCursorOnActor&);
    bool operator==(const MachGuiIntelligentCursorOnActor&);

    // Do standard machine processing
    void processStandardMachine(MachLogMachine* pActor);

    // bool canAtLeastOneCorralActorEnterConstruction( const MachLogConstruction& );

    // Data members
    MachActor* pCursorActor_; // The actor under the cursor
    MachGui::Cursor2dType useCursor_; // Set as the cursor type to use
    bool ctrlPressed_;
    bool shiftPressed_;
    bool altPressed_;
    MachInGameScreen* pInGameScreen_;
};

#endif

/* End INTCURAC.HPP *************************************************/
