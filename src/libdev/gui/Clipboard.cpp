#include "gui/Clipboard.hpp"

#include <SDL3/SDL_clipboard.h>

// static
GuiClipboard& GuiClipboard::instance()
{
    static GuiClipboard instance_;
    return instance_;
}

std::string GuiClipboard::getText() const
{
    if (!SDL_HasClipboardText())
        return {};

    char* rawText = SDL_GetClipboardText();
    if (!rawText)
        return {};

    std::string result(rawText);
    SDL_free(rawText);
    return result;
}
