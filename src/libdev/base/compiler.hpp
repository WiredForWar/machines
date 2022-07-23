/*
 * C O M P I L E R . H P P
 * (c) Charybdis Limited, 1995,1996. All Rights Reserved.
 */

#ifndef COMPILER_HPP
#define COMPILER_HPP

//////////////////////////////////////////////////////////////////////

// Disable exceptions in Charybdis code
#define NO_EXCEPTIONS 1

//////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
    #if __GNUC__ >= 5
        //#include "base/watc1060.hpp"
        #define _COMPILER_VERSION
    #elif defined (__clang__)   // Appease Qt Creator's code model
        #define _COMPILER_VERSION
    #endif

    #define _COMPILER_NAME  GCC

    #if defined(__x86_64__)
        /* 64 bit detected */
        #include "base/limitw64.hpp"
    #endif
    #if defined(__aarch64__)
        /* 64 bit arm detected */
        #include "base/limitw64.hpp"
    #endif
    #if defined(__i386__)
        /* 32 bit x86 detected */
        #include "base/limitw32.hpp"
    #endif
    #if defined(__arm__)
        /* 32 bit arm detected */
        #include "base/limitw32.hpp"
    #endif

    //#define _WIN95APP
    #define _SDLAPP

    #define WEAK_SYMBOL __attribute__((weak))
#else
    #define WEAK_SYMBOL
#endif

//////////////////////////////////////////////////////////////////////

#ifdef NO_BOOL
    #include "base/bool.h"
#endif  /* #ifdef NO_BOOL   */

#ifdef NO_ISO646_DEFINITIONS
    #include "base/iso646.h"
#endif  /* #ifdef NO_ISO646_DEFINITIONS */

//////////////////////////////////////////////////////////////////////

#ifndef _COMPILER_NAME
    #error This compiler is not catered for in file "base/compiler.hpp".
#endif  /*  #ifndef _COMPILER_NAME  */

#ifndef _COMPILER_VERSION
    #error This version of the compiler is not catered for in file "base/compiler.hpp".
#endif  /*  #ifndef _COMPILER_VERSION   */

//////////////////////////////////////////////////////////////////////

#ifndef _DOSAPP
    #ifndef _WIN95APP
        #ifndef _SDLAPP
            #error No operating system defined
        #endif
    #endif
#endif

//////////////////////////////////////////////////////////////////////

#endif  /*  #ifndef COMPILER_HPP    */
