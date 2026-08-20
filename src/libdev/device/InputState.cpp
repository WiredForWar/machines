#include "device/InputState.hpp"

// static
Device::InputState& Device::InputState::instance()
{
    static InputState state;
    return state;
}

bool Device::InputState::isButtonPressed(KeyCode code) const
{
    PRE(isValidCode(code));
    TEST_INVARIANT;

    return button(code);
}

bool Device::InputState::isAnyKeyPressed() const
{
    TEST_INVARIANT;

    return keysPressed_ > 0;
}

KeyModifierFlags Device::InputState::heldModifiers() const
{
    TEST_INVARIANT;

    return KeyModifierFlags::fromCtrlAltShiftState(
        button(KeyCode::LEFT_CONTROL) || button(KeyCode::RIGHT_CONTROL),
        button(KeyCode::LEFT_ALT) || button(KeyCode::RIGHT_ALT),
        button(KeyCode::LEFT_SHIFT) || button(KeyCode::RIGHT_SHIFT));
}

const Device::PointerPosition& Device::InputState::pointerPosition() const
{
    return pointerPosition_;
}

void Device::InputState::setPointerPosition(PointerPosition position)
{
    pointerPosition_ = position;
}

Device::PointerMotion Device::InputState::takePointerMotion()
{
    const PointerMotion travelled = pointerMotion_;
    pointerMotion_ = PointerMotion();
    return travelled;
}

void Device::InputState::addPointerMotion(double relativeX, double relativeY)
{
    pointerMotion_.x += relativeX;
    pointerMotion_.y += relativeY;
}

void Device::InputState::pressButton(KeyCode code)
{
    PRE(isValidCode(code));

    if (button(code))
        return;

    button(code) = true;
    if (isKey(code))
        ++keysPressed_;

    TEST_INVARIANT;
}

void Device::InputState::releaseButton(KeyCode code)
{
    PRE(isValidCode(code));

    if (!button(code))
        return;

    button(code) = false;
    if (isKey(code))
        --keysPressed_;

    TEST_INVARIANT;
}

void Device::InputState::releaseAllButtons()
{
    for (bool& held : buttons_)
        held = false;

    keysPressed_ = 0;

    TEST_INVARIANT;
}

void Device::InputState::CLASS_INVARIANT
{
    int keysHeld = 0;
    for (int code = 0; code != MAX_CODE; ++code)
    {
        if (buttons_[code] && isKey(static_cast<KeyCode>(code)))
            ++keysHeld;
    }

    INVARIANT_INFO(keysPressed_);
    INVARIANT_INFO(keysHeld);
    INVARIANT(keysPressed_ == keysHeld);
}
