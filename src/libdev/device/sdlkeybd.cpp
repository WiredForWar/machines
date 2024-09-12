/*
 * W 9 5 K E Y B D . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved
 */

#include "device/private/sdlkeybd.hpp"

#include <limits.h>

#include "device/eventq.hpp"

#include <unordered_map>

DevSdlKeyboard& DevSdlKeyboard::sdlInstance()
{
    static DevSdlKeyboard keyboard;
    return keyboard;
}

DevKeyboard& DevKeyboard::instance()
{
    return DevSdlKeyboard::sdlInstance();
}

DevSdlKeyboard::DevSdlKeyboard()
{
}

DevSdlKeyboard::~DevSdlKeyboard()
{
}

void DevSdlKeyboard::wm_key(const DevButtonEvent& ev)
{
    // For state changes, we are only interested in key-press transitions and
    // can ignore all auto-repeat events, i.e. when we receive a keydown
    // message and previous state was also down.
    // Device::KeyCode::printScanCode(std::cout, ev.scanCode()); std::cout << ": "< ev.action() <<" "<<ev.scanCode()<< std::endl;
    switch (ev.action())
    {
        case DevButtonEvent::PRESS:
            if (!ev.previous())
                sdlInstance().pressed(ev.scanCode());
            break;
        case DevButtonEvent::RELEASE:
            sdlInstance().released(ev.scanCode());
            break;
        default:
            ASSERT_BAD_CASE;
    }

    // Regardless of the previous value, we must add sth. to the event queue.
    DevEventQueue::instance().queueEvent(ev);
}

// When the app's window looses input focus, we don't get any key
// up messages for keys which are depressed at the time.  Passing
// this msg to the keyboard object allows it to reset its internal
// state.  Not an ideal solution, but an unlikely situation.
void DevSdlKeyboard::wm_killfocus()
{
    allKeysReleased();
}

// Called in response to a WM_CHAR message
void DevSdlKeyboard::wm_char(const DevButtonEvent& ev)
{
    DevEventQueue::instance().queueEvent(ev);
}

