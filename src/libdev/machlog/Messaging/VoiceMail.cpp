/*
 * V M A I L . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "base/Diag.hpp"
#include "ctl/PtrVector.hpp"
#include "ctl/Vector.hpp"
#include "system/PathName.hpp"
#include "machlog/Internal/VoiceMail.hpp"
#include "machlog/Internal/VoiceMailImpl.hpp"
#include "machlog/Internal/VoiceMailInfo.hpp"
#include "mathex/Point3d.hpp"
#include "sound/Mixer.hpp"
#include "sound/WaveformId.hpp"
#include "sound/SampleParameters.hpp"

MachLogVoiceMail::~MachLogVoiceMail()
{
    TEST_INVARIANT;

    CB_MachLogVoiceMail_DEPIMPL();

    invalidateSample();

    delete pImpl_;
}

MachLogVoiceMail::MachLogVoiceMail(const MachLogVoiceMailInfo &info)
    : pImpl_(new MachLogVoiceMailImpl(info))
{
}

MachLogVoiceMail::MachLogVoiceMail(const MachLogVoiceMailInfo &info, UtlId actorId)
    : pImpl_(new MachLogVoiceMailImpl(info))
{
    CB_MachLogVoiceMail_DEPIMPL();

    // Calculate position from actorId_
    actorId_ = actorId;
}

MachLogVoiceMail::MachLogVoiceMail(const MachLogVoiceMailInfo &info, UtlId actorId, MexPoint3d& position)
    : pImpl_(new MachLogVoiceMailImpl(info))
{
    CB_MachLogVoiceMail_DEPIMPL();

    actorId_ = actorId;
    position_ = position;
}

MachLogVoiceMail::MachLogVoiceMail(const MachLogVoiceMailInfo &info, MexPoint3d& position)
    : pImpl_(new MachLogVoiceMailImpl(info))
{
    CB_MachLogVoiceMail_DEPIMPL();

    position_ = position;
}

VoiceMailID MachLogVoiceMail::id() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return info_.id_;
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

    SOUND_STREAM("Playing voicemail with id " << uint(info_.id_) << std::endl);

    SndWaveformId param(info_.wavName_);
    sampleHandle_ = SndMixer::instance().playSample(param);

    // A mail the mixer had no channel for never becomes valid. It has still
    // started, so the manager retires it on its next sweep rather than waiting
    // on a sample that will never play.
    hasStarted_ = true;
}

bool MachLogVoiceMail::isPlaying() const
{
    CB_MachLogVoiceMail_DEPIMPL();
    bool isPlaying = false;

    if (isSampleValid() && SndMixer::instance().isActive(sampleHandle_.value()))
        isPlaying = true;

    return isPlaying;
}

bool MachLogVoiceMail::isSampleValid() const
{
    CB_MachLogVoiceMail_DEPIMPL();

    return sampleHandle_.has_value();
}

void MachLogVoiceMail::stop()
{
    CB_MachLogVoiceMail_DEPIMPL();
    PRE(isSampleValid());
    bool isIt = isPlaying(); // Recording prevents this call inside PRE()
    PRE(isIt);

    SndMixer::instance().stopSample(sampleHandle_.value());
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

    // A mail that never got a channel has nothing to release.
    if (!isSampleValid())
        return;

    // This call is made outside the precondition for the benefit of the playback/recording mechanism.
    // isPlaying() ends up calling a recorded function in the soundlibrary (SndMixer::isActive()),
    // and the rule is we can't do any recording during preconditions.
    bool isIt = isPlaying();
    PRE(!isIt);

    SndMixer::instance().freeSampleResources(sampleHandle_.value());
    sampleHandle_.reset();

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
