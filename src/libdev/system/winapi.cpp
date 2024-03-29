/*
 * W I N A P I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "system/winapi.hpp"
#include <SDL.h>

SysWindowsAPI::SysWindowsAPI()
{

    TEST_INVARIANT;
}

SysWindowsAPI::~SysWindowsAPI()
{
    TEST_INVARIANT;
}

void SysWindowsAPI::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const SysWindowsAPI& t)
{

    o << "SysWindowsAPI " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "SysWindowsAPI " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// static
void SysWindowsAPI::sleep(double milliseconds)
{
    // Sleep( milliseconds );
}

// static
void SysWindowsAPI::messageBox(const char* pMessage, const char* pTitle)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, pTitle, pMessage, nullptr);
}

// static
void SysWindowsAPI::messageBoxError(const char* pMessage, const char* pTitle)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, pTitle, pMessage, nullptr);
}

// static
void SysWindowsAPI::peekMessage()
{
}

/* End WINAPI.CPP ***************************************************/
