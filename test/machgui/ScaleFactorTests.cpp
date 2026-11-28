#include <gtest/gtest.h>

#include "machgui/ScaleFactor.hpp"

#include <limits>

///////////////////////////////////////////////////////////////////////////////

TEST(ScaleFactorTests, TheMenusFitAWindowOfTheirOwnSizeUnscaled)
{
    EXPECT_TRUE(MachGui::scaleFactorFits(100, MachGui::MenuSize));
}

TEST(ScaleFactorTests, DoubledMenusNeedTwiceTheirSize)
{
    EXPECT_TRUE(MachGui::scaleFactorFits(200, Ren::Size(1280, 960)));
    EXPECT_TRUE(MachGui::scaleFactorFits(200, Ren::Size(1920, 1080)));
}

TEST(ScaleFactorTests, DoubledMenusDoNotFitAWindowShorterThanTheyAre)
{
    // Tall enough in one direction is not enough: a laptop screen of this shape
    // has the width for doubled menus but not the height.
    EXPECT_FALSE(MachGui::scaleFactorFits(200, Ren::Size(1280, 800)));
    EXPECT_FALSE(MachGui::scaleFactorFits(200, Ren::Size(1280, 720)));
}

TEST(ScaleFactorTests, DoubledMenusDoNotFitAWindowNarrowerThanTheyAre)
{
    EXPECT_FALSE(MachGui::scaleFactorFits(200, Ren::Size(1024, 768)));
    EXPECT_FALSE(MachGui::scaleFactorFits(200, Ren::Size(1152, 864)));
}

TEST(ScaleFactorTests, AWindowOfExactlyTheScaledSizeFits)
{
    EXPECT_TRUE(MachGui::scaleFactorFits(150, Ren::Size(960, 720)));
    EXPECT_FALSE(MachGui::scaleFactorFits(150, Ren::Size(959, 720)));
    EXPECT_FALSE(MachGui::scaleFactorFits(150, Ren::Size(960, 719)));
}

TEST(ScaleFactorTests, LeavingTheScaleToTheGameFitsAnyWindow)
{
    EXPECT_TRUE(MachGui::scaleFactorFits(0, Ren::Size(640, 480)));
    EXPECT_TRUE(MachGui::scaleFactorFits(0, Ren::Size(1024, 768)));
}

TEST(ScaleFactorTests, LargerMenusMustFitBothDimensions)
{
    EXPECT_TRUE(MachGui::scaleFactorFits(300, {1920, 1440}));
    EXPECT_FALSE(MachGui::scaleFactorFits(300, {2560, 1080}));
    EXPECT_TRUE(MachGui::scaleFactorFits(400, {2560, 1920}));
    EXPECT_TRUE(MachGui::scaleFactorFits(400, {3840, 2160}));
    EXPECT_FALSE(MachGui::scaleFactorFits(400, {3840, 1600}));
}

TEST(ScaleFactorTests, AutomaticUsesTheLargestSupportedScaleThatFits)
{
    EXPECT_EQ(100, MachGui::resolveScaleFactor(0, {1280, 720}));
    EXPECT_EQ(200, MachGui::resolveScaleFactor(0, {1920, 1080}));
    EXPECT_EQ(300, MachGui::resolveScaleFactor(0, {2560, 1440}));
    EXPECT_EQ(400, MachGui::resolveScaleFactor(0, {3840, 2160}));
    EXPECT_EQ(200, MachGui::resolveScaleFactor(0, {3440, 1080}));
    EXPECT_EQ(300, MachGui::resolveScaleFactor(0, {3440, 1440}));
}

TEST(ScaleFactorTests, AnExplicitSmallerScaleIsHonoured)
{
    EXPECT_EQ(100, MachGui::resolveScaleFactor(100, {3840, 2160}));
    EXPECT_EQ(200, MachGui::resolveScaleFactor(200, {3840, 2160}));
    EXPECT_EQ(300, MachGui::resolveScaleFactor(300, {3840, 2160}));
}

TEST(ScaleFactorTests, AMovedConfigurationFallsBackWithoutClippingTheMenu)
{
    EXPECT_EQ(200, MachGui::resolveScaleFactor(400, {1920, 1080}));
    EXPECT_EQ(100, MachGui::resolveScaleFactor(300, {1280, 720}));
}

TEST(ScaleFactorTests, InvalidPercentagesCannotOverflowIntoAValidScale)
{
    EXPECT_FALSE(MachGui::scaleFactorFits(-100, {3840, 2160}));
    EXPECT_FALSE(MachGui::scaleFactorFits(std::numeric_limits<int>::max(), {3840, 2160}));
    EXPECT_EQ(400, MachGui::resolveScaleFactor(-100, {3840, 2160}));
    EXPECT_EQ(400, MachGui::resolveScaleFactor(150, {3840, 2160}));
}
