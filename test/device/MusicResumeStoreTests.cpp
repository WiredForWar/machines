#include "device/MusicResumeStore.hpp"

#include <gtest/gtest.h>

TEST(DevMusicResumeStoreTests, unknownTrackHasNoResumePosition)
{
    DevMusicResumeStore store;

    EXPECT_FALSE(store.takeResumeSeconds(4).has_value());
}

TEST(DevMusicResumeStoreTests, interruptedTrackResumesWhereItLeft)
{
    DevMusicResumeStore store;
    store.recordInterruption(4, 116.41);

    const std::optional<double> resume = store.takeResumeSeconds(4);
    ASSERT_TRUE(resume.has_value());
    EXPECT_DOUBLE_EQ(*resume, 116.41);
}

TEST(DevMusicResumeStoreTests, resumePositionIsConsumedByUse)
{
    DevMusicResumeStore store;
    store.recordInterruption(4, 116.41);

    store.takeResumeSeconds(4);

    EXPECT_FALSE(store.takeResumeSeconds(4).has_value());
}

TEST(DevMusicResumeStoreTests, laterInterruptionOverwritesEarlier)
{
    DevMusicResumeStore store;
    store.recordInterruption(4, 30.0);
    store.recordInterruption(4, 75.5);

    const std::optional<double> resume = store.takeResumeSeconds(4);
    ASSERT_TRUE(resume.has_value());
    EXPECT_DOUBLE_EQ(*resume, 75.5);
}

TEST(DevMusicResumeStoreTests, takingOneTrackLeavesTheOthers)
{
    DevMusicResumeStore store;
    store.recordInterruption(1, 12.0);
    store.recordInterruption(4, 30.0);

    store.takeResumeSeconds(1);

    EXPECT_TRUE(store.takeResumeSeconds(4).has_value());
}

TEST(DevMusicResumeStoreTests, clearForgetsPositions)
{
    DevMusicResumeStore store;
    store.recordInterruption(4, 30.0);

    store.clear();

    EXPECT_FALSE(store.takeResumeSeconds(4).has_value());
}

TEST(DevMusicResumeStoreTests, randomInterruptionResumesSameTrackForSameRange)
{
    DevMusicResumeStore store;
    store.recordRandomInterruption(7, 42.0, 4, 11);

    const std::optional<DevCDTrackIndex> track = store.takeRandomTrack(4, 11);
    ASSERT_TRUE(track.has_value());
    EXPECT_EQ(*track, 7);
}

TEST(DevMusicResumeStoreTests, randomSessionIsConsumedByUse)
{
    DevMusicResumeStore store;
    store.recordRandomInterruption(7, 42.0, 4, 11);

    store.takeRandomTrack(4, 11);

    EXPECT_FALSE(store.takeRandomTrack(4, 11).has_value());
}

TEST(DevMusicResumeStoreTests, randomSessionIgnoresDifferentRange)
{
    DevMusicResumeStore store;
    store.recordRandomInterruption(7, 42.0, 4, 11);

    EXPECT_FALSE(store.takeRandomTrack(5, 11).has_value());
}

TEST(DevMusicResumeStoreTests, randomInterruptionAlsoRecordsThePosition)
{
    DevMusicResumeStore store;
    store.recordRandomInterruption(7, 42.0, 4, 11);

    const std::optional<double> resume = store.takeResumeSeconds(7);
    ASSERT_TRUE(resume.has_value());
    EXPECT_DOUBLE_EQ(*resume, 42.0);
}

TEST(DevMusicResumeStoreTests, plainInterruptionLeavesNoRandomSession)
{
    DevMusicResumeStore store;
    store.recordInterruption(7, 42.0);

    EXPECT_FALSE(store.takeRandomTrack(4, 11).has_value());
}

TEST(DevMusicResumeStoreTests, clearForgetsTheRandomSession)
{
    DevMusicResumeStore store;
    store.recordRandomInterruption(7, 42.0, 4, 11);

    store.clear();

    EXPECT_FALSE(store.takeRandomTrack(4, 11).has_value());
}
