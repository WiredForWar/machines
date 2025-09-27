/*
 * C O M P I L E R . H P P
 * (c) Charybdis Limited, 1995,1996. All Rights Reserved.
 */

#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <cstdint>

//////////////////////////////////////////////////////////////////////

// Disable exceptions in Charybdis code
#define NO_EXCEPTIONS 1

//////////////////////////////////////////////////////////////////////

using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using int32 = int32_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

//////////////////////////////////////////////////////////////////////

#define _SDLAPP

#if defined(_WIN32)
#define MACHINES_PLATFORM_WINDOWS
#elif defined(__linux__)
#define MACHINES_PLATFORM_LINUX
#else
#error "Unsupported platform"
#endif

#if defined(__GNUC__)
#define MACHINES_COMPILER_GCC
#elif defined(_MSC_VER)
#define MACHINES_COMPILER_MSVC
#endif

#if defined(__GNUC__) && !defined(__MINGW32__)
#define WEAK_SYMBOL __attribute__((weak))
#define DECL_DEPRECATED __attribute__((__deprecated__))
#else
#define WEAK_SYMBOL
#define DECL_DEPRECATED
#endif

#define DECL_UNUSED [[maybe_unused]]

#if defined(_MSC_VER)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#endif /*  #ifndef COMPILER_HPP    */
