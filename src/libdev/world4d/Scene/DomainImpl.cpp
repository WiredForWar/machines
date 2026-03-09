/*
 * D O M A I N I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <algorithm>
#include "world4d/Subject/Subject.hpp"
#include "world4d/Subject/Observer.hpp"
#include "world4d/Scene/Internal/DomainImpl.hpp"
#include "world4d/Entity/EntityFilter.hpp"

PER_DEFINE_PERSISTENT(W4dDomainImpl);

W4dDomainImpl::W4dDomainImpl()
{

    TEST_INVARIANT;
}

W4dDomainImpl::~W4dDomainImpl()
{
    TEST_INVARIANT;
}
void W4dDomainImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const W4dDomainImpl& t)
{

    o << "W4dDomainImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "W4dDomainImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const W4dDomainImpl& t)
{
    ostr << t.portals_;

    // Not persisted
    // intersectingEntities_
    // observers_
    // subjects_
}

void perRead(PerIstream& istr, W4dDomainImpl& t)
{
    istr >> t.portals_;
}

W4dDomainImpl::W4dDomainImpl(PerConstructor)
{
}

/* End DOMAINI.CPP **************************************************/
