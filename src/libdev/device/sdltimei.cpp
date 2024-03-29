/*
 * W 9 5 T I M E I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <ctime>
#include <cstdio>

#include "device/internal/sdltimei.hpp"

DevSdlTimeInternal::DevSdlTimeInternal()
{
    method_ = TIME_GET_TIME;
    resolution_ = 0.001;

    TEST_INVARIANT;
}

DevSdlTimeInternal::~DevSdlTimeInternal()
{
    TEST_INVARIANT;
}

void DevSdlTimeInternal::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const DevSdlTimeInternal& t)
{

    o << "DevSdlTimeInternal " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "DevSdlTimeInternal " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End W95TIMEI.CPP *************************************************/
