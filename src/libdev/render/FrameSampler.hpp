#pragma once

#include <chrono>
#include <vector>

#include <cstddef>

namespace Ren
{

// Frame timings collected over a run of frames, one entry per frame. Nothing is
// averaged on the way in: what decides whether a frame rate is comfortable is
// the slowest few frames, and a mean is exactly what hides those.
//
// Times itself rather than reading the game's clock, which counts in whole
// milliseconds -- the same order as a frame, so it would quantise every reading
// here to nothing.
class FrameSampler
{
public:
    struct Frame
    {
        // What the renderer spent composing the frame -- the span the render
        // library times. Presenting the frame is not in here, so neither vsync
        // nor the frame rate limit is either.
        double renderSeconds{};

        // From one frame being composed to the next. This is the whole frame:
        // the simulation, presenting, and whatever the frame rate limit slept.
        double intervalSeconds{};
    };

    // The one a running game feeds. There is a single render loop, so there is
    // one collection to read.
    static FrameSampler& instance();

    // Collects the next `frames` frames, dropping anything collected before.
    // Asking for none stops collecting and keeps nothing.
    void collect(std::size_t frames);

    // Keeps what has arrived so far and stops. What was asked for is forgotten,
    // so collected() against wanted() no longer says a run was cut short.
    void stop();

    bool collecting() const { return frames_.size() < wanted_; }
    std::size_t wanted() const { return wanted_; }
    std::size_t collected() const { return frames_.size(); }

    // Called either side of a frame being composed, and only recorded while a
    // run is collecting, so the frames a reader spends reading the last run do
    // not join it. A run takes one more frame than it records: the first is
    // where the second one's interval is measured from.
    void frameStarted();
    void frameEnded();

    const std::vector<Frame>& frames() const { return frames_; }

private:
    using Clock = std::chrono::steady_clock;

    std::size_t wanted_{};
    std::vector<Frame> frames_{};

    Clock::time_point started_{};
    // When the previous frame finished, which is what an interval is measured
    // from. Unset until the first frame of a run has been through.
    Clock::time_point finished_{};
};

// The shape of a set of frame times, in milliseconds. A percentile is the value
// at that rank in the sorted sample -- no interpolation -- so every field is a
// time some frame actually took. An empty sample gives all zeroes, which no
// real frame can produce.
struct FrameStatistics
{
    std::size_t frames{};
    double meanMs{};
    double minMs{};
    double medianMs{};
    double p95Ms{};
    double p99Ms{};
    double maxMs{};

    // Frames per second, from the mean. Zero for an empty sample.
    double perSecond() const;
};

// Takes the sample by value: it has to be sorted, and a caller that wants to
// keep its own order should not have to defend against this.
FrameStatistics frameStatistics(std::vector<double> seconds);

// The two views of one collection.
FrameStatistics renderStatistics(const std::vector<FrameSampler::Frame>& frames);
FrameStatistics intervalStatistics(const std::vector<FrameSampler::Frame>& frames);

} // namespace Ren
