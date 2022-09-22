/*
 * C D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "base/diag.hpp"
#include "device/cd.hpp"
#include "device/cd_helper.hpp"
#include "device/cdlist.hpp"
#include "system/pathname.hpp"

#include "device/DevCDImpl.hpp"

#if !USE_ALURE
#include <AL/al.h>
#endif

#define STREAM_NUM_BUFFERS 3
#define STREAM_BUFFER_SIZE 250000
#define STREAM_UPDATE_INTERVAL 0.125f

DevCDImpl* DevCDImpl::getInstance( DevCD* parent )
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
    : pImpl_( new DevCDImpl() )
{
#if USE_ALURE
    // This will enable/disable music!
    device::helper::cd::configure(this);

    ALuint&  source_ = pImpl_-> source_;
    unsigned int&  savedVolume_ = pImpl_-> savedVolume_;
    DevCDPlayList*&  pPlayList_ = pImpl_-> pPlayList_;
    bool&  haveMixer_ = pImpl_-> haveMixer_;
    MexBasicRandom&  randomGenerator_ = pImpl_-> randomGenerator_;
    bool& musicEnabled_ =  pImpl_->musicEnabled_;

    savedVolume_ = 20;

    //    int nDevices = mixerGetNumDevs();

    bool noErrors = true;

    /*if(!alureInitDevice(nullptr, nullptr)) //Device context is created by alsound on sndmixer init
    {
        std::cerr << "Failed to open OpenAL device: " << alureGetErrorString() << std::endl;
        noErrors = false;
    }*/

    if (musicEnabled_)
    {
        alGenSources(1, &source_);
        if (alGetError() != AL_NO_ERROR)
        {
            std::cerr << "Failed to create OpenAL source for music mixer!" << std::endl;
            alureShutdownDevice();
            noErrors = false;
        }
        else
        {
            alureStreamSizeIsMicroSec(AL_TRUE);
            alureUpdateInterval(STREAM_UPDATE_INTERVAL);
        }
    }
    haveMixer_ = noErrors;
    pPlayList_ = new DevCDPlayList(numberOfTracks());

    randomGenerator_.seedFromTime();
#endif
}

DevCD::~DevCD()
{
#if USE_ALURE
    alureStream*&  stream_ = pImpl_-> stream_;
    ALuint&  source_ = pImpl_-> source_;
    unsigned int&  savedVolume_ = pImpl_-> savedVolume_;
    DevCDPlayList*&  pPlayList_ = pImpl_-> pPlayList_;
    bool& musicEnabled_ =  pImpl_->musicEnabled_;

    stopPlaying();

    RICHARD_STREAM("Setting vol to saved volume " << savedVolume_ << std::endl);
    volume(savedVolume_);

    //    mixerClose(hMixer_);

    if (musicEnabled_)
    {
        alDeleteSources(1, &source_);
        alureDestroyStream(stream_, 0, nullptr);

        alureShutdownDevice();
    }

    //    delete [] pMixerValues_;
    delete  pPlayList_ ;
    delete pImpl_;
#endif
}

void DevCD::update()
{
    if ( pImpl_->needsUpdate_ )
    {
        handleMessages(DevCD::SUCCESS, 0);
        pImpl_->needsUpdate_ = false;
        //std::cout << "music mixer update" << std::endl;
    }
}

bool DevCD::isPlayingAudioCd() const
{
#if USE_ALURE
    ALuint&  source_ = pImpl_-> source_;
    bool& musicEnabled_ =  pImpl_->musicEnabled_;

    if (musicEnabled_)
    {
        ALint sourceState;
        alGetSourcei(source_, AL_SOURCE_STATE, &sourceState);
        return sourceState == AL_PLAYING;
    }
#endif

    return false;
}

bool DevCD::supportsVolumeControl() const
{
    bool&  haveMixer_ = pImpl_-> haveMixer_;
    return haveMixer_;
}

Volume DevCD::volume() const
{
    Volume percentageVolume = 0;
    if (supportsVolumeControl())
    {
        /*HMIXER&  hMixer_ = pImpl_-> hMixer_;
        MIXERLINE&  mixerLine_ = pImpl_-> mixerLine_;
        MIXERLINECONTROLS&  mixerLineControl_ = pImpl_-> mixerLineControl_;
        MIXERCONTROL&  mixerControl_ = pImpl_-> mixerControl_;
        MIXERCONTROLDETAILS&  mixerControlDetails_ = pImpl_-> mixerControlDetails_;
        MIXERCONTROLDETAILS_UNSIGNED*&  pMixerValues_ = pImpl_-> pMixerValues_;*/;
        unsigned int&  savedVolume_ = pImpl_-> savedVolume_;

        /*HRESULT hr = mixerGetControlDetails( ( HMIXEROBJ )hMixer_, &mixerControlDetails_,
            MIXER_GETCONTROLDETAILSF_VALUE );
        if(hr == MMSYSERR_NOERROR)
        {
            RICHARD_STREAM("Got control details" << std::endl);
        }
        else
        {
            RICHARD_STREAM("Couldn't get control details" << std::endl);
        }

        float theVolume = pMixerValues_[0].dwValue;
        percentageVolume = (((float)theVolume / (float)DevCDImpl::MAX_CDVOLUME) * 100.0) + 0.5;
        RICHARD_STREAM("Current vol " << theVolume << std::endl);*/
        percentageVolume = savedVolume_;
        RICHARD_STREAM("Current percentage vol " << percentageVolume << std::endl);
    }
    return percentageVolume;
}

