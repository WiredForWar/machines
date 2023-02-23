/*
 * M A C H V M A N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogMachineVoiceMailManager

    A singleton handler class cross-referencing machine type and message type
    and passing on machine-specific mail id to voicemail manager
*/

#ifndef _MACHLOG_MACHVMAN_HPP
#define _MACHLOG_MACHVMAN_HPP

#include "utility/id.hpp"

#include "mathex/point3d.hpp"

#include "machlog/vmman.hpp"
#include "machlog/vmdata.hpp"
#include "machlog/machlog.hpp"
#include "machlog/MachineVoiceMailEventID.hpp"

class MachLogMachineVoiceMailManager
// Canonical form revoked
{
public:
    //  Singleton class
    static MachLogMachineVoiceMailManager& instance();
    ~MachLogMachineVoiceMailManager();

    // Post verbal mail onto queue
    void postNewMail(const MachActor& fromActor, MachineVoiceMailEventID id);

    void CLASS_INVARIANT;

private:
    // methods deliberately revoked

    MachLogMachineVoiceMailManager(const MachLogMachineVoiceMailManager&);
    MachLogMachineVoiceMailManager& operator=(const MachLogMachineVoiceMailManager&);
    MachLogMachineVoiceMailManager();

    //-------------------------------

    VoiceMailID
    getGlobalFromMachineEvent(MachLog::ObjectType ot, int subType, MachineVoiceMailEventID id, UtlId actorId = 0);

    friend ostream& operator<<(ostream& o, const MachLogMachineVoiceMailManager& t);
};

ostream& operator<<(ostream& o, MachineVoiceMailEventID);

#endif

/* End MACHVMAN.HPP ****************************************************/
