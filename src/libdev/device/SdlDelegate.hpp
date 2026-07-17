#pragma once

#include <utility>
#include <SDL3/SDL.h>

struct SdlDelegate
{
    SdlDelegate() { }
    virtual ~SdlDelegate() { }

    //////////////////////////////////////////////////////////////////////

    virtual bool showCursor(const bool show) { return show ? SDL_ShowCursor() : SDL_HideCursor(); }

    virtual std::pair<int, int> getCursorPosition()
    {
        float x, y;
        SDL_GetMouseState(&x, &y);
        return std::make_pair(static_cast<int>(x), static_cast<int>(y));
    }

    virtual void moveCursorToPosition(SDL_Window* window, const int x, const int y)
    {
        return SDL_WarpMouseInWindow(window, x, y);
    }
};
