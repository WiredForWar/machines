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

    alureStream*        stream_;
    ALuint              source_;

    PlayStatus status_ = NORMAL;
    DevCDTrackIndex trackPlaying_;

    bool needsUpdate_ = false;
    unsigned int savedVolume_;

    DevCDPlayList* pPlayList_;

    bool haveMixer_;

    DevCDTrackIndex randomStartTrack_;
    DevCDTrackIndex randomEndTrack_;
    MexBasicRandom    randomGenerator_;

    bool musicEnabled_;

};
