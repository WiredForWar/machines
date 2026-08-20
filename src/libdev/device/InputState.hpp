#pragma once

#include "device/Key.hpp"
#include "device/KeyWithModifiers.hpp"

#include "base/base.hpp"

#include <cstddef>

namespace Device
{

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

    bool buttons_[MAX_CODE]{};

    // How many of the held buttons are keys, kept alongside so that
    // isAnyKeyPressed does not have to sweep the map.
    int keysPressed_{};
};

} // namespace Device
