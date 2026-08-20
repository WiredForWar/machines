#include "device/SdlKeyCodes.hpp"

#include <unordered_map>

Device::KeyCode Device::codeFromSdlScanCode(SDL_Scancode sdlCode)
{
    static const std::unordered_map<SDL_Scancode, Device::KeyCode> map = {
        /* clang-format off */
        // Trick clang-format to keep one pair per line
        {SDL_SCANCODE_UNKNOWN, KeyCode::UNKNOWN},
        /* clang-format on */

        { SDL_SCANCODE_0, KeyCode::KEY_0 },
        { SDL_SCANCODE_1, KeyCode::KEY_1 },
        { SDL_SCANCODE_2, KeyCode::KEY_2 },
        { SDL_SCANCODE_3, KeyCode::KEY_3 },
        { SDL_SCANCODE_4, KeyCode::KEY_4 },
        { SDL_SCANCODE_5, KeyCode::KEY_5 },
        { SDL_SCANCODE_6, KeyCode::KEY_6 },
        { SDL_SCANCODE_7, KeyCode::KEY_7 },
        { SDL_SCANCODE_8, KeyCode::KEY_8 },
        { SDL_SCANCODE_9, KeyCode::KEY_9 },

        { SDL_SCANCODE_A, KeyCode::KEY_A },
        { SDL_SCANCODE_B, KeyCode::KEY_B },
        { SDL_SCANCODE_C, KeyCode::KEY_C },
        { SDL_SCANCODE_D, KeyCode::KEY_D },
        { SDL_SCANCODE_E, KeyCode::KEY_E },
        { SDL_SCANCODE_F, KeyCode::KEY_F },
        { SDL_SCANCODE_G, KeyCode::KEY_G },
        { SDL_SCANCODE_H, KeyCode::KEY_H },
        { SDL_SCANCODE_I, KeyCode::KEY_I },
        { SDL_SCANCODE_J, KeyCode::KEY_J },
        { SDL_SCANCODE_K, KeyCode::KEY_K },
        { SDL_SCANCODE_L, KeyCode::KEY_L },
        { SDL_SCANCODE_M, KeyCode::KEY_M },
        { SDL_SCANCODE_N, KeyCode::KEY_N },
        { SDL_SCANCODE_O, KeyCode::KEY_O },
        { SDL_SCANCODE_P, KeyCode::KEY_P },
        { SDL_SCANCODE_Q, KeyCode::KEY_Q },
        { SDL_SCANCODE_R, KeyCode::KEY_R },
        { SDL_SCANCODE_S, KeyCode::KEY_S },
        { SDL_SCANCODE_T, KeyCode::KEY_T },
        { SDL_SCANCODE_U, KeyCode::KEY_U },
        { SDL_SCANCODE_V, KeyCode::KEY_V },
        { SDL_SCANCODE_W, KeyCode::KEY_W },
        { SDL_SCANCODE_X, KeyCode::KEY_X },
        { SDL_SCANCODE_Y, KeyCode::KEY_Y },
        { SDL_SCANCODE_Z, KeyCode::KEY_Z },

        { SDL_SCANCODE_KP_DIVIDE, KeyCode::FORWARD_SLASH_PAD },
        { SDL_SCANCODE_KP_MULTIPLY, KeyCode::ASTERISK_PAD },
        { SDL_SCANCODE_KP_MINUS, KeyCode::MINUS_PAD },
        { SDL_SCANCODE_KP_PLUS, KeyCode::PLUS_PAD },
        { SDL_SCANCODE_KP_ENTER, KeyCode::ENTER_PAD },
        { SDL_SCANCODE_KP_0, KeyCode::PAD_0 },
        { SDL_SCANCODE_KP_1, KeyCode::PAD_1 },
        { SDL_SCANCODE_KP_2, KeyCode::PAD_2 },
        { SDL_SCANCODE_KP_3, KeyCode::PAD_3 },
        { SDL_SCANCODE_KP_4, KeyCode::PAD_4 },
        { SDL_SCANCODE_KP_5, KeyCode::PAD_5 },
        { SDL_SCANCODE_KP_6, KeyCode::PAD_6 },
        { SDL_SCANCODE_KP_7, KeyCode::PAD_7 },
        { SDL_SCANCODE_KP_8, KeyCode::PAD_8 },
        { SDL_SCANCODE_KP_9, KeyCode::PAD_9 },
        { SDL_SCANCODE_KP_PERIOD, KeyCode::DELETE_PAD },

        { SDL_SCANCODE_SLASH, KeyCode::FORWARD_SLASH },

        { SDL_SCANCODE_F1, KeyCode::F1 },
        { SDL_SCANCODE_F2, KeyCode::F2 },
        { SDL_SCANCODE_F3, KeyCode::F3 },
        { SDL_SCANCODE_F4, KeyCode::F4 },
        { SDL_SCANCODE_F5, KeyCode::F5 },
        { SDL_SCANCODE_F6, KeyCode::F6 },
        { SDL_SCANCODE_F7, KeyCode::F7 },
        { SDL_SCANCODE_F8, KeyCode::F8 },
        { SDL_SCANCODE_F9, KeyCode::F9 },
        { SDL_SCANCODE_F10, KeyCode::F10 },
        { SDL_SCANCODE_F11, KeyCode::F11 },
        { SDL_SCANCODE_F12, KeyCode::F12 },

        { SDL_SCANCODE_PAUSE, KeyCode::BREAK },

        { SDL_SCANCODE_RIGHT, KeyCode::RIGHT_ARROW },
        { SDL_SCANCODE_LEFT, KeyCode::LEFT_ARROW },
        { SDL_SCANCODE_DOWN, KeyCode::DOWN_ARROW },
        { SDL_SCANCODE_UP, KeyCode::UP_ARROW },

        { SDL_SCANCODE_LALT, KeyCode::LEFT_ALT },
        { SDL_SCANCODE_RALT, KeyCode::RIGHT_ALT },
        { SDL_SCANCODE_LSHIFT, KeyCode::LEFT_SHIFT },
        { SDL_SCANCODE_RSHIFT, KeyCode::RIGHT_SHIFT },
        { SDL_SCANCODE_LCTRL, KeyCode::LEFT_CONTROL },
        { SDL_SCANCODE_RCTRL, KeyCode::RIGHT_CONTROL },
        { SDL_SCANCODE_CAPSLOCK, KeyCode::CAPS_LOCK },

        { SDL_SCANCODE_RETURN, KeyCode::ENTER },
        { SDL_SCANCODE_ESCAPE, KeyCode::ESCAPE },
        { SDL_SCANCODE_BACKSPACE, KeyCode::BACK_SPACE },
        { SDL_SCANCODE_TAB, KeyCode::TAB },
        { SDL_SCANCODE_SPACE, KeyCode::SPACE },

        { SDL_SCANCODE_GRAVE, KeyCode::GRAVE },

        { SDL_SCANCODE_INSERT, KeyCode::INSERT },
        { SDL_SCANCODE_DELETE, KeyCode::DELETE },
        { SDL_SCANCODE_HOME, KeyCode::HOME },
        { SDL_SCANCODE_END, KeyCode::END },
        { SDL_SCANCODE_PAGEUP, KeyCode::PAGE_UP },
        { SDL_SCANCODE_PAGEDOWN, KeyCode::PAGE_DOWN },
    };

    const auto it = map.find(sdlCode);
    if (it != map.cend())
        return it->second;

    return Device::KeyCode::UNKNOWN;
}
