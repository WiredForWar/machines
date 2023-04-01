/*
 * K E Y . H P P
 * (c) Charybdis Limited, 1995. All Rights Reserved
 */

#ifndef DEV_KEY_HPP
#define DEV_KEY_HPP

#include <ostream>

class DevKey
{
public:
    enum Code
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
        // They allow the mouse to be processed in the saame manner as
        // keyboard keys.
        LEFT_MOUSE,
        RIGHT_MOUSE,

        // Not trusting that these guys didn't go LEFT_MOUSE+1 or something janky somewhere
        MIDDLE_MOUSE,

        // This *must* be the highest code in this enumeration plus 1.
        MAX_CODE
    };

    static std::string getKeyName(Code keyCode);
    static void printScanCode(std::ostream&, Code);
};

#endif
