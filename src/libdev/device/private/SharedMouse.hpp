#pragma once

#ifndef DEVICE_MOUSE
#error Do not include SharedMouse.hpp file directly, include Mouse.hpp
#endif

#include "base/base.hpp"

#include <utility>

// All the bits common to all mouse implementations.
class DevSharedMouse
{
public:
    using XCoord = int32;
    using YCoord = int32;

    using Position = std::pair<XCoord, YCoord>;

    // Pointer travel in device counts. Fractional because a system may report
    // sub-count travel once its own pointer scaling has been applied.
    struct Motion
    {
        double x = 0.0;
        double y = 0.0;

        bool isZero() const { return x == 0.0 && y == 0.0; }
    };

    enum ButtonState
    {
        NO_CHANGE,
        PRESSED,
        RELEASED
    };

protected:
    DevSharedMouse() = default;

    mutable bool lastLeftButtonState_{};
    mutable bool lastRightButtonState_{};
};
