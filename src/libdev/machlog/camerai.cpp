/*
 * C A M E R A I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/internal/camerai.hpp"

MachLogCameraImpl::MachLogCameraImpl()
{

    TEST_INVARIANT;
}

MachLogCameraImpl::~MachLogCameraImpl()
{
    TEST_INVARIANT;
}

void MachLogCameraImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogCameraImpl& t)
{

    o << "MachLogCameraImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogCameraImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End CAMERAI.CPP **************************************************/
