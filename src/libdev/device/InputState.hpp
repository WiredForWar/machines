#pragma once

#include "device/Key.hpp"
#include "device/KeyWithModifiers.hpp"

#include "base/base.hpp"

#include <cstddef>

namespace Device
{

// Where the pointer is, in window coordinates.
struct PointerPosition
{
    int32 x{};
    int32 y{};

    bool operator==(const PointerPosition&) const = default;
};

// Pointer travel in device counts. Fractional because a system may report
// sub-count travel once its own pointer scaling has been applied.
struct PointerMotion
{
    double x{};
    double y{};

    bool isZero() const { return x == 0.0 && y == 0.0; }
};

// What input looks like right now.
//
// Updated only by what is submitted, never read back from the platform, so a
// composed event and a real one leave the same state behind.
class InputState
{
public:
    static InputState& instance();

    // Is this button held? A button is anything in KeyCode: a key, a mouse
    // button, and in time a pad or hand-controller button.
    bool isButtonPressed(KeyCode code) const;

    // Is any key held? Mouse buttons do not count -- a held button is not a
    // held key.
    bool isAnyKeyPressed() const;

    // The modifiers held at this moment.
    KeyModifierFlags heldModifiers() const;

    // Where the pointer is. Quantised to the window, so it stops changing once
    // the pointer is held at an edge.
    const PointerPosition& pointerPosition() const;
    void setPointerPosition(PointerPosition position);

    // Travel since this was last asked, and clear the accumulator. Unlike a
    // difference of two positions this is not quantised, so it keeps reporting
    // while the pointer is held still.
    PointerMotion takePointerMotion();
    void addPointerMotion(double relativeX, double relativeY);

    // How far an axis is pushed. Zero for one nothing has reported.
    float axisValue(Axis axis) const;
    void setAxisValue(Axis axis, float value);

    void pressButton(KeyCode code);
    void releaseButton(KeyCode code);

    // Every button reads as released afterwards. What was held is not
    // announced; a caller that owes an event for it sends that itself.
    void releaseAllButtons();

    void CLASS_INVARIANT;

    InputState(const InputState&) = delete;
    InputState& operator=(const InputState&) = delete;

private:
    InputState() = default;
    ~InputState() = default;

    bool& button(KeyCode code) { return buttons_[static_cast<std::size_t>(code)]; }
    bool button(KeyCode code) const { return buttons_[static_cast<std::size_t>(code)]; }

    PointerPosition pointerPosition_{};
    PointerMotion pointerMotion_{};

    bool buttons_[MAX_CODE]{};
    float axes_[MAX_AXIS]{};

    // How many of the held buttons are keys, kept alongside so that
    // isAnyKeyPressed does not have to sweep the map.
    int keysPressed_{};
};

} // namespace Device
