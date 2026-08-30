#pragma once

#include <atomic>
#include <chrono>
#include <optional>

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

    // Is the current stream still being heard on source_?
    bool isStreamAudible() const;

    // Hand the current stream over to the fade-out slot and swap the sources,
    // freeing source_ for the incoming track. Starts the crossfade clock.
    void beginFadeOut();

    // Drop any fade in flight and restore the incoming track to full gain.
    void cancelFade();

    // Advance the crossfade; deletes the outgoing stream once it is silent.
    void updateFade();

    // Set both sources' gains from the user volume and the fade progress.
    void applyGains();

    OggStream* musicStream_{};
    ALuint source_{};

    OggStream* fadeOutStream_{};
    ALuint fadeOutSource_{};
    std::optional<std::chrono::steady_clock::time_point> fadeStart_;
    double fadeInGain_{1.0};
    double fadeOutGain_{};
    double fadeOutStartGain_{1.0};

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
