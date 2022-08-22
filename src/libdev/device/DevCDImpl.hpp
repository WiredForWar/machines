#pragma once

#include <AL/alure.h>
#include "device/cdlist.hpp"
#include "mathex/random.hpp"

void eosCallback( void*, ALuint );

class DevCD;

enum PlayStatus
{
    PROGRAMMED,
    CONTINUOUS,
    SINGLE,
    REPEAT,
    NORMAL,
    STOPPED,
    RANDOM
};

class DevCDImpl
{
public:
    friend void eosCallback( void*, ALuint );

    static DevCDImpl* getInstance( DevCD* parent );

    enum CDVOLUME
    {
        MAX_CDVOLUME = 65535,
        MIN_CDVOLUME = 0
    };

    alureStream* stream_ = nullptr;
    ALuint source_ = 0;

    PlayStatus status_ = NORMAL;
    DevCDTrackIndex trackPlaying_ = 0;

    bool needsUpdate_ = false;
    unsigned int savedVolume_ = 0;

    DevCDPlayList* pPlayList_ = nullptr;

    bool haveMixer_ = false;

    DevCDTrackIndex randomStartTrack_ = 0;
    DevCDTrackIndex randomEndTrack_ = 0;
    MexBasicRandom    randomGenerator_;

    bool musicEnabled_ = false;
};
