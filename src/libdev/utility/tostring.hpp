
#ifndef _UTL_TOSTRING_HPP
#define _UTL_TOSTRING_HPP

#include "utility/subrange.hpp"

#include <string>
#include <utility>

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
