/*
 * O P A U T S C I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/internal/opautsci.hpp"

#include "machlog/debris.hpp"
#include "machlog/operatio.hpp"
#include "machlog/rescarr.hpp"

PER_DEFINE_PERSISTENT(MachLogAutoScavengeOperationImpl);

MachLogAutoScavengeOperationImpl::MachLogAutoScavengeOperationImpl(
    MachLogResourceCarrier* pScavenger,
    MachLogDebris* pDebris)
    : pScavenger_(pScavenger)
    , pDebris_(pDebris)
    , finished_(false)
    , initiatedScavengeOp_(false)
{
    PRE(pScavenger->isScavenger());

    TEST_INVARIANT;
}

MachLogAutoScavengeOperationImpl::~MachLogAutoScavengeOperationImpl()
{
    TEST_INVARIANT;
}

void MachLogAutoScavengeOperationImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogAutoScavengeOperationImpl& t)
{

    o << "MachLogAutoScavengeOperationImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogAutoScavengeOperationImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachLogAutoScavengeOperationImpl& AutoScavengeOpImpl)
{
    ostr << AutoScavengeOpImpl.pScavenger_;
    ostr << AutoScavengeOpImpl.pDebris_;
    ostr << AutoScavengeOpImpl.finished_;
    ostr << AutoScavengeOpImpl.initiatedScavengeOp_;
    ostr << AutoScavengeOpImpl.pCachedOperation_.get();
}

void perRead(PerIstream& istr, MachLogAutoScavengeOperationImpl& AutoScavengeOpImpl)
{
    istr >> AutoScavengeOpImpl.pScavenger_;
    istr >> AutoScavengeOpImpl.pDebris_;
    istr >> AutoScavengeOpImpl.finished_;
    istr >> AutoScavengeOpImpl.initiatedScavengeOp_;
    MachLogOperation *operation{};
    istr >> operation;
    AutoScavengeOpImpl.pCachedOperation_.reset(operation);
}

MachLogAutoScavengeOperationImpl::MachLogAutoScavengeOperationImpl(PerConstructor)
{
}

/* End OPAUTSCI.CPP ***************************************************/