void DevCD::volume( Volume newLevel )
{
    if (supportsVolumeControl())
    {
        unsigned int&  savedVolume_ = pImpl_-> savedVolume_;
        ALuint&  source_ = pImpl_-> source_;
        bool& musicEnabled_ =  pImpl_->musicEnabled_;

        if (newLevel > 100)
        {
            newLevel = 100;
        }
        savedVolume_ = newLevel;

        if (musicEnabled_)
        {
            ALfloat fVol = (float) (savedVolume_) / 100.0f; // Maybe use log model instead of linear?
            alSourcef(source_, AL_GAIN, fVol);
        }
        /*unsigned int theVolume = ((float)newLevel / 100.0) * DevCDImpl::MAX_CDVOLUME;

        pMixerValues_[0].dwValue = theVolume;
        pMixerValues_[1].dwValue = theVolume;

        HRESULT hr;
        hr = mixerSetControlDetails( ( HMIXEROBJ )hMixer_, &mixerControlDetails_,
            MIXER_SETCONTROLDETAILSF_VALUE );
        if(hr == MMSYSERR_NOERROR)
        {
            RICHARD_STREAM("Percentage volume set to " << newLevel << std::endl);
            RICHARD_STREAM("Volume set to " << theVolume << std::endl);
        }
        else
        {
            RICHARD_STREAM("Couldn't set volume" << std::endl);
        }*/
        RICHARD_STREAM("NewVolume set to " << volume() << std::endl);
    }
}

DevCDTrackIndex DevCD::currentTrackIndex() const
{
    PRE( isPlayingAudioCd() );
    return pImpl_->trackPlaying_;
}

DevCDTrackIndex DevCD::numberOfTracks() const
{
    return 10 + 1; // Hardcoded number
}

Seconds DevCD::currentTrackLengthInSeconds() const
{
    PRE( isPlayingAudioCd() );
    // Not implemented
    return 0;
}

Seconds DevCD::currentTrackRunningTime() const
{
    PRE( isPlayingAudioCd() );
    ASSERT(false, "Function not implemented");
    return 0;
}

Seconds DevCD::currentTrackTimeRemaining() const
{
    PRE( isPlayingAudioCd() );
    ASSERT(false, "Function not implemented");
    return 0;
}

void DevCD::play()
{
    bool& musicEnabled_ =  pImpl_->musicEnabled_;

    if (musicEnabled_)
    {
        play(1);
    }
}

void DevCD::playFrom( DevCDTrackIndex track )
{
    PRE( track >= 0 and track < numberOfTracks() );
    play(track);
}

void eosCallback(void *unused, ALuint unused2)
{
    (void) unused;
    (void) unused2;
    //DevCD::instance().handleMessages(DevCD::SUCCESS, 0); // This called from alure thread creates little problems
    DevCDImpl* pImpl = DevCDImpl::getInstance( &DevCD::instance() );
    pImpl->needsUpdate_ = true;
    //std::cout << "Done playing track" << std::endl;
}

void DevCD::play( DevCDTrackIndex track, bool repeat /* = false */ )
{
#if USE_ALURE
    alureStream*&  stream_ = pImpl_-> stream_;
    ALuint&  source_ = pImpl_-> source_;
    PlayStatus& status_ = pImpl_->status_;
    DevCDTrackIndex& trackPlaying_ = pImpl_->trackPlaying_;
    unsigned int&  savedVolume_ = pImpl_-> savedVolume_;
    bool& musicEnabled_ =  pImpl_->musicEnabled_;

    PRE( track >= 0 and track < numberOfTracks() );

    trackPlaying_ = track;

    if (musicEnabled_ and stream_ != nullptr)
    {
        alureDestroyStream(stream_, 0, nullptr);
    }

    if (not musicEnabled_ or savedVolume_ <= 0) // Muted
    {
        return;
    }

    char fileName[40];
    sprintf(fileName, "sounds/music/track%d.ogg", trackPlaying_);
    SysPathName filePath(fileName);
    stream_ = alureCreateStreamFromFile(filePath.pathname().c_str(), STREAM_BUFFER_SIZE, 0, nullptr);

    if (stream_ == nullptr)
    {
        std::cerr << "Could not load " << filePath.pathname() << " reason: " << alureGetErrorString() << std::endl;
        return;
    }

    ALfloat fVol = (float) (savedVolume_) / 100.0f;
    alSourcef(source_, AL_GAIN, fVol);
    if (!alurePlaySourceStream(source_, stream_, STREAM_NUM_BUFFERS, 0, eosCallback, nullptr))
    {
        std::cerr << "Failed to play stream: " << alureGetErrorString() << std::endl;
        return;
    }
    //std::cout << "Playing track: " << trackPlaying_ << " volume: " << savedVolume_ << std::endl;

    if ( repeat )
    {
        status_ = REPEAT;
    }
    else
    {
        status_ = SINGLE;
    }
#endif
}

