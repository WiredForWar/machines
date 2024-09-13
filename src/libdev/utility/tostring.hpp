
#ifndef _UTL_TOSTRING_HPP
#define _UTL_TOSTRING_HPP

#include "base/base.hpp"

// #include <string.hpp>
#include "stdlib/string.hpp"
#include <utility>

#include "utility/subrange.hpp"

//////////////////////////////////////////////////////////////////////

UTL_INT_SUBRANGE(2, 35, UtlRadix);

std::string utlToString(int i);
std::string utlToString(std::pair<int, UtlRadix> p);

std::string utlToString(unsigned i);
std::string utlToString(std::pair<unsigned, UtlRadix> p);

std::string utlToString(long i);
std::string utlToString(std::pair<long, UtlRadix> p);

std::string utlToString(unsigned long i);
std::string utlToString(std::pair<unsigned long, UtlRadix> p);

std::string utlToString(float);
std::string utlToString(double);
std::string utlToString(long double);

//////////////////////////////////////////////////////////////////////

#endif // #ifndef _UTL_TOSTRING_HPP

/* End TOSTRING.HPP *************************************************/
