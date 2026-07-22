#pragma once

#include <atomic>

#include "al.h"

#include "device/CDPlayList.hpp"
#include "mathex/Random.hpp"

class DevCD;
class OggStream;

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
    static DevCDImpl* getInstance(DevCD* parent);

    enum CDVOLUME
    {
        MAX_CDVOLUME = 65535,
        MIN_CDVOLUME = 0
    };

    OggStream* musicStream_{};
    ALuint source_{};

    PlayStatus status_ = NORMAL;
    DevCDTrackIndex trackPlaying_{};

    std::atomic<bool> needsUpdate_{};
    unsigned int savedVolume_{};

    DevCDPlayList* pPlayList_{};

    bool haveMixer_{};

    DevCDTrackIndex randomStartTrack_{};
    DevCDTrackIndex randomEndTrack_{};
    MexBasicRandom randomGenerator_;

    bool musicEnabled_{};
};
