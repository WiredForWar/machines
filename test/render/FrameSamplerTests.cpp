#include "render/FrameSampler.hpp"

#include <gtest/gtest.h>

#include <vector>

TEST(FrameSamplerTests, AnEmptySampleHasNoShape)
{
    const Ren::FrameStatistics statistics = Ren::frameStatistics({});

    EXPECT_EQ(0u, statistics.frames);
    EXPECT_EQ(0.0, statistics.meanMs);
    EXPECT_EQ(0.0, statistics.medianMs);
    EXPECT_EQ(0.0, statistics.perSecond());
}

TEST(FrameSamplerTests, SecondsAreReportedAsMilliseconds)
{
    const Ren::FrameStatistics statistics = Ren::frameStatistics({ 0.016 });

    EXPECT_EQ(1u, statistics.frames);
    EXPECT_NEAR(16.0, statistics.meanMs, 1e-9);
    EXPECT_NEAR(62.5, statistics.perSecond(), 1e-9);
}

TEST(FrameSamplerTests, EveryPercentileIsATimeSomeFrameTook)
{
    // A hundred frames of 1ms, then one of 100ms: the sort of sample where a
    // mean says everything is fine and the tail says it is not.
    std::vector<double> seconds(100, 0.001);
    seconds.push_back(0.100);

    const Ren::FrameStatistics statistics = Ren::frameStatistics(seconds);

    EXPECT_EQ(101u, statistics.frames);
    EXPECT_NEAR(1.0, statistics.minMs, 1e-9);
    EXPECT_NEAR(1.0, statistics.medianMs, 1e-9);
    EXPECT_NEAR(1.0, statistics.p95Ms, 1e-9);
    EXPECT_NEAR(100.0, statistics.maxMs, 1e-9);
}

TEST(FrameSamplerTests, TheOrderTheFramesArrivedInDoesNotMatter)
{
    const Ren::FrameStatistics rising = Ren::frameStatistics({ 0.001, 0.002, 0.003 });
    const Ren::FrameStatistics falling = Ren::frameStatistics({ 0.003, 0.002, 0.001 });

    EXPECT_EQ(rising.medianMs, falling.medianMs);
    EXPECT_EQ(rising.minMs, falling.minMs);
    EXPECT_EQ(rising.maxMs, falling.maxMs);
}

TEST(FrameSamplerTests, TheTwoViewsReadTheirOwnField)
{
    const std::vector<Ren::FrameSampler::Frame> frames = {
        { .renderSeconds = 0.001, .intervalSeconds = 0.010 },
        { .renderSeconds = 0.003, .intervalSeconds = 0.030 },
    };

    EXPECT_NEAR(2.0, Ren::renderStatistics(frames).meanMs, 1e-9);
    EXPECT_NEAR(20.0, Ren::intervalStatistics(frames).meanMs, 1e-9);
}

TEST(FrameSamplerTests, ARunCollectsWhatItWasAskedForAndNoMore)
{
    Ren::FrameSampler sampler;

    EXPECT_FALSE(sampler.collecting());

    sampler.collect(3);
    EXPECT_TRUE(sampler.collecting());

    // Four frames, because the first is only where the second's interval is
    // measured from.
    for (int frame = 0; frame != 4; ++frame)
    {
        sampler.frameStarted();
        sampler.frameEnded();
    }

    EXPECT_FALSE(sampler.collecting());
    EXPECT_EQ(3u, sampler.collected());

    // Frames after the run is full are not part of it.
    sampler.frameStarted();
    sampler.frameEnded();
    EXPECT_EQ(3u, sampler.collected());
}

TEST(FrameSamplerTests, StoppingEarlyKeepsWhatArrived)
{
    Ren::FrameSampler sampler;
    sampler.collect(1000);

    for (int frame = 0; frame != 3; ++frame)
    {
        sampler.frameStarted();
        sampler.frameEnded();
    }

    sampler.stop();

    EXPECT_FALSE(sampler.collecting());
    EXPECT_EQ(2u, sampler.collected());
}

TEST(FrameSamplerTests, AFrameThatNeverBeganIsNotTimed)
{
    Ren::FrameSampler sampler;
    sampler.collect(2);

    // What the menus do: the frame ends without a 3D one ever being composed,
    // so nothing called frameStarted() and there is no span to measure.
    for (int frame = 0; frame != 4; ++frame)
        sampler.frameEnded();

    EXPECT_EQ(0u, sampler.collected());
    EXPECT_TRUE(sampler.collecting());

    // A frame that did begin is timed, and is not charged for the ones that did
    // not: its render span is its own, not the run's.
    sampler.frameStarted();
    sampler.frameEnded();

    ASSERT_EQ(1u, sampler.collected());
    EXPECT_LT(sampler.frames().front().renderSeconds, 1.0);
}

TEST(FrameSamplerTests, ANewRunDropsTheOldOne)
{
    Ren::FrameSampler sampler;
    sampler.collect(2);
    for (int frame = 0; frame != 3; ++frame)
    {
        sampler.frameStarted();
        sampler.frameEnded();
    }
    ASSERT_EQ(2u, sampler.collected());

    sampler.collect(2);

    EXPECT_EQ(0u, sampler.collected());
    EXPECT_TRUE(sampler.collecting());
}