void DevCD::play( const DevCDPlayList& params )
{
    PlayStatus& status_ = pImpl_->status_;
    DevCDPlayList*&  pPlayList_ = pImpl_-> pPlayList_;

    //Naughty and evil, replace with a copy construction
    *pPlayList_ = params;
    pPlayList_->reset();
    play(pPlayList_->firstTrack());

    status_ = PROGRAMMED;
}

void DevCD::stopPlaying()
{
    ALuint&  source_ = pImpl_-> source_;
    bool& musicEnabled_ =  pImpl_->musicEnabled_;

#if USE_ALURE
    if (musicEnabled_)
    {
        alureStopSource(source_, AL_FALSE);
    }
#endif
}

void DevCD::handleMessages( CDMessage message, unsigned int devID)
{
    PlayStatus& status_ = pImpl_->status_;
    DevCDTrackIndex& trackPlaying_ = pImpl_->trackPlaying_;
    DevCDPlayList*&  pPlayList_ = pImpl_-> pPlayList_;
    MexBasicRandom&  randomGenerator_ = pImpl_-> randomGenerator_;
    DevCDTrackIndex&  randomStartTrack_ = pImpl_-> randomStartTrack_;
    DevCDTrackIndex&  randomEndTrack_ = pImpl_-> randomEndTrack_;

    switch (message)
    {
        case ABORT:
            break;

        case FAIL:
            break;

        case SUCCESS:
        {
            if ( status_ == PROGRAMMED )
            {
                if (not pPlayList_->isFinished())
                {
                    play(pPlayList_->nextTrack());
                }
            }
            else if ( status_ == REPEAT )
            {
                play( trackPlaying_, true );
            }
            else if ( status_ == RANDOM )
            {
                if ( randomStartTrack_ < numberOfTracks() )
                {
                    // Make sure we're not asking it to randomise a number outside the range of tracks
                    // on the CD.
                    DevCDTrackIndex tmpEndTrack = std::min( numberOfTracks(), randomEndTrack_ );

                    if ( randomStartTrack_ < tmpEndTrack )
                    {
                        // Make sure we don't play the same track twice (unless it is the only track)
                        DevCDTrackIndex trackToPlay = trackPlaying_;
                        while ( trackToPlay == trackPlaying_ and
                            ( trackToPlay != randomStartTrack_ or trackToPlay != tmpEndTrack ) )
                        {
                            trackToPlay = mexRandomInt( &randomGenerator_, randomStartTrack_, tmpEndTrack );
                        }
                        play( trackToPlay );
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
    unsigned int&  savedVolume_ = pImpl_-> savedVolume_;
    bool& musicEnabled_ =  pImpl_->musicEnabled_;

    // If music is muted then just say no
    if (not musicEnabled_ or savedVolume_ <= 0)
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

std::ostream& operator <<( std::ostream& o, const DevCD& devCD)
{
    o << "Number of tracks " << devCD.numberOfTracks() << "\n"
        << "Current Track " << devCD.currentTrackIndex() << "\n"
        << "Track time " << devCD.currentTrackLengthInSeconds() << "\n"
        << "Track running time " << devCD.currentTrackRunningTime() << "\n"
        << "Track remaining time " << devCD.currentTrackTimeRemaining() << std::endl;

    return o;
}

void DevCD::randomPlay( DevCDTrackIndex startTrack, DevCDTrackIndex endTrack, DevCDTrackIndex firstTrack /*= -1*/ )
{
    PlayStatus& status_ = pImpl_->status_;
    DevCDTrackIndex&  randomStartTrack_ = pImpl_-> randomStartTrack_;
    DevCDTrackIndex&  randomEndTrack_ = pImpl_-> randomEndTrack_;
    MexBasicRandom&  randomGenerator_ = pImpl_-> randomGenerator_;

    PRE( startTrack >= 0 );
    PRE( startTrack <= endTrack );

    randomStartTrack_ = startTrack;
    randomEndTrack_ = endTrack + 1;

    if ( firstTrack != -1 )
    {
        play( firstTrack );
    }
    else
    {
        if ( randomStartTrack_ < numberOfTracks() )
        {
            // Make sure we're not asking it to randomise a number outside the range of tracks
            // on the CD.
            DevCDTrackIndex tmpEndTrack = std::min( numberOfTracks(), randomEndTrack_ );

            if ( randomStartTrack_ < tmpEndTrack )
            {
                DevCDTrackIndex trackToPlay = mexRandomInt( &randomGenerator_, randomStartTrack_, tmpEndTrack );
                play( trackToPlay );
            }
        }
    }

    status_ = RANDOM;
}

/* End CD.CPP *******************************************************/
