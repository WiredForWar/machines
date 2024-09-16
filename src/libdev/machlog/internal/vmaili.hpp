/*
 * V M A I L I . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogVoiceMailImpl

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_VMAILI_HPP
#define _MACHLOG_VMAILI_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "mathex/point3d.hpp"
#include "sound/smphandl.hpp"
#include "machlog/vmdata.hpp"
#include "machlog/internal/vminfo.hpp"

#include <optional>

class MachLogVoiceMailImpl
// Canonical form revoked
{
private:
    friend std::ostream& operator<<(std::ostream& o, const MachLogVoiceMailImpl& t);

    MachLogVoiceMailImpl(const MachLogVoiceMailInfo& info)
        : info_(info)
    {
    }

    friend class MachLogVoiceMail;

    PhysAbsoluteTime timeStamp_ {};
    const MachLogVoiceMailInfo& info_;
    std::optional<MexPoint3d> position_;
    UtlId actorId_ {};
    SndSampleHandle sampleHandle_ {};
    bool hasStarted_ {};
    bool sampleHandleValid_ {};
};

#define CB_MachLogVoiceMail_DEPIMPL()                                                                                  \
    CB_DEPIMPL_AUTO(timeStamp_);                                                                                       \
    CB_DEPIMPL_AUTO(info_);                                                                                              \
    CB_DEPIMPL_AUTO(position_);                                                                                        \
    CB_DEPIMPL_AUTO(actorId_);                                                                                         \
    CB_DEPIMPL_AUTO(sampleHandle_);                                                                                    \
    CB_DEPIMPL_AUTO(sampleHandleValid_);                                                                               \
    CB_DEPIMPL_AUTO(hasStarted_);

#endif

/* End VMAILI.HPP ***************************************************/
