/*
 * V M A I L . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "base/diag.hpp"
#include "ctl/pvector.hpp"
#include "ctl/vector.hpp"
#include "system/pathname.hpp"
#include "machlog/internal/vmail.hpp"
#include "machlog/internal/vmaili.hpp"
#include "machlog/internal/vminfo.hpp"
#include "machlog/vmman.hpp"
#include "mathex/point3d.hpp"
#include "sound/soundmix.hpp"
#include "sound/sndwavid.hpp"
#include "sound/smpparam.hpp"

MachLogVoiceMail::~MachLogVoiceMail()
{
    TEST_INVARIANT;

    CB_MachLogVoiceMail_DEPIMPL();

    if (sampleHandleValid_)
        invalidateSample();

    delete pImpl_;
}

MachLogVoiceMail::MachLogVoiceMail(VoiceMailID id)
    : pImpl_(new MachLogVoiceMailImpl())
{
    CB_MachLogVoiceMail_DEPIMPL();

    id_ = id;
}

MachLogVoiceMail::MachLogVoiceMail(VoiceMailID id, UtlId actorId)
    : pImpl_(new MachLogVoiceMailImpl())
{
    CB_MachLogVoiceMail_DEPIMPL();

    // Calculate position from actorId_
    id_ = id;
    actorId_ = actorId;
}

MachLogVoiceMail::MachLogVoiceMail(VoiceMailID id, UtlId actorId, MexPoint3d& position)
    : pImpl_(new MachLogVoiceMailImpl())
{
    CB_MachLogVoiceMail_DEPIMPL();

    id_ = id;
    actorId_ = actorId;
    position_ = position;
}

MachLogVoiceMail::MachLogVoiceMail(VoiceMailID id, MexPoint3d& position)
    : pImpl_(new MachLogVoiceMailImpl())
{
    CB_MachLogVoiceMail_DEPIMPL();

    id_ = id;
    position_ = position;
}

VoiceMailID MachLogVoiceMail::id() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return id_;
}

bool MachLogVoiceMail::hasActorId() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return actorId_ != 0;
}

UtlId MachLogVoiceMail::actorId() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return actorId_;
}

bool MachLogVoiceMail::hasPosition() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return position_.has_value();
}

MexPoint3d MachLogVoiceMail::position() const
{
    CB_MachLogVoiceMail_DEPIMPL();
    PRE(position_.has_value())

    return position_.value();
}

void MachLogVoiceMail::play()
{
    CB_MachLogVoiceMail_DEPIMPL();
    PRE(!isSampleValid())

    MachLogVoiceMailManager::MailInfoVector& availableMail = *MachLogVoiceMailManager::instance().pAvailableVEMails();
    MachLogVoiceMailInfo* info = availableMail[id_];
    SOUND_STREAM("Playing voicemail with id " << uint(id_) << std::endl);
    ASSERT(info, "Invalid info ptr");

    SndWaveformId param(info->wavName_);
    sampleHandle_ = SndMixer::instance().playSample(param);
    sampleHandleValid_ = true;
    bool internalSampleHandleIsValid = SndMixer::instance().isAllocated(sampleHandle_);

    POST(isSampleValid());

    hasStarted_ = true;
}

bool MachLogVoiceMail::isPlaying() const
{
    CB_MachLogVoiceMail_DEPIMPL();
    bool isPlaying = false;

    if (isSampleValid() && SndMixer::instance().isActive(sampleHandle_))
        isPlaying = true;

    return isPlaying;
}

bool MachLogVoiceMail::isSampleValid() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return sampleHandleValid_;
}

void MachLogVoiceMail::stop()
{
    CB_MachLogVoiceMail_DEPIMPL();
    PRE(isSampleValid());
    bool isIt = isPlaying(); // Recording prevents this call inside PRE()
    PRE(isIt);

    SndMixer::instance().stopSample(sampleHandle_);
    invalidateSample();

    isIt = isPlaying();
    POST(!isIt);
}

PhysAbsoluteTime MachLogVoiceMail::timeStamp() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return timeStamp_;
}

bool MachLogVoiceMail::hasStarted() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return hasStarted_;
}

void MachLogVoiceMail::invalidateSample()
{
    CB_MachLogVoiceMail_DEPIMPL();

    PRE(isSampleValid());

    // This call is made outside the precondition for the benefit of the playback/recording mechanism.
    // isPlaying() ends up calling a recorded function in the soundlibrary (SndMixer::isActive()),
    // and the rule is we can't do any recording during preconditions.
    bool isIt = isPlaying();
    PRE(!isIt);

    sampleHandleValid_ = false;
    SndMixer::instance().freeSampleResources(sampleHandle_);

    POST(!isSampleValid());
}

void MachLogVoiceMail::timeStamp(const PhysAbsoluteTime& newTimeStamp)
{
    CB_MachLogVoiceMail_DEPIMPL();

    timeStamp_ = newTimeStamp;
}

void MachLogVoiceMail::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogVoiceMail& t)
{

    o << "MachLogVoiceMail " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogVoiceMail " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End VMAIL.CPP ****************************************************/
