/*
 * C S 2 D M A R C . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "phys/internal/cs2dmarc.hpp"

#ifndef _INLINE
#include "phys/internal/cs2dmarc.ipp"
#endif

//////////////////////////////////////////////////////////////////////////////////////////

void PhysCS2dDomainArc::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}
//////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const PhysCS2dDomainArc& t)
{

    o << "PhysCS2dDomainArc " << static_cast<const void*>(&t) << " domain id = " << t.domainId_.asScalar() << std::endl;

    return o;
}
//////////////////////////////////////////////////////////////////////////////////////////
/* End CS2DMARC.CPP *************************************************/
