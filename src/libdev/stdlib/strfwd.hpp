// Include this file instead of <string.hpp> when you can
// get away with a foward reference to string.

#ifndef _STDLIB_STRFWD_HPP
#define _STDLIB_STRFWD_HPP

template <class T> class basic_string;
using string = basic_string<char>;

#endif //_STDLIB_STRFWD_HPP