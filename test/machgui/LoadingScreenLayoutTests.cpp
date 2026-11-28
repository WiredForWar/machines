#include "machgui/LoadingScreenLayout.hpp"
#include "machgui/ProgressBarFill.hpp"

#include <gtest/gtest.h>

TEST(LoadingScreenLayoutTests, OriginalResolutionPreservesTheOriginalArtworkCoordinates)
{
    const auto layout = MachGui::loadingScreenLayout({640, 480}, 1);
    EXPECT_EQ(layout.backdrop.origin(), (Ren::Point{0, 0}));
    EXPECT_EQ(layout.backdrop.size(), (Ren::Size{640, 480}));
    EXPECT_EQ(layout.animationOffset, (Ren::Point{32, 129}));
    EXPECT_EQ(layout.noticeBottom, (Ren::Point{32, 115}));
    EXPECT_EQ(layout.progressBar.origin(), (Ren::Point{98, 362}));
    EXPECT_EQ(layout.progressBar.size(), (Ren::Size{441, 5}));
}

TEST(LoadingScreenLayoutTests, FourKCentersTheEnlargedBackdropAndRobotTogether)
{
    const auto layout = MachGui::loadingScreenLayout({3840, 2160}, 4);
    EXPECT_EQ(layout.backdrop.origin(), (Ren::Point{640, 120}));
    EXPECT_EQ(layout.backdrop.size(), (Ren::Size{2560, 1920}));
    EXPECT_EQ(layout.backdrop.origin() + layout.animationOffset, (Ren::Point{768, 636}));
    EXPECT_EQ(layout.noticeBottom, (Ren::Point{768, 580}));
    EXPECT_EQ(layout.progressBar.origin(), (Ren::Point{1032, 1568}));
    EXPECT_EQ(layout.progressBar.size(), (Ren::Size{1764, 20}));
}

TEST(LoadingScreenLayoutTests, AnimationAndProgressStayInsideTheBackdropAtEveryScale)
{
    for (int scale = 1; scale <= 4; ++scale)
    {
        SCOPED_TRACE(scale);
        const auto layout = MachGui::loadingScreenLayout({3840, 2160}, scale);
        // postload.smk is 572x184; its last pixels must cover the same portion
        // of wait.bmp without reaching the wordmark or the loading bar.
        EXPECT_GE(layout.animationOffset.x, 0);
        EXPECT_GE(layout.animationOffset.y, 0);
        EXPECT_LE(layout.animationOffset.x + 572 * scale, layout.backdrop.width);
        EXPECT_LT(layout.animationOffset.y + 184 * scale, 362 * scale);
        EXPECT_LT(layout.noticeBottom.y, layout.backdrop.originY + layout.animationOffset.y);
        EXPECT_LE(layout.progressBar.originX + layout.progressBar.width,
                  layout.backdrop.originX + layout.backdrop.width);
        EXPECT_LE(layout.progressBar.originY + layout.progressBar.height,
                  layout.backdrop.originY + layout.backdrop.height);
    }
}

TEST(LoadingScreenLayoutTests, ChangingWindowSizeMovesEveryElementByTheSameOffset)
{
    const auto original = MachGui::loadingScreenLayout({1280, 960}, 2);
    const auto widescreen = MachGui::loadingScreenLayout({1920, 1080}, 2);
    const Ren::Point displacement{320, 60};
    EXPECT_EQ(widescreen.backdrop.origin(), original.backdrop.origin() + displacement);
    EXPECT_EQ(widescreen.animationOffset, original.animationOffset);
    EXPECT_EQ(widescreen.noticeBottom, original.noticeBottom + displacement);
    EXPECT_EQ(widescreen.progressBar.origin(), original.progressBar.origin() + displacement);
}

TEST(LoadingScreenLayoutTests, TrainingBarUsesTheMenusFractionalScale)
{
    const auto area = MachGui::loadGameProgressArea({480, 0}, 4.5);
    EXPECT_EQ(area.origin(), (Ren::Point{1718, 1125}));
    EXPECT_EQ(area.size(), (Ren::Size{392, 23}));
}

TEST(LoadingScreenLayoutTests, ProgressIsBoundedAndPhaseCountsAreIndependent)
{
    EXPECT_EQ(MachGui::progressBarFillWidth(392, 0, 100, 0, 1), 0);
    EXPECT_EQ(MachGui::progressBarFillWidth(392, 100, 100, 0, 1), 392);
    EXPECT_EQ(MachGui::progressBarFillWidth(392, 150, 100, 0, 1), 392);
    EXPECT_EQ(MachGui::progressBarFillWidth(392, 0, 0, 0, 1), 0);
    // The same raw count in consecutive phases must paint different widths.
    EXPECT_EQ(MachGui::progressBarFillWidth(1000, 10, 100, 0.0, 0.2), 20);
    EXPECT_EQ(MachGui::progressBarFillWidth(1000, 10, 100, 0.2, 0.7), 250);
}
