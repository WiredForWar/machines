/*
 * size2d.cpp
 */

#include <iostream>

#include "mathex/size2d.hpp"

std::ostream& operator<<(std::ostream& o, const MexSize2d& t)
{

    o << "  (" << t.width_ << "," << t.height_ << ")";
    return o;
}
