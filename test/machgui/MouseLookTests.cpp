#include <gtest/gtest.h>

#include "machgui/MouseLook.hpp"

#include "mathex/Degrees.hpp"

namespace
{

MachGui::MouseLookSettings settings(double yawPerCount, double pitchPerCount, bool invertPitch = false)
{
    MachGui::MouseLookSettings result;
    result.yawPerCount = MexDegrees(yawPerCount);
    result.pitchPerCount = MexDegrees(pitchPerCount);
    result.invertPitch = invertPitch;
    return result;
}

double inDegrees(MexRadians angle)
{
    return MexDegrees(angle).asScalar();
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(MouseLookTests, StandingStillAsksForNoRotation)
{
    const auto rotation = MachGui::mouseLookRotation(0.0, 0.0, settings(0.5, 0.5));

    ASSERT_DOUBLE_EQ(0.0, inDegrees(rotation.yaw));
    ASSERT_DOUBLE_EQ(0.0, inDegrees(rotation.pitch));
}

TEST(MouseLookTests, RotationIsTravelTimesSensitivity)
{
    const auto rotation = MachGui::mouseLookRotation(100.0, 40.0, settings(0.5, 0.25));

    ASSERT_DOUBLE_EQ(50.0, inDegrees(rotation.yaw));
    ASSERT_DOUBLE_EQ(10.0, inDegrees(rotation.pitch));
}

TEST(MouseLookTests, TravellingLeftAndUpAsksForNegativeRotation)
{
    const auto rotation = MachGui::mouseLookRotation(-100.0, -40.0, settings(0.5, 0.25));

    ASSERT_DOUBLE_EQ(-50.0, inDegrees(rotation.yaw));
    ASSERT_DOUBLE_EQ(-10.0, inDegrees(rotation.pitch));
}

TEST(MouseLookTests, InvertingPitchFlipsOnlyTheVerticalAxis)
{
    const auto rotation = MachGui::mouseLookRotation(100.0, 40.0, settings(0.5, 0.25, true));

    ASSERT_DOUBLE_EQ(50.0, inDegrees(rotation.yaw));
    ASSERT_DOUBLE_EQ(-10.0, inDegrees(rotation.pitch));
}

TEST(MouseLookTests, TheSameTravelAsksForTheSameRotationHoweverItIsSplitUp)
{
    const auto lookSettings = settings(0.5, 0.25);

    // One report of a frame's worth of travel, as a low frame rate produces.
    const auto inOneGo = MachGui::mouseLookRotation(120.0, 60.0, lookSettings);

    // The same travel spread over many reports, as a high frame rate produces.
    // A rotation that depends on the frame rate is the bug this guards against.
    double yawInDegrees = 0.0;
    double pitchInDegrees = 0.0;
    for (int i = 0; i < 60; ++i)
    {
        const auto step = MachGui::mouseLookRotation(2.0, 1.0, lookSettings);
        yawInDegrees += inDegrees(step.yaw);
        pitchInDegrees += inDegrees(step.pitch);
    }

    ASSERT_DOUBLE_EQ(inDegrees(inOneGo.yaw), yawInDegrees);
    ASSERT_DOUBLE_EQ(inDegrees(inOneGo.pitch), pitchInDegrees);
}

TEST(MouseLookTests, NeitherAxisSuppressesTheOther)
{
    const auto lookSettings = settings(0.5, 0.5);

    // Travel dominated by one axis must still turn the other. Aiming that drops the
    // smaller axis is the bug this guards against.
    const auto mostlySideways = MachGui::mouseLookRotation(200.0, 4.0, lookSettings);
    ASSERT_DOUBLE_EQ(2.0, inDegrees(mostlySideways.pitch));

    const auto mostlyVertical = MachGui::mouseLookRotation(4.0, 200.0, lookSettings);
    ASSERT_DOUBLE_EQ(2.0, inDegrees(mostlyVertical.yaw));
}

TEST(MouseLookTests, BothAxesRespondAlikeToTravelOfTheSameLength)
{
    // Equal sensitivities must mean equal rotation for equal travel, with no
    // hidden dependence on the shape of the screen.
    const auto rotation = MachGui::mouseLookRotation(37.0, 37.0, settings(0.4, 0.4));

    ASSERT_DOUBLE_EQ(inDegrees(rotation.yaw), inDegrees(rotation.pitch));
}

TEST(MouseLookTests, RotationScalesSmoothlyWithTravel)
{
    const auto lookSettings = settings(0.5, 0.5);

    // No thresholds: doubling the travel doubles the rotation, at any size of step.
    for (const double counts : { 0.5, 1.0, 9.0, 11.0, 49.0, 51.0, 61.0, 500.0 })
    {
        const auto single = MachGui::mouseLookRotation(counts, counts, lookSettings);
        const auto doubled = MachGui::mouseLookRotation(2.0 * counts, 2.0 * counts, lookSettings);

        ASSERT_DOUBLE_EQ(2.0 * inDegrees(single.yaw), inDegrees(doubled.yaw)) << "at " << counts << " counts";
        ASSERT_DOUBLE_EQ(2.0 * inDegrees(single.pitch), inDegrees(doubled.pitch)) << "at " << counts << " counts";
    }
}
