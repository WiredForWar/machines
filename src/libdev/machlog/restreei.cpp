/*
 * R E S T R E E I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/internal/restreei.hpp"

MachLogResearchTreeImpl::MachLogResearchTreeImpl()
{

    TEST_INVARIANT;
}

MachLogResearchTreeImpl::~MachLogResearchTreeImpl()
{
    TEST_INVARIANT;
}

void MachLogResearchTreeImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogResearchTreeImpl& t)
{

    o << "MachLogResearchTreeImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogResearchTreeImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End RESTREEI.CPP *************************************************/
