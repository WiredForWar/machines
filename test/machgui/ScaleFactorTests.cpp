#include <gtest/gtest.h>

#include "machgui/gui.hpp"

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
