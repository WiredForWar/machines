#pragma once

#include <SDL2/SDL_endian.h>

namespace System
{

template <typename T>
concept IntegralType = requires { requires std::is_integral_v<T>; };

template <IntegralType T> T swapEndian(T source)
{
    if constexpr (sizeof(T) == 2)
    {
        return SDL_Swap16(source);
    }
    else if constexpr (sizeof(T) == 4)
    {
        return SDL_Swap32(source);
    }
    else if constexpr (sizeof(T) == 8)
    {
        return SDL_Swap64(source);
    }
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
template <IntegralType T> inline constexpr T toBigEndian(T source)
{
    return source;
}

template <IntegralType T> inline constexpr T fromBigEndian(T source)
{
    return source;
}

#else // SDL_LIL_ENDIAN
template <IntegralType T> inline constexpr T toBigEndian(T source)
{
    return swapEndian(source);
}

template <IntegralType T> inline constexpr T fromBigEndian(T source)
{
    return swapEndian(source);
}

#endif

} // namespace System
