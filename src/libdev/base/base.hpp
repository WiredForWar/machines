/*
 * B A S E . H P P
 * (c) Charybdis Limited, 1995,1996. All Rights Reserved.
 */

#ifndef _BASE_HPP
#define _BASE_HPP

//////////////////////////////////////////////////////////////////////////

#define PREFIX
#define POSTFIX int

#include "base/compiler.hpp"

//////////////////////////////////////////////////////////////////////

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

//////////////////////////////////////////////////////////////////////

#include <ostream>

// #defines //////////////////////////////////////////////////////////

#define INSPECT_ON(ostream, var) (ostream << #var " :\t" << (var) << std::endl)

///////////////////////////////////

#define _STR(x) _VAL(x)
#define _VAL(x) #x

///////////////////////////////////

#include "base/PrePost.hpp"

#include "base/MemChk.hpp"

//////////////////////////////////////////////////////////////////////////

// extensions to iso646 operators

#define nor(a, b) (not((a) or (b)))
#define nand(a, b) (not((a) and (b)))
// #define xor( a, b )     (  (a) ? !(b) : !!(b) )
#define iff(a, b) (static_cast<bool>(a) == static_cast<bool>(b))
#define implies(a, b) (static_cast<bool>(a) ? static_cast<bool>(b) : true)

// #defines for documentation purposes
#define _ABSTACT
#define _INSTANCE
#define _LEAF

// array must be statically allocated storage
#define elementsof(array) (sizeof(array) / sizeof(array[0]))

//////////////////////////////////////////////////////////////////////

#ifndef NO_ENUM_OVERLOAD

#define _BITMASK(E, T)                                                                                                 \
    E& operator&=(E& _X, E& _Y)                                                                                        \
    {                                                                                                                  \
        _X = (E)(_X & _Y);                                                                                             \
        return (_X);                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    E& operator|=(E& _X, E& _Y)                                                                                        \
    {                                                                                                                  \
        _X = (E)(_X | _Y);                                                                                             \
        return (_X);                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    E& operator^=(E& _X, E& _Y)                                                                                        \
    {                                                                                                                  \
        _X = (E)(_X ^ _Y);                                                                                             \
        return (_X);                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    E operator&(E _X, E _Y)                                                                                            \
    {                                                                                                                  \
        return ((E)(_X & _Y );                                                                                         \
    }                                                                                                                  \
                                                                                                                       \
    E operator|(E _X, E _Y)                                                                                            \
    {                                                                                                                  \
        return ((E)(_X | _Y );                                                                                         \
    }                                                                                                                  \
                                                                                                                       \
    E operator&(E _X, E _Y)                                                                                            \
    {                                                                                                                  \
        return ((E)(_X & _Y );                                                                                         \
    }                                                                                                                  \
    typedef E T

#endif /* #ifndef NO_ENUM_OVERLOAD */

//////////////////////////////////////////////////////////////////////

// Macros make switching between pImpl_ and no pImpl_ very easy.
#define CB_DEPIMPL_AUTO(varname)                                                                                       \
    PRE(pImpl_)                                                                                                        \
    DECL_UNUSED auto& varname = pImpl_->varname;

#define CB_DEPIMPL(vartype, varname)                                                                                   \
    PRE(pImpl_)                                                                                                        \
    DECL_UNUSED vartype& varname = pImpl_->varname;

// If data member is "int var[10]" then pass vartype as int.
#define CB_DEPIMPL_ARRAY(vartype, varname)                                                                             \
    PRE(pImpl_)                                                                                                        \
    DECL_UNUSED vartype* varname = pImpl_->varname;

#define CB_DEPIMPL_2D_ARRAY(vartype, varname, secondDimension)                                                         \
    PRE(pImpl_);                                                                                                       \
    DECL_UNUSED vartype(*varname)[secondDimension] = pImpl_->varname;

//////////////////////////////////////////////////////////////////////

#endif /*  #ifndef BASE_HPP    */
