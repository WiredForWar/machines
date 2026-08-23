#include "render/FrameSampler.hpp"

#include <algorithm>
#include <numeric>

namespace Ren
{

namespace FrameSamplerImpl
{
namespace
{

// The rank a percentile reads, for a sample of n. Clamped so that p100 is the
// last entry rather than one past it.
std::size_t rankFor(std::size_t n, double fraction)
{
    const std::size_t rank = static_cast<std::size_t>(fraction * static_cast<double>(n));

    return std::min(rank, n - 1);
}

} // namespace
} // namespace FrameSamplerImpl

FrameSampler& FrameSampler::instance()
{
    static FrameSampler sampler;

    return sampler;
}

void FrameSampler::collect(std::size_t frames)
{
    wanted_ = frames;
    frames_.clear();
    frames_.reserve(frames);
    started_ = {};
    finished_ = {};
}

void FrameSampler::stop()
{
    wanted_ = frames_.size();
}

void FrameSampler::frameStarted()
{
    if (!collecting())
        return;

    started_ = Clock::now();
}

void FrameSampler::frameEnded(const Frame& counts)
{
    if (!collecting())
        return;

    const Clock::time_point now = Clock::now();

    // The first frame of a run is only the point an interval is measured from.
    // Recording it would put a zero in the sample, and a zero no frame took
    // moves the mean and the minimum both.
    //
    // A frame that never began composing is left out for the same reason from
    // the other end: there is no start to measure a render span from, and one
    // measured from an unset start reads as however long the machine has been
    // up. It still counts as where the next interval is measured from, which is
    // what it is.
    if (started_ != Clock::time_point{} && finished_ != Clock::time_point{})
    {
        const std::chrono::duration<double> render = now - started_;
        const std::chrono::duration<double> interval = now - finished_;

        Frame frame = counts;
        frame.renderSeconds = render.count();
        frame.intervalSeconds = interval.count();

        frames_.push_back(frame);
    }

    started_ = {};
    finished_ = now;
}

double FrameStatistics::perSecond() const
{
    if (meanMs <= 0.0)
        return 0.0;

    return 1000.0 / meanMs;
}

FrameStatistics frameStatistics(std::vector<double> seconds)
{
    using namespace FrameSamplerImpl;

    if (seconds.empty())
        return {};

    for (double& value : seconds)
        value *= 1000.0;

    std::sort(seconds.begin(), seconds.end());

    const std::size_t n = seconds.size();
    const double total = std::accumulate(seconds.begin(), seconds.end(), 0.0);

    return FrameStatistics{
        .frames = n,
        .meanMs = total / static_cast<double>(n),
        .minMs = seconds.front(),
        .medianMs = seconds[rankFor(n, 0.5)],
        .p95Ms = seconds[rankFor(n, 0.95)],
        .p99Ms = seconds[rankFor(n, 0.99)],
        .maxMs = seconds.back(),
    };
}

FrameStatistics renderStatistics(const std::vector<FrameSampler::Frame>& frames)
{
    std::vector<double> seconds;
    seconds.reserve(frames.size());
    for (const FrameSampler::Frame& frame : frames)
        seconds.push_back(frame.renderSeconds);

    return frameStatistics(std::move(seconds));
}

FrameStatistics intervalStatistics(const std::vector<FrameSampler::Frame>& frames)
{
    std::vector<double> seconds;
    seconds.reserve(frames.size());
    for (const FrameSampler::Frame& frame : frames)
        seconds.push_back(frame.intervalSeconds);

    return frameStatistics(std::move(seconds));
}

} // namespace Ren
