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

namespace Base
{

// Makes a failed assertion end the process straight away instead of putting up a
// dialog and waiting to be told what to do. For a run nobody is watching: the
// dialog blocks the loop, so the run neither finishes nor reports anything.
//
// What was asserted is in assert.log either way, or in the file CB_ASSERT_TO
// names.
void abortWithoutADialog();

} // namespace Base

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
