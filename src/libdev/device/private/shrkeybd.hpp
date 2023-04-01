/*
 * S H R K E Y B D . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

#ifndef DEVICE_SHARED_KEYBOARD_HPP
#define DEVICE_SHARED_KEYBOARD_HPP

// #ifndef DEVICE_KEYBOARD
//   #error Do not include shrkeybd.hpp file directly, include keyboard.hpp
// #endif

#include "base/base.hpp"
// class ostream;

#include "device/key.hpp"

//////////////////////////////////////////////////////////////////////
// This class maintains a table of which keys are currently pressed
// on a keyboard.  Derived classes provide the means of determining
// when a key is pressed under a particular OS.
//
// Getting this object's internal state to match the real world is quite
// tricky. In fact, it's impossible in a Windows environment where press
// and release events don't necessarily come in matched pairs (because
// your window may not always have the input focus).
class DevKeyboard
{
public:
    static DevKeyboard& instance();

    using ScanCode = DevKey::ScanCode;

    // Is the key corresponding to a given character or key code pressed?
    bool key(unsigned char) const;
    bool keyCode(ScanCode) const;

    // Is any key depressed?
    bool anyKey() const;

    // Are either of these modifier keys pressed?
    bool shiftPressed() const;
    bool ctrlPressed() const;
    bool altPressed() const;

    // Has the given key changed its state since the last time it was queried?
    enum KeyState
    {
        NO_CHANGE,
        PRESSED,
        RELEASED
    };
    KeyState deltaKey(unsigned char) const;
    KeyState deltaKeyCode(ScanCode) const;

    // These are **not** for public use as they subvert the recording mechanism.
    bool keyNoRecord(unsigned char) const;
    bool keyCodeNoRecord(ScanCode) const;
    bool anyKeyNoRecord() const;
    bool shiftPressedNoRecord() const;
    bool ctrlPressedNoRecord() const;
    bool altPressedNoRecord() const;
    KeyState deltaKeyCodeNoRecord(ScanCode) const;

    // For testing only: to be removed.
    int pressedCount() const { return pressedCount_; }

    void CLASS_INVARIANT
    {
#ifdef _TEST_INVARIANTS
        keys_invariant(file, line);
#endif
    }

protected:
    DevKeyboard();
    ~DevKeyboard();

    // Derived classes inform us of key-state changes with these msgs.
    void pressed(ScanCode);
    void released(ScanCode);
    void allKeysReleased();

private:
    ScanCode getCharacterIndex(unsigned char) const;

    enum
    {
        N_KEYS = 256
    };
    bool keyMap_[N_KEYS];
    bool lastKeyMap_[N_KEYS];

    // This is the number of keys currently depressed, this should
    // equal the number of true elements in keyMap_.  Keeping these
    // consistent, especially in the presence of interrupts and races,
    // is tricky.  Is there a better way to implement anyKey()?
    int pressedCount_;

    void keys_invariant(const char* file, const char* line) const;

    // Singleton => ops revoked.
    DevKeyboard(const DevKeyboard&);
    const DevKeyboard& operator=(const DevKeyboard&);
    bool operator==(const DevKeyboard&) const;
};

#endif

///////////////////////////////////
