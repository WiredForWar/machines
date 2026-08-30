/*
 * C D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "base/Diag.hpp"
#include "device/CD.hpp"
#include "device/CDHelper.hpp"
#include "device/CDPlayList.hpp"
#include "device/OggStream.hpp"
#include "system/PathName.hpp"

#include "device/DevCDImpl.hpp"

#include <optional>
#include <utility>

#include "spdlog/spdlog.h"

#include "al.h"

namespace
{
// How long the outgoing track takes to fall silent, and the incoming one to
// reach full volume.
constexpr double FadeSeconds = 0.5;
}

DevCDImpl* DevCDImpl::getInstance(DevCD* parent)
{
    return parent->pImpl_;
}

bool DevCDImpl::isStreamAudible() const
{
    if (musicStream_ == nullptr || !musicEnabled_)
    {
        return false;
    }

    ALint state = 0;
    alGetSourcei(source_, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

void DevCDImpl::captureResumePosition()
{
    if (!isStreamAudible())
    {
        return;
    }

    const double seconds = musicStream_->playheadSeconds();
    if (status_ == RANDOM)
    {
        resumeStore_.recordRandomInterruption(trackPlaying_, seconds, randomStartTrack_, randomEndTrack_);
    }
    else
    {
        resumeStore_.recordInterruption(trackPlaying_, seconds);
    }
}

void DevCDImpl::beginFadeOut()
{
    // Two tracks at once is the most the two sources can carry: a fade
    // already in flight loses its stream.
    delete fadeOutStream_;

    fadeOutStream_ = musicStream_;
    musicStream_ = nullptr;
    std::swap(source_, fadeOutSource_);

    // An interrupted fade-in falls away from wherever it had risen to.
    fadeOutStartGain_ = fadeInGain_;
    fadeOutGain_ = fadeOutStartGain_;
    fadeStart_ = std::chrono::steady_clock::now();
}

void DevCDImpl::cancelFade()
{
    delete fadeOutStream_;
    fadeOutStream_ = nullptr;
    fadeStart_.reset();
    fadeInGain_ = 1.0;
    applyGains();
}

void DevCDImpl::updateFade()
{
    if (!fadeStart_)
    {
        return;
    }

    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - *fadeStart_;
    const double progress = elapsed.count() / FadeSeconds;
    if (progress >= 1.0)
    {
        fadeInGain_ = 1.0;
        fadeOutGain_ = 0.0;
        delete fadeOutStream_;
        fadeOutStream_ = nullptr;
        fadeStart_.reset();
    }
    else
    {
        fadeInGain_ = fadeInStartGain_ + (1.0 - fadeInStartGain_) * progress;
        fadeOutGain_ = fadeOutStartGain_ * (1.0 - progress);
    }
    applyGains();
}

void DevCDImpl::applyGains()
{
    if (!musicEnabled_)
    {
        return;
    }

    const float baseGain = static_cast<float>(savedVolume_) / 100.0f;
    alSourcef(source_, AL_GAIN, baseGain * static_cast<float>(fadeInGain_));
    if (fadeOutStream_ != nullptr)
    {
        alSourcef(fadeOutSource_, AL_GAIN, baseGain * static_cast<float>(fadeOutGain_));
    }
}

// static

DevCD& DevCD::instance()
{
    static DevCD instance;
    return instance;
}

DevCD::DevCD()
    : pImpl_(new DevCDImpl())
{
    // This will enable/disable music!
    device::helper::cd::configure(this);

    ALuint& source_ = pImpl_->source_;
    unsigned int& savedVolume_ = pImpl_->savedVolume_;
    DevCDPlayList*& pPlayList_ = pImpl_->pPlayList_;
    bool& haveMixer_ = pImpl_->haveMixer_;
    MexBasicRandom& randomGenerator_ = pImpl_->randomGenerator_;
    bool& musicEnabled_ = pImpl_->musicEnabled_;

    savedVolume_ = 20;

    bool noErrors = true;

    if (musicEnabled_)
    {
        spdlog::info("Enabling the music...");

        ALenum errorCode = alGetError();
        if (errorCode == AL_NO_ERROR)
        {
            alGenSources(1, &source_);
            if (alGetError() == AL_NO_ERROR)
            {
                alGenSources(1, &pImpl_->fadeOutSource_);
            }
            errorCode = alGetError();
            if (errorCode != AL_NO_ERROR)
            {
                spdlog::warn("Failed to create OpenAL sources for music mixer! Code: {}", errorCode);
            }
        }
        else
        {
            spdlog::warn("OpenAL reports an error before we do anything. Code: {}", errorCode);
        }

        noErrors = errorCode == AL_NO_ERROR;
    }
    haveMixer_ = noErrors;
    pPlayList_ = new DevCDPlayList(numberOfTracks());

    randomGenerator_.seedFromTime();
}

DevCD::~DevCD()
{
    ALuint& source_ = pImpl_->source_;
    unsigned int& savedVolume_ = pImpl_->savedVolume_;
    DevCDPlayList*& pPlayList_ = pImpl_->pPlayList_;
    bool& musicEnabled_ = pImpl_->musicEnabled_;

    stopPlaying();

    RICHARD_STREAM("Setting vol to saved volume " << savedVolume_ << std::endl);
    volume(savedVolume_);

    if (musicEnabled_)
    {
        // Teardown order: stop the refill threads and free the decoders
        // (OggStream dtor) BEFORE deleting the sources they queue onto.
        delete pImpl_->musicStream_;
        pImpl_->musicStream_ = nullptr;
        delete pImpl_->fadeOutStream_;
        pImpl_->fadeOutStream_ = nullptr;

        alDeleteSources(1, &source_);
        alDeleteSources(1, &pImpl_->fadeOutSource_);
    }

    delete pPlayList_;
    delete pImpl_;
}

void DevCD::update()
{
    pImpl_->updateFade();

    if (pImpl_->needsUpdate_)
    {
        handleMessages(DevCD::SUCCESS, 0);
        pImpl_->needsUpdate_ = false;
    }
}

bool DevCD::isPlayingAudioCd() const
{
    ALuint& source_ = pImpl_->source_;
    bool& musicEnabled_ = pImpl_->musicEnabled_;

    if (musicEnabled_)
    {
        ALint sourceState;
        alGetSourcei(source_, AL_SOURCE_STATE, &sourceState);
        return sourceState == AL_PLAYING;
    }

    return false;
}

bool DevCD::supportsVolumeControl() const
{
    bool& haveMixer_ = pImpl_->haveMixer_;
    return haveMixer_;
}

Volume DevCD::volume() const
{
    Volume percentageVolume = 0;
    if (supportsVolumeControl())
    {
        unsigned int& savedVolume_ = pImpl_->savedVolume_;
        percentageVolume = savedVolume_;
        RICHARD_STREAM("Current percentage vol " << percentageVolume << std::endl);
    }
    return percentageVolume;
}

void DevCD::volume(Volume newLevel)
{
    if (supportsVolumeControl())
    {
        unsigned int& savedVolume_ = pImpl_->savedVolume_;
        ALuint& source_ = pImpl_->source_;
        bool& musicEnabled_ = pImpl_->musicEnabled_;

        if (newLevel > 100)
        {
            newLevel = 100;
        }
        savedVolume_ = newLevel;

        pImpl_->applyGains();
        RICHARD_STREAM("NewVolume set to " << volume() << std::endl);
    }
}

DevCDTrackIndex DevCD::currentTrackIndex() const
{
    PRE(isPlayingAudioCd());
    return pImpl_->trackPlaying_;
}

DevCDTrackIndex DevCD::numberOfTracks() const
{
    return 10 + 1; // Hardcoded number
}

Seconds DevCD::currentTrackLengthInSeconds() const
{
    PRE(isPlayingAudioCd());
    // Not implemented
    return 0;
}

Seconds DevCD::currentTrackRunningTime() const
{
    PRE(isPlayingAudioCd());
    ASSERT(false, "Function not implemented");
    return 0;
}

Seconds DevCD::currentTrackTimeRemaining() const
{
    PRE(isPlayingAudioCd());
    ASSERT(false, "Function not implemented");
    return 0;
}

void DevCD::play()
{
    bool& musicEnabled_ = pImpl_->musicEnabled_;

    if (musicEnabled_)
    {
        play(1);
    }
}

void DevCD::playFrom(DevCDTrackIndex track)
{
    PRE(track >= 0 && track < numberOfTracks());
    play(track);
}

void DevCD::play(DevCDTrackIndex track, bool repeat /* = false */)
{
    PRE(track >= 0 && track < numberOfTracks());

    DevCDImpl* pImpl = pImpl_;

    const bool muted = !pImpl->musicEnabled_ || pImpl->savedVolume_ <= 0;

    pImpl->captureResumePosition();

    if (!muted && pImpl->isStreamAudible())
    {
        pImpl->beginFadeOut();
    }
    else
    {
        // Tear down any previous stream (stops its refill thread) before
        // starting a new one on the same source.
        delete pImpl->musicStream_;
        pImpl->musicStream_ = nullptr;
    }

    pImpl->trackPlaying_ = track;

    if (muted)
    {
        return;
    }

    char fileName[40];
    snprintf(fileName, sizeof(fileName), "sounds/music/track%d.ogg", track);
    SysPathName filePath(fileName);

    const std::optional<double> resumeSeconds = pImpl->resumeStore_.takeResumeSeconds(track);

    OggStream* stream = new OggStream(pImpl->source_, [pImpl]() { pImpl->needsUpdate_ = true; });
    if (!stream->open(filePath.pathname(), resumeSeconds.value_or(0.0)))
    {
        std::cerr << "Could not load " << filePath.pathname() << std::endl;
        delete stream;
        return;
    }
    pImpl->musicStream_ = stream;

    // A resumed track comes up quietly under the fading one, masking the
    // rewound repeat. A track starting from the top has nothing to mask and
    // plays at full volume from its first note.
    const bool resuming = pImpl->fadeOutStream_ != nullptr && resumeSeconds.value_or(0.0) > 0.0;
    pImpl->fadeInStartGain_ = resuming ? 0.0 : 1.0;
    pImpl->fadeInGain_ = pImpl->fadeInStartGain_;
    pImpl->applyGains();
    stream->play();

    pImpl->status_ = repeat ? REPEAT : SINGLE;
}

