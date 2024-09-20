// STL helper header.
// Copyright (c) 1994,1995 by ObjectSpace, Inc.  All rights reserved.
// Email: sales@objectspace.com

#ifndef _CTL_ALGORITH_HPP
#define _CTL_ALGORITH_HPP

#include <algorithm>
// #include "ctl/numeric.hpp"

#ifdef _INLINE
#define _CODE_INLINE inline
#else
#define _CODE_INLINE
#endif

//////////////////////////////////////////////////////////////////////

// This header defines several "helper" algorithms that can be used
// with STL implementations.  The idea is to make some of the more
// common operations easier to use and less error prone.

// Permission is granted to anyone to use this software for any
// purpose on any computer system, and to redistribute it freely,
// as long as the ObjectSpace copyright is included in the source.
// There is no guarantee provided or implied for these algorithms.

// If your compiler has problems with the Container::size_type and
// Container::difference_type typedefs, try changing them to
// size_t and ptrdiff_t respectively.

//////////////////////////////////////////////////////////////////////

template <class Container, class T> _CODE_INLINE void ctl_append(Container* cPtr, const T& t)
{
    cPtr->insert(cPtr->end(), t);
}

//////////////////////////////////////////////////////////////////////

template <class Container, class T>
// inline
// Container::size_type
size_t ctl_count(const Container& c, const T& value)
{
    //  Container::size_type n = 0;
    size_t n = 0;
    // TODO std::count( c.begin(), c.end(), value, n );
    n = std::count(c.begin(), c.end(), value);
    return n;
}

template <class Container, class Predicate>
inline
    // Container::size_type
    size_t
    ctl_count_if(const Container& c, Predicate pred)
{
    typename Container::size_type n = 0;
    count_if(c.begin(), c.end(), pred, n);
    return n;
}

//////////////////////////////////////////////////////////////////////
//  The ctl_erase methods have been removed because they do not work
//  properly on associative containers - what is worse they compile
//  but then fail in action. When we get a compiler which allows
//  partial specialisation we will re introduce them.
//////////////////////////////////////////////////////////////////////

template <class CONTAINER, class T> inline bool ctl_contains(CONTAINER* cPtr, const T& value)
{
    return find(cPtr->begin(), cPtr->end(), value) != cPtr->end();
}

//////////////////////////////////////////////////////////////////////

// SYNOPSIS
//    Apply a function to every element in a container.
// DESCRIPTION
//    Apply `f` to every element in container `c` and return the
//    input parameter `f`.
// SEE ALSO
//    for_each()

template <class Container, class Function> inline Function ctl_for_each(Container& c, Function f)
{
    return for_each(c.begin(), c.end(), f);
}

//////////////////////////////////////////////////////////////////////

// SYNOPSIS
//    Replace specified values in a container that satisfy a predicate.
// DESCRIPTION
//    Replaces every element in the container `c` that causes `pred`
//    to evaluate to `true` with `new_value`.
// SEE ALSO
//    replace_if()

template <class Container, class Predicate, class T>
inline void ctl_replace_if(Container& c, Predicate pred, const T& new_value)
{
    replace_if(c.begin(), c.end(), pred, new_value);
}

//////////////////////////////////////////////////////////////////////

// #ifdef _INSTANTIATE_TEMPLATE_FUNCTIONS
//     #include "ctl/algorith.ctf"
// #endif

#endif
