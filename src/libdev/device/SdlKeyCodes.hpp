#pragma once

#include "device/Key.hpp"

#include <SDL3/SDL.h>

namespace Device
{

// The key code SDL's scan code stands for, or UNKNOWN for one the game has no
// code for.
KeyCode codeFromSdlScanCode(SDL_Scancode sdlCode);

} // namespace Device
