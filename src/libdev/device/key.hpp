#pragma once

#include <ostream>

namespace Device
{

namespace detail
{

    using KeyCodeUnderlying = uint8_t;
    constexpr int ModifiersBitsShift = sizeof(KeyCodeUnderlying) * 8;

} // namespace detail

enum class KeyModifier
{
    None = 0x0,
    Alt = 0x1 << detail::ModifiersBitsShift,
    Ctrl = 0x2 << detail::ModifiersBitsShift,
    Shift = 0x4 << detail::ModifiersBitsShift,
};

enum class KeyCode : detail::KeyCodeUnderlying
{
    UNKNOWN,

    // keypad keys
    UP_ARROW_PAD,
    DOWN_ARROW_PAD,
    LEFT_ARROW_PAD,
    RIGHT_ARROW_PAD,
    ENTER_PAD,
    INSERT_PAD,
    DELETE_PAD,
    HOME_PAD,
    END_PAD,
    PAGE_UP_PAD,
    PAGE_DOWN_PAD,
    FIVE_PAD,
    ASTERISK_PAD,
    FORWARD_SLASH_PAD,
    PLUS_PAD,
    MINUS_PAD,
    PAD_0,
    PAD_1,
    PAD_2,
    PAD_3,
    PAD_4,
    PAD_5,
    PAD_6,
    PAD_7,
    PAD_8,
    PAD_9,

    // non-keypad keys
    FORWARD_SLASH,
    NUM_LOCK,
    SCROLL_LOCK,
    RIGHT_CONTROL,
    LEFT_CONTROL,
    UP_ARROW,
    DOWN_ARROW,
    LEFT_ARROW,
    RIGHT_ARROW,
    INSERT,
    KEY_DELETE, // Unfortunately Windows has a #define for DELETE.
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,
    BREAK,
    ESCAPE,
    ENTER,
    SPACE,
    BACK_SPACE,
    TAB,
    GRAVE,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    CAPS_LOCK,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    LEFT_ALT,
    RIGHT_ALT,

    // These codes represent the mouse buttons, numbered left-to-right.
    // They allow the mouse to be processed in the same manner as
    // keyboard keys.
    MOUSE_LEFT,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,

    MOUSE_EXTRA1,
    MOUSE_EXTRA2,
    MOUSE_EXTRA3,
    MOUSE_EXTRA4,
    MOUSE_EXTRA5,
    MOUSE_EXTRA6,
    MOUSE_EXTRA7,
    MOUSE_EXTRA8,

    COUNT,
    INVALID = COUNT
};

// This *must* be the highest code in this enumeration plus 1.
inline constexpr int MAX_CODE = static_cast<detail::KeyCodeUnderlying>(KeyCode::COUNT);
inline constexpr bool isValidCode(KeyCode code)
{
    return (static_cast<detail::KeyCodeUnderlying>(code) < MAX_CODE);
}

void printScanCode(std::ostream&, KeyCode code);

} // namespace Device