void DevCD::play(const DevCDPlayList& params)
{
    PlayStatus& status_ = pImpl_->status_;
    DevCDPlayList*& pPlayList_ = pImpl_->pPlayList_;

    // Naughty and evil, replace with a copy construction
    *pPlayList_ = params;
    pPlayList_->reset();
    play(pPlayList_->firstTrack());

    status_ = PROGRAMMED;
}

void DevCD::stopPlaying()
{
    if (!pImpl_->musicEnabled_)
    {
        return;
    }

    pImpl_->captureResumePosition();
    pImpl_->cancelFade();

    if (pImpl_->musicStream_ != nullptr)
    {
        pImpl_->musicStream_->stop();
    }
}

void DevCD::forgetResumePositions()
{
    pImpl_->resumeStore_.clear();
}

void DevCD::handleMessages(CDMessage message, unsigned int devID)
{
    PlayStatus& status_ = pImpl_->status_;
    DevCDTrackIndex& trackPlaying_ = pImpl_->trackPlaying_;
    DevCDPlayList*& pPlayList_ = pImpl_->pPlayList_;
    MexBasicRandom& randomGenerator_ = pImpl_->randomGenerator_;
    DevCDTrackIndex& randomStartTrack_ = pImpl_->randomStartTrack_;
    DevCDTrackIndex& randomEndTrack_ = pImpl_->randomEndTrack_;

    switch (message)
    {
        case ABORT:
            break;

        case FAIL:
            break;

        case SUCCESS:
            {
                if (status_ == PROGRAMMED)
                {
                    if (! pPlayList_->isFinished())
                    {
                        play(pPlayList_->nextTrack());
                    }
                }
                else if (status_ == REPEAT)
                {
                    play(trackPlaying_, true);
                }
                else if (status_ == RANDOM)
                {
                    if (randomStartTrack_ < numberOfTracks())
                    {
                        // Make sure we're not asking it to randomise a number outside the range of tracks
                        // on the CD.
                        DevCDTrackIndex tmpEndTrack = std::min(numberOfTracks(), randomEndTrack_);

                        if (randomStartTrack_ < tmpEndTrack)
                        {
                            // Make sure we don't play the same track twice (unless it is the only track)
                            DevCDTrackIndex trackToPlay = trackPlaying_;
                            while (trackToPlay == trackPlaying_
                                   && (trackToPlay != randomStartTrack_ || trackToPlay != tmpEndTrack))
                            {
                                trackToPlay = mexRandomInt(&randomGenerator_, randomStartTrack_, tmpEndTrack);
                            }
                            play(trackToPlay);
                            status_ = RANDOM; // 'play' sets the status_ to SINGLE
                        }
                    }
                }
                break;
            }

        case SUPERSEDED:
            break;

        case UNKNOWN:
            break;

        default:
            break;
    }
}

