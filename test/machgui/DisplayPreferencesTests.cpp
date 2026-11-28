#include "machgui/DisplayPreferences.hpp"

#include <gtest/gtest.h>

using MachGui::DisplayPreferences;
using MachGui::applyDisplayChoices;
using Ren::WindowMode;

TEST(DisplayPreferencesTests, AcceptingUnrelatedOptionsDoesNotPinTheDetectedDesktop)
{
    const DisplayPreferences saved{WindowMode::Fullscreen, {}, 0};
    const DisplayPreferences detected{WindowMode::Fullscreen, {3840, 2160}, 60};
    const auto after = applyDisplayChoices(saved, detected, false, false);
    EXPECT_EQ(Ren::Size(), after.resolution);
    EXPECT_EQ(0, after.refreshRate);
}

TEST(DisplayPreferencesTests, AnExplicitSelectionIsSavedEvenWhenItMatchesTheDesktop)
{
    const DisplayPreferences saved{WindowMode::Fullscreen, {}, 0};
    const DisplayPreferences selected{WindowMode::Fullscreen, {3840, 2160}, 60};
    const auto after = applyDisplayChoices(saved, selected, true, false);
    EXPECT_EQ(Ren::Size(3840, 2160), after.resolution);
    EXPECT_EQ(60, after.refreshRate);
}

TEST(DisplayPreferencesTests, ChoosingAutomaticClearsAnExplicitPreference)
{
    const DisplayPreferences saved{WindowMode::Fullscreen, {1920, 1080}, 144};
    const DisplayPreferences selected{WindowMode::Fullscreen, {}, 0};
    const auto after = applyDisplayChoices(saved, selected, true, false);
    EXPECT_EQ(Ren::Size(), after.resolution);
    EXPECT_EQ(0, after.refreshRate);
}

TEST(DisplayPreferencesTests, ADisplayedFallbackDoesNotOverwriteAnUnavailableSavedMode)
{
    const DisplayPreferences saved{WindowMode::Fullscreen, {3840, 2160}, 144};
    const DisplayPreferences fallback{WindowMode::Fullscreen, {1920, 1080}, 60};
    const auto after = applyDisplayChoices(saved, fallback, false, false);
    EXPECT_EQ(saved.resolution, after.resolution);
    EXPECT_EQ(saved.refreshRate, after.refreshRate);
}

TEST(DisplayPreferencesTests, BorderlessRetainsTheExplicitModeForALaterReturnToFullscreen)
{
    const DisplayPreferences saved{WindowMode::Fullscreen, {1920, 1080}, 144};
    const DisplayPreferences borderless{WindowMode::Borderless, {3840, 2160}, 60};
    const auto after = applyDisplayChoices(saved, borderless, true, true);
    EXPECT_EQ(WindowMode::Borderless, after.windowMode);
    EXPECT_EQ(saved.resolution, after.resolution);
    EXPECT_EQ(saved.refreshRate, after.refreshRate);
}

TEST(DisplayPreferencesTests, ARefreshRateChangeDoesNotPinAnAutomaticResolution)
{
    const DisplayPreferences saved{WindowMode::Fullscreen, {}, 0};
    const DisplayPreferences shown{WindowMode::Fullscreen, {3840, 2160}, 144};
    const auto after = applyDisplayChoices(saved, shown, false, true);
    EXPECT_EQ(Ren::Size(), after.resolution);
    EXPECT_EQ(144, after.refreshRate);
}

TEST(DisplayPreferencesTests, AWindowedSizeCanBeExplicitOrAutomatic)
{
    const DisplayPreferences saved{WindowMode::Fullscreen, {}, 0};
    const DisplayPreferences windowed{WindowMode::Windowed, {1280, 720}, 0};
    const auto explicitSize = applyDisplayChoices(saved, windowed, true, false);
    EXPECT_EQ(WindowMode::Windowed, explicitSize.windowMode);
    EXPECT_EQ(Ren::Size(1280, 720), explicitSize.resolution);

    const auto automatic = applyDisplayChoices(saved, windowed, false, false);
    EXPECT_EQ(WindowMode::Windowed, automatic.windowMode);
    EXPECT_EQ(Ren::Size(), automatic.resolution);
}
