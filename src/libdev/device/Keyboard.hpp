#pragma once

#include "device/ButtonEvent.hpp"
#include "device/Key.hpp"

#include "base/base.hpp"

// Which keys are held, and the way key events enter the game.
//
// This state can only track the real world while the window holds input focus.
// Press and release do not otherwise arrive in matched pairs, which is what
// submitFocusLost() settles.
class DevKeyboard
{
public:
    static DevKeyboard& instance();

    using ScanCode = Device::KeyCode;

    // Is the key corresponding to a given key code pressed?
    bool keyCode(ScanCode) const;

    // Is any key depressed?
    bool anyKey() const;

    // Are either of these modifier keys pressed?
    bool shiftPressed() const;
    bool ctrlPressed() const;
    bool altPressed() const;

    // Report a key going down or up.
    void submitKeyEvent(const DevButtonEvent& ev);

    // Report a typed character.
    void submitCharEvent(const DevButtonEvent& ev);

    // The window has lost input focus. Every key held at this moment counts as
    // released, and no further event will report it.
    void submitFocusLost();

    void CLASS_INVARIANT
    {
#ifdef _TEST_INVARIANTS
        keys_invariant(file, line);
#endif
    }

    DevKeyboard(const DevKeyboard&) = delete;
    DevKeyboard& operator=(const DevKeyboard&) = delete;
    bool operator==(const DevKeyboard&) const = delete;

private:
    DevKeyboard();
    ~DevKeyboard();

    void pressed(ScanCode);
    void released(ScanCode);
    void allKeysReleased();

    bool& keyMap(ScanCode code) { return keyMap_[static_cast<int>(code)]; }
    bool keyMap(ScanCode code) const { return keyMap_[static_cast<int>(code)]; }

    static constexpr std::size_t N_KEYS = 256;
    static_assert(N_KEYS >= Device::MAX_CODE);
    bool keyMap_[N_KEYS]{};

    // This is the number of keys currently depressed, this should
    // equal the number of true elements in keyMap_.
    int pressedCount_{};

    void keys_invariant(const char* file, const char* line) const;
};
