#include "machgui/MouseLook.hpp"

namespace MachGui
{

namespace
{

// Rotation per unit of pointer travel. Sweeping a machine through its whole range of
// pitch takes about a hand's width of travel on a common pointing device.
constexpr double DegreesPerCount = 0.1;

} // namespace

MouseLookRotation mouseLookRotation(double xCounts, double yCounts, const MouseLookSettings& settings)
{
    const double pitchSign = settings.invertPitch ? -1.0 : 1.0;

    MouseLookRotation rotation;
    rotation.yaw = MexRadians(settings.yawPerCount * xCounts);
    rotation.pitch = MexRadians(settings.pitchPerCount * (yCounts * pitchSign));
    return rotation;
}

MouseLookSettings firstPersonMouseLookSettings(bool invertPitch, double precisionScale)
{
    MouseLookSettings settings;
    settings.yawPerCount = MexDegrees(DegreesPerCount * precisionScale);
    settings.pitchPerCount = MexDegrees(DegreesPerCount * precisionScale);
    settings.invertPitch = invertPitch;
    return settings;
}

} // namespace MachGui
