#include "device/Keyboard.hpp"

#include "device/EventQueue.hpp"
#include "device/InputState.hpp"
#include "device/Mouse.hpp"
#include "device/Time.hpp"

// static
DevKeyboard& DevKeyboard::instance()
{
    static DevKeyboard keyboard;
    return keyboard;
}

DevKeyboard::DevKeyboard()
{
}

DevKeyboard::~DevKeyboard()
{
}

bool DevKeyboard::keyCode(ScanCode sCode) const
{
    return Device::InputState::instance().isButtonPressed(sCode);
}

bool DevKeyboard::anyKey() const
{
    return Device::InputState::instance().isAnyKeyPressed();
}

bool DevKeyboard::shiftPressed() const
{
    return Device::InputState::instance().heldModifiers() & KeyModifierFlags(Device::KeyModifier::Shift);
}

bool DevKeyboard::ctrlPressed() const
{
    return Device::InputState::instance().heldModifiers() & KeyModifierFlags(Device::KeyModifier::Ctrl);
}

bool DevKeyboard::altPressed() const
{
    return Device::InputState::instance().heldModifiers() & KeyModifierFlags(Device::KeyModifier::Alt);
}

void DevKeyboard::submitKeyEvent(const DevButtonEvent& ev)
{
    // For state changes, we are only interested in key-press transitions and
    // can ignore all auto-repeat events, i.e. when we receive a keydown
    // message and previous state was also down.
    switch (ev.action())
    {
        case DevButtonEvent::PRESS:
            if (!ev.previous())
                Device::InputState::instance().pressButton(ev.scanCode());
            break;
        case DevButtonEvent::RELEASE:
            Device::InputState::instance().releaseButton(ev.scanCode());
            break;
        default:
            ASSERT_BAD_CASE;
    }

    // Regardless of the previous value, we must add sth. to the event queue.
    DevEventQueue::instance().queueEvent(ev);
}

void DevKeyboard::submitCharEvent(const DevButtonEvent& ev)
{
    DevEventQueue::instance().queueEvent(ev);
}

void DevKeyboard::submitFocusLost()
{
    // Every key still held is now released, and no further event will say so.
    // Queue the releases so that the state is announced and not merely dropped;
    // the modifiers read clear because nothing is held any more.
    const DevMouse::Position cursor = DevMouse::instance().position();
    const double now = DevTime::instance().time();

    for (int code = 0; code != Device::MAX_CODE; ++code)
    {
        const ScanCode scanCode = static_cast<ScanCode>(code);
        if (!keyCode(scanCode))
            continue;

        const DevButtonEvent ev(
            scanCode,
            DevButtonEvent::RELEASE,
            true, // previous: the key was down
            KeyModifierFlags(), // nothing is held any more
            now,
            cursor.x,
            cursor.y,
            1); // repeat count must be >= 1
        DevEventQueue::instance().queueEvent(ev);
    }

    Device::InputState::instance().releaseAllButtons();
}
