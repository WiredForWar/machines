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

#include "spdlog/spdlog.h"

#include "al.h"

DevCDImpl* DevCDImpl::getInstance(DevCD* parent)
{
    return parent->pImpl_;
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
            errorCode = alGetError();
            if (errorCode != AL_NO_ERROR)
            {
                spdlog::warn("Failed to create OpenAL source for music mixer! Code: {}", errorCode);
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
        // Teardown order: stop the refill thread and free the decoder (OggStream
        // dtor) BEFORE deleting the source it queues onto.
        delete pImpl_->musicStream_;
        pImpl_->musicStream_ = nullptr;

        alDeleteSources(1, &source_);
    }

    delete pPlayList_;
    delete pImpl_;
}

void DevCD::update()
{
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

        if (musicEnabled_)
        {
            ALfloat fVol = (float)(savedVolume_) / 100.0f; // Maybe use log model instead of linear?
            alSourcef(source_, AL_GAIN, fVol);
        }
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
    ALuint& source_ = pImpl_->source_;
    PlayStatus& status_ = pImpl_->status_;
    DevCDTrackIndex& trackPlaying_ = pImpl_->trackPlaying_;
    unsigned int& savedVolume_ = pImpl_->savedVolume_;
    bool& musicEnabled_ = pImpl_->musicEnabled_;

    PRE(track >= 0 && track < numberOfTracks());

    trackPlaying_ = track;

    // Tear down any previous stream (stops its refill thread) before starting
    // a new one on the shared source.
    delete pImpl_->musicStream_;
    pImpl_->musicStream_ = nullptr;

    if (! musicEnabled_ || savedVolume_ <= 0) // Muted
    {
        return;
    }

    char fileName[40];
    snprintf(fileName, sizeof(fileName), "sounds/music/track%d.ogg", trackPlaying_);
    SysPathName filePath(fileName);

    DevCDImpl* pImpl = pImpl_;
    auto* stream = new OggStream(source_, [pImpl]() { pImpl->needsUpdate_ = true; });
    if (!stream->open(filePath.pathname()))
    {
        std::cerr << "Could not load " << filePath.pathname() << std::endl;
        delete stream;
        return;
    }
    pImpl_->musicStream_ = stream;

    ALfloat fVol = (float)(savedVolume_) / 100.0f;
    alSourcef(source_, AL_GAIN, fVol);
    stream->play();

    if (repeat)
    {
        status_ = REPEAT;
    }
    else
    {
        status_ = SINGLE;
    }
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
    bool& musicEnabled_ = pImpl_->musicEnabled_;

    if (musicEnabled_ && pImpl_->musicStream_ != nullptr)
    {
        pImpl_->musicStream_->stop();
    }
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
