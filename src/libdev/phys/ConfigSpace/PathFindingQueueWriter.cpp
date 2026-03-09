/*
 * C S 2 Q W R I T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "phys/ConfigSpace/PathFindingQueueWriter.hpp"
#include "mathex/poly2d.hpp"
#include "mathex/point2d.hpp"
#include "phys/ConfigSpace/ConfigSpace2d.hpp"
#include "phys/ConfigSpace/Internal/ConfigSpace2dImpl.hpp"
#include "phys/ConfigSpace/Internal/DomainFindPath.hpp"
#include "phys/ConfigSpace/Internal/FindPath.hpp"

PhysPathFindingQueueWriter::PhysPathFindingQueueWriter(const PhysConfigSpace2d& configSpace)
    : configSpace_(configSpace)
{

    TEST_INVARIANT;
}

PhysPathFindingQueueWriter::~PhysPathFindingQueueWriter()
{
    TEST_INVARIANT;
}

void PhysPathFindingQueueWriter::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const PhysPathFindingQueueWriter& t)
{
    const PhysCS2dImpl::DomainFindPaths& domainFindPathQueue = t.configSpace_.impl().domainFindPathQueue_;

    o << "Domain find path queue length " << domainFindPathQueue.size() << std::endl;

    for (PhysCS2dImpl::DomainFindPaths::const_iterator i = domainFindPathQueue.begin(); i != domainFindPathQueue.end();
         ++i)
    {
        o << "  " << (*(*i)) << std::endl;
    }

    const PhysCS2dImpl::FindPaths& findPathQueue = t.configSpace_.impl().findPathQueue_;

    o << "Find path queue length " << findPathQueue.size() << std::endl;

    for (PhysCS2dImpl::FindPaths::const_iterator i = findPathQueue.begin(); i != findPathQueue.end(); ++i)
    {
        o << "  " << (*(*i)) << std::endl;
    }

    return o;
}

/* End QWRITE.CPP ***************************************************/
