/*
 * G A R R I S O I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/internal/garrisoi.hpp"
#include "sim/manager.hpp"

PER_DEFINE_PERSISTENT(MachLogGarrisonImpl);

MachLogGarrisonImpl::MachLogGarrisonImpl()
    : lastUpdateTime_(SimManager::instance().currentTime())
{
    TEST_INVARIANT;
}

MachLogGarrisonImpl::~MachLogGarrisonImpl()
{
    TEST_INVARIANT;
}

void MachLogGarrisonImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogGarrisonImpl& t)
{

    o << "MachLogGarrisonImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogGarrisonImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachLogGarrisonImpl& actorImpl)
{
    ostr << actorImpl.lastUpdateTime_;
}

void perRead(PerIstream& istr, MachLogGarrisonImpl& actorImpl)
{
    istr >> actorImpl.lastUpdateTime_;
}

/* End GARRISOI.CPP *************************************************/
