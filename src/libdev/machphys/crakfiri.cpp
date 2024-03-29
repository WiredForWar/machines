/*
 * C R A K F I R I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/internal/crakfiri.hpp"

PER_DEFINE_PERSISTENT(MachPhysCrackFireImpl);

MachPhysCrackFireImpl::MachPhysCrackFireImpl(const MATHEX_SCALAR& length)
    : pCrack_(nullptr)
    , pFire_(nullptr)
    , length_(length)
{

    TEST_INVARIANT;
}

MachPhysCrackFireImpl::MachPhysCrackFireImpl()
    : pCrack_(nullptr)
    , pFire_(nullptr)
    , length_(0)
{

    TEST_INVARIANT;
}

MachPhysCrackFireImpl::~MachPhysCrackFireImpl()
{
    TEST_INVARIANT;
}

void MachPhysCrackFireImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysCrackFireImpl& t)
{

    o << "MachPhysCrackFireImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysCrackFireImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

MachPhysCrackFireImpl::MachPhysCrackFireImpl(PerConstructor)
{
}

void perWrite(PerOstream& ostr, const MachPhysCrackFireImpl& crackFire)
{
    ostr << crackFire.pCrack_;
    ostr << crackFire.pFire_;
    ostr << crackFire.length_;
    ostr << crackFire.debris_;
}

void perRead(PerIstream& istr, MachPhysCrackFireImpl& crackFire)
{
    istr >> crackFire.pCrack_;
    istr >> crackFire.pFire_;
    istr >> crackFire.length_;
    istr >> crackFire.debris_;
}

/* End CRAKFIRI.CPP *************************************************/
