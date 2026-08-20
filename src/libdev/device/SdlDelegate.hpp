#pragma once

#include <SDL3/SDL.h>

struct SdlDelegate
{
    SdlDelegate() { }
    virtual ~SdlDelegate() { }

    //////////////////////////////////////////////////////////////////////

    virtual bool showCursor(const bool show) { return show ? SDL_ShowCursor() : SDL_HideCursor(); }

    virtual void moveCursorToPosition(SDL_Window* window, const int x, const int y)
    {
        return SDL_WarpMouseInWindow(window, x, y);
    }
};
