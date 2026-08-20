#include "device/Keyboard.hpp"

#include "device/EventQueue.hpp"
#include "device/Mouse.hpp"
#include "device/Time.hpp"

#include <sstream>

// static
DevKeyboard& DevKeyboard::instance()
{
    static DevKeyboard keyboard;
    return keyboard;
}

DevKeyboard::DevKeyboard()
{
    allKeysReleased();
}

DevKeyboard::~DevKeyboard()
{
}

bool DevKeyboard::keyCode(ScanCode sCode) const
{
    PRE(Device::isValidCode(sCode));
    TEST_INVARIANT;

    return keyMap(sCode);
}

DevKeyboard::KeyState DevKeyboard::deltaKeyCode(ScanCode sCode) const
{
    PRE(static_cast<int>(sCode) < N_KEYS);
    TEST_INVARIANT;

    // Sample once: the two comparisons below must see the same value.
    const bool current = keyMap(sCode);
    const bool last = lastKeyMap(sCode);
    lastKeyMap(sCode) = current;

    if (current == last)
        return NO_CHANGE;

    return current ? PRESSED : RELEASED;
}

bool DevKeyboard::anyKey() const
{
    TEST_INVARIANT;

    return pressedCount_ > 0;
}

bool DevKeyboard::shiftPressed() const
{
    TEST_INVARIANT;

    return keyMap(Device::KeyCode::RIGHT_SHIFT) || keyMap(Device::KeyCode::LEFT_SHIFT);
}

bool DevKeyboard::ctrlPressed() const
{
    TEST_INVARIANT;

    return keyMap(Device::KeyCode::RIGHT_CONTROL) || keyMap(Device::KeyCode::LEFT_CONTROL);
}

bool DevKeyboard::altPressed() const
{
    TEST_INVARIANT;

    return keyMap(Device::KeyCode::RIGHT_ALT) || keyMap(Device::KeyCode::LEFT_ALT);
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
                pressed(ev.scanCode());
            break;
        case DevButtonEvent::RELEASE:
            released(ev.scanCode());
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
            false, // shift
            false, // ctrl
            false, // alt
            now,
            cursor.first,
            cursor.second,
            1); // repeat count must be >= 1
        DevEventQueue::instance().queueEvent(ev);
    }

    allKeysReleased();
}

void DevKeyboard::pressed(ScanCode code)
{
    PRE(static_cast<int>(code) < N_KEYS);

    if (!keyMap(code))
    {
        keyMap(code) = true;
        ++pressedCount_;
    }
}

void DevKeyboard::released(ScanCode scanCode)
{
    const auto code = static_cast<std::size_t>(scanCode);
    PRE(code < N_KEYS);

    if (keyMap_[code])
    {
        keyMap_[code] = false;
        --pressedCount_;
    }
}

void DevKeyboard::allKeysReleased()
{
    TEST_INVARIANT;

    for (int i = 0; i < N_KEYS; ++i)
    {
        keyMap_[i] = false;
        lastKeyMap_[i] = false;
    }

    pressedCount_ = 0;

    TEST_INVARIANT;
}

#ifdef _TEST_INVARIANTS
void DevKeyboard::keys_invariant(const char* file, const char* line) const
#else
void DevKeyboard::keys_invariant(const char*, const char*) const
#endif
{
    // pressedCount_ should always reflect the number of keys depressed
    // in keyMap_.
    int count = 0;
    for (int i = 0; i != N_KEYS; ++i)
        if (keyMap_[i])
            ++count;

    if (pressedCount_ != count)
    {
        std::ostringstream ostr;
        ostr << "KeyMap is:\n";
        for (int j = 0; j != N_KEYS; ++j)
        {
            ostr << ((keyMap_[j]) ? "1 " : "0 ");

            if (!(j % 32))
                ostr << "\n";
        }

        INVARIANT_INFO(ostr.str());
    }

    INVARIANT_INFO(pressedCount_);
    INVARIANT_INFO(count);
    INVARIANT(pressedCount_ == count);
}