bool DevCD::isAudioCDPresent()
{
    unsigned int& savedVolume_ = pImpl_->savedVolume_;
    bool& musicEnabled_ = pImpl_->musicEnabled_;

    // If music is muted then just say no
    if (! musicEnabled_ || savedVolume_ <= 0)
    {
        return false;
    }

    return true;
}

void DevCD::enableMusic()
{
    pImpl_->musicEnabled_ = true;
}
void DevCD::disableMusic()
{
    pImpl_->musicEnabled_ = false;
}

std::ostream& operator<<(std::ostream& o, const DevCD& devCD)
{
    o << "Number of tracks " << devCD.numberOfTracks() << "\n"
      << "Current Track " << devCD.currentTrackIndex() << "\n"
      << "Track time " << devCD.currentTrackLengthInSeconds() << "\n"
      << "Track running time " << devCD.currentTrackRunningTime() << "\n"
      << "Track remaining time " << devCD.currentTrackTimeRemaining() << std::endl;

    return o;
}

void DevCD::randomPlay(DevCDTrackIndex startTrack, DevCDTrackIndex endTrack, DevCDTrackIndex firstTrack /*= -1*/)
{
    PlayStatus& status_ = pImpl_->status_;
    DevCDTrackIndex& randomStartTrack_ = pImpl_->randomStartTrack_;
    DevCDTrackIndex& randomEndTrack_ = pImpl_->randomEndTrack_;
    MexBasicRandom& randomGenerator_ = pImpl_->randomGenerator_;

    PRE(startTrack >= 0);
    PRE(startTrack <= endTrack);

    randomStartTrack_ = startTrack;
    randomEndTrack_ = endTrack + 1;

    // A rotation over the same range that was interrupted continues with the
    // track that was playing, not with 'firstTrack' again.
    const std::optional<DevCDTrackIndex> resumeTrack
        = pImpl_->resumeStore_.takeRandomTrack(randomStartTrack_, randomEndTrack_);
    if (resumeTrack.has_value())
    {
        play(*resumeTrack);
        status_ = RANDOM;
        return;
    }

    if (firstTrack != -1)
    {
        play(firstTrack);
    }
    else
    {
        if (randomStartTrack_ < numberOfTracks())
        {
            // Make sure we're not asking it to randomise a number outside the range of tracks
            // on the CD.
            DevCDTrackIndex tmpEndTrack = std::min(numberOfTracks(), randomEndTrack_);

            if (randomStartTrack_ < tmpEndTrack)
            {
                DevCDTrackIndex trackToPlay = mexRandomInt(&randomGenerator_, randomStartTrack_, tmpEndTrack);
                play(trackToPlay);
            }
        }
    }

    status_ = RANDOM;
}

/* End CD.CPP *******************************************************/
