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

#ifdef __GNUC__
    #define WEAK_SYMBOL __attribute__((weak))
#else
    #define WEAK_SYMBOL
#endif

#endif  /*  #ifndef COMPILER_HPP    */
