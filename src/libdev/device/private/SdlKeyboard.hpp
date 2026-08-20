#ifndef DEVICE_SDL_KEYBOARD_HPP
#define DEVICE_SDL_KEYBOARD_HPP

#include "device/ButtonEvent.hpp"

#include "device/private/SharedKeyboard.hpp"

#include <SDL3/SDL.h>

//////////////////////////////////////////////////////////////////////
// Implements the mechanism for determining key-presses under SDL2.0
// The public interface is entirely supplied by DevKeyboard.
class DevSdlKeyboard : public DevKeyboard
{
public:
    static DevSdlKeyboard& sdlInstance();

    // Report a key going down or up.
    void submitKeyEvent(const DevButtonEvent& ev);

    // Report a typed character.
    void submitCharEvent(const DevButtonEvent& ev);

    // The window has lost input focus. Every key held at this moment counts as
    // released, and no further event will report it.
    void submitFocusLost();

    static ScanCode translateScanCode(SDL_Scancode sdlCode);

private:
    friend DevKeyboard& DevKeyboard::instance();
    DevSdlKeyboard();
    ~DevSdlKeyboard();
};

#undef DevButtonEvent

#endif

///////////////////////////////////
