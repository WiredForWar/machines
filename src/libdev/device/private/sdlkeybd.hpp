#ifndef DEVICE_SDL_KEYBOARD_HPP
#define DEVICE_SDL_KEYBOARD_HPP

#include "device/butevent.hpp"

#include "device/private/shrkeybd.hpp"

#include <SDL.h>

//////////////////////////////////////////////////////////////////////
// Implements the mechanism for determining key-presses under SDL2.0
// The public interface is entirely supplied by DevKeyboard.
class DevSdlKeyboard : public DevKeyboard
{
private:
    friend DevKeyboard& DevKeyboard::instance();
    static DevSdlKeyboard& sdlInstance();
    DevSdlKeyboard();
    ~DevSdlKeyboard();

    // The Windows proceedure communicates key-press events to this
    // class using these messages.
    friend class AfxSdlApp;
    void wm_key(const DevButtonEvent& ev);
    void wm_char(const DevButtonEvent& ev);
    void wm_killfocus();

    static ScanCode translateScanCode(SDL_Scancode sdlCode);
};

#undef DevButtonEvent

#endif

///////////////////////////////////