Device::KeyCode DevSdlKeyboard::translateScanCode(SDL_Scancode sdlCode)
{
    static const std::unordered_map<SDL_Scancode, Device::KeyCode> map = {
        /* clang-format off */
        // Trick clang-format to keep one pair per line
        {SDL_SCANCODE_UNKNOWN, ScanCode::UNKNOWN},
        /* clang-format on */

        { SDL_SCANCODE_0, ScanCode::KEY_0 },
        { SDL_SCANCODE_1, ScanCode::KEY_1 },
        { SDL_SCANCODE_2, ScanCode::KEY_2 },
        { SDL_SCANCODE_3, ScanCode::KEY_3 },
        { SDL_SCANCODE_4, ScanCode::KEY_4 },
        { SDL_SCANCODE_5, ScanCode::KEY_5 },
        { SDL_SCANCODE_6, ScanCode::KEY_6 },
        { SDL_SCANCODE_7, ScanCode::KEY_7 },
        { SDL_SCANCODE_8, ScanCode::KEY_8 },
        { SDL_SCANCODE_9, ScanCode::KEY_9 },

        { SDL_SCANCODE_A, ScanCode::KEY_A },
        { SDL_SCANCODE_B, ScanCode::KEY_B },
        { SDL_SCANCODE_C, ScanCode::KEY_C },
        { SDL_SCANCODE_D, ScanCode::KEY_D },
        { SDL_SCANCODE_E, ScanCode::KEY_E },
        { SDL_SCANCODE_F, ScanCode::KEY_F },
        { SDL_SCANCODE_G, ScanCode::KEY_G },
        { SDL_SCANCODE_H, ScanCode::KEY_H },
        { SDL_SCANCODE_I, ScanCode::KEY_I },
        { SDL_SCANCODE_J, ScanCode::KEY_J },
        { SDL_SCANCODE_K, ScanCode::KEY_K },
        { SDL_SCANCODE_L, ScanCode::KEY_L },
        { SDL_SCANCODE_M, ScanCode::KEY_M },
        { SDL_SCANCODE_N, ScanCode::KEY_N },
        { SDL_SCANCODE_O, ScanCode::KEY_O },
        { SDL_SCANCODE_P, ScanCode::KEY_P },
        { SDL_SCANCODE_Q, ScanCode::KEY_Q },
        { SDL_SCANCODE_R, ScanCode::KEY_R },
        { SDL_SCANCODE_S, ScanCode::KEY_S },
        { SDL_SCANCODE_T, ScanCode::KEY_T },
        { SDL_SCANCODE_U, ScanCode::KEY_U },
        { SDL_SCANCODE_V, ScanCode::KEY_V },
        { SDL_SCANCODE_W, ScanCode::KEY_W },
        { SDL_SCANCODE_X, ScanCode::KEY_X },
        { SDL_SCANCODE_Y, ScanCode::KEY_Y },
        { SDL_SCANCODE_Z, ScanCode::KEY_Z },

        { SDL_SCANCODE_KP_DIVIDE, ScanCode::FORWARD_SLASH_PAD },
        { SDL_SCANCODE_KP_MULTIPLY, ScanCode::ASTERISK_PAD },
        { SDL_SCANCODE_KP_MINUS, ScanCode::MINUS_PAD },
        { SDL_SCANCODE_KP_PLUS, ScanCode::PLUS_PAD },
        { SDL_SCANCODE_KP_ENTER, ScanCode::ENTER_PAD },
        { SDL_SCANCODE_KP_0, ScanCode::PAD_0 },
        { SDL_SCANCODE_KP_1, ScanCode::PAD_1 },
        { SDL_SCANCODE_KP_2, ScanCode::PAD_2 },
        { SDL_SCANCODE_KP_3, ScanCode::PAD_3 },
        { SDL_SCANCODE_KP_4, ScanCode::PAD_4 },
        { SDL_SCANCODE_KP_5, ScanCode::PAD_5 },
        { SDL_SCANCODE_KP_6, ScanCode::PAD_6 },
        { SDL_SCANCODE_KP_7, ScanCode::PAD_7 },
        { SDL_SCANCODE_KP_8, ScanCode::PAD_8 },
        { SDL_SCANCODE_KP_9, ScanCode::PAD_9 },
        { SDL_SCANCODE_KP_PERIOD, ScanCode::DELETE_PAD },

        { SDL_SCANCODE_SLASH, ScanCode::FORWARD_SLASH },

        { SDL_SCANCODE_F1, ScanCode::F1 },
        { SDL_SCANCODE_F2, ScanCode::F2 },
        { SDL_SCANCODE_F3, ScanCode::F3 },
        { SDL_SCANCODE_F4, ScanCode::F4 },
        { SDL_SCANCODE_F5, ScanCode::F5 },
        { SDL_SCANCODE_F6, ScanCode::F6 },
        { SDL_SCANCODE_F7, ScanCode::F7 },
        { SDL_SCANCODE_F8, ScanCode::F8 },
        { SDL_SCANCODE_F9, ScanCode::F9 },
        { SDL_SCANCODE_F10, ScanCode::F10 },
        { SDL_SCANCODE_F11, ScanCode::F11 },
        { SDL_SCANCODE_F12, ScanCode::F12 },

        { SDL_SCANCODE_PAUSE, ScanCode::BREAK },

        { SDL_SCANCODE_RIGHT, ScanCode::RIGHT_ARROW },
        { SDL_SCANCODE_LEFT, ScanCode::LEFT_ARROW },
        { SDL_SCANCODE_DOWN, ScanCode::DOWN_ARROW },
        { SDL_SCANCODE_UP, ScanCode::UP_ARROW },

        { SDL_SCANCODE_LALT, ScanCode::LEFT_ALT },
        { SDL_SCANCODE_RALT, ScanCode::RIGHT_ALT },
        { SDL_SCANCODE_LSHIFT, ScanCode::LEFT_SHIFT },
        { SDL_SCANCODE_RSHIFT, ScanCode::RIGHT_SHIFT },
        { SDL_SCANCODE_LCTRL, ScanCode::LEFT_CONTROL },
        { SDL_SCANCODE_RCTRL, ScanCode::RIGHT_CONTROL },
        { SDL_SCANCODE_CAPSLOCK, ScanCode::CAPS_LOCK },

        { SDL_SCANCODE_RETURN, ScanCode::ENTER },
        { SDL_SCANCODE_ESCAPE, ScanCode::ESCAPE },
        { SDL_SCANCODE_BACKSPACE, ScanCode::BACK_SPACE },
        { SDL_SCANCODE_TAB, ScanCode::TAB },
        { SDL_SCANCODE_SPACE, ScanCode::SPACE },

        { SDL_SCANCODE_GRAVE, ScanCode::GRAVE },

        { SDL_SCANCODE_INSERT, ScanCode::INSERT },
        { SDL_SCANCODE_DELETE, ScanCode::KEY_DELETE },
        { SDL_SCANCODE_HOME, ScanCode::HOME },
        { SDL_SCANCODE_END, ScanCode::END },
        { SDL_SCANCODE_PAGEUP, ScanCode::PAGE_UP },
        { SDL_SCANCODE_PAGEDOWN, ScanCode::PAGE_DOWN },
    };

    const auto it = map.find(sdlCode);
    if (it != map.cend())
        return it->second;

    return Device::KeyCode::UNKNOWN;
}

///////////////////////////////////
