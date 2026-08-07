#pragma once

#include "mathex/Degrees.hpp"
#include "mathex/Radians.hpp"

namespace MachGui
{

// How much rotation a unit of pointer travel is worth.
struct MouseLookSettings
{
    MexDegrees yawPerCount { 0.0 };
    MexDegrees pitchPerCount { 0.0 };

    // Look up where the pointer is pushed down, and down where it is pushed up.
    bool invertPitch = false;
};

// The rotation a stretch of pointer travel asks for.
struct MouseLookRotation
{
    MexRadians yaw { 0.0 }; // positive turns to the right
    MexRadians pitch { 0.0 }; // positive looks down
};

// The rotation asked for by travelling xCounts to the right and yCounts down.
//
// The result depends on the distance travelled and nothing else. It is therefore the
// same whether that distance arrives as one large report or many small ones, which
// keeps aiming identical at any frame rate, at any screen resolution, and for any
// mixture of the two axes.
MouseLookRotation mouseLookRotation(double xCounts, double yCounts, const MouseLookSettings& settings);

// Settings for aiming a first-person machine.
// A precision scale below one is applied to both axes for finer aiming.
MouseLookSettings firstPersonMouseLookSettings(bool invertPitch, double precisionScale = 1.0);

} // namespace MachGui
