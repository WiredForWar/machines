/*
 * S H R K E Y B D . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

#include "base/base.hpp"

#ifdef _DOSAPP
#include <i86.h>
#endif
#include "device/Keyboard.hpp"

#include <sstream>

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

void DevKeyboard::pressed(ScanCode code)
{
    // Although this is an OS independent piece of code, we must be
    // aware that it can be called from within a DOS interrupt
    // handler.  It appears that we are quite limited in the system
    // calls that can be made here.  Hence, no TEST_INVARIANT.
    PRE(static_cast<int>(code) < N_KEYS);

    if (!keyMap(code))
    {
        // An interrupt between the next two statements could cause this
        // object's state to become inconsistent.  We rely on the caller
        // having disabled interrupts under DOS!!
        keyMap(code) = true;
        ++pressedCount_;
    }
}

void DevKeyboard::released(ScanCode scanCode)
{
    const auto code = static_cast<std::size_t>(scanCode);
    // Don't test invariant, see DevKeyboard::pressed.
    PRE(code < N_KEYS);

    if (keyMap_[code])
    {
        // Interrupts must be disabled, see DevKeyboard::pressed.
        keyMap_[code] = false;
        --pressedCount_;
    }
}

void DevKeyboard::allKeysReleased()
{
    TEST_INVARIANT;

// Disable interrupts because pressedCount_ and keyMap_ can change
// asyncronously.
#ifdef _DOSAPP
    _disable();
#endif

    for (int i = 0; i < N_KEYS; ++i)
    {
        keyMap_[i] = false;
        lastKeyMap_[i] = false;
    }

    // An interrupt occuring here, between these statements, could set an
    // element of keyMap_ and increment pressedCount_, hence the disable.
    pressedCount_ = 0;

#ifdef _DOSAPP
    _enable();
#endif

    TEST_INVARIANT;
}

#ifdef _TEST_INVARIANTS
#include <sstream>
#endif

// We appear to get a crash if this is called from within a DOS interrupt
// handler.  Probably due to complicated non-reentrant things (like malloc).
#ifdef _TEST_INVARIANTS
void DevKeyboard::keys_invariant(const char* file, const char* line) const
#else
void DevKeyboard::keys_invariant(const char*, const char*) const
#endif
{
// Disable interrupts because pressedCount_ and keyMap_ can change
// asyncronously.
#ifdef _DOSAPP
    _disable();
#endif

    // pressedCount_ should always reflect the number of keys depressed
    // in keyMap_.
    int count = 0;
    for (int i = 0; i != N_KEYS; ++i)
        if (keyMap_[i])
            ++count;

    // Re-enable interrupts before testing any invariant which may fail.
    const int pressed_count = pressedCount_;
#ifdef _DOSAPP
    _enable();
#endif

    if (pressed_count != count)
    {
        std::ostringstream ostr;
        ostr << "KeyMap is:\n";
        for (int j = 0; j != N_KEYS; ++j)
        {
            ostr << ((keyMap_[j]) ? "1 " : "0 ");

            if (!(j % 32))
                ostr << "\n";
        }

        ostr << std::ends;
        INVARIANT_INFO(ostr.str());
        // ostr.rdbuf()->freeze(0);
    }

    INVARIANT_INFO(pressed_count);
    INVARIANT_INFO(count);
    INVARIANT(pressed_count == count);
}
