#pragma once

#include "device/CDDefs.hpp"

#include <map>
#include <optional>

// Remembers where interrupted music tracks left off, so a later request for
// the same track can resume rather than restart. A recorded position is
// consumed by the take that uses it; a track without one starts from the top.
class DevMusicResumeStore
{
public:
    // Record that 'track' was superseded while audible at 'seconds'.
    void recordInterruption(DevCDTrackIndex track, double seconds);

    // Record that a random rotation over [startTrack, endTrack) was superseded
    // while 'track' was audible at 'seconds'.
    void recordRandomInterruption(DevCDTrackIndex track, double seconds, DevCDTrackIndex startTrack, DevCDTrackIndex endTrack);

    // The position to resume 'track' from, if it holds one. Consumes it.
    std::optional<double> takeResumeSeconds(DevCDTrackIndex track);

    // The track an interrupted random rotation over the same [startTrack,
    // endTrack) should continue with, if one was recorded. Consumes it.
    std::optional<DevCDTrackIndex> takeRandomTrack(DevCDTrackIndex startTrack, DevCDTrackIndex endTrack);

    // Forget every recorded position and session.
    void clear();

private:
    struct RandomSession
    {
        DevCDTrackIndex track{};
        DevCDTrackIndex startTrack{};
        DevCDTrackIndex endTrack{};
    };

    std::map<DevCDTrackIndex, double> resumeSeconds_;
    std::optional<RandomSession> randomSession_;
};
