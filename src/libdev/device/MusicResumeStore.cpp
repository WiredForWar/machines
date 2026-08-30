#include "device/MusicResumeStore.hpp"

void DevMusicResumeStore::recordInterruption(DevCDTrackIndex track, double seconds)
{
    resumeSeconds_[track] = seconds;
}

void DevMusicResumeStore::recordRandomInterruption(
    DevCDTrackIndex track,
    double seconds,
    DevCDTrackIndex startTrack,
    DevCDTrackIndex endTrack)
{
    recordInterruption(track, seconds);
    randomSession_ = RandomSession{
        .track = track,
        .startTrack = startTrack,
        .endTrack = endTrack,
    };
}

std::optional<double> DevMusicResumeStore::takeResumeSeconds(DevCDTrackIndex track)
{
    const auto it = resumeSeconds_.find(track);
    if (it == resumeSeconds_.end())
    {
        return std::nullopt;
    }

    const double seconds = it->second;
    resumeSeconds_.erase(it);
    return seconds;
}

std::optional<DevCDTrackIndex> DevMusicResumeStore::takeRandomTrack(DevCDTrackIndex startTrack, DevCDTrackIndex endTrack)
{
    if (!randomSession_ || randomSession_->startTrack != startTrack || randomSession_->endTrack != endTrack)
    {
        return std::nullopt;
    }

    const DevCDTrackIndex track = randomSession_->track;
    randomSession_.reset();
    return track;
}

void DevMusicResumeStore::clear()
{
    resumeSeconds_.clear();
    randomSession_.reset();
}
