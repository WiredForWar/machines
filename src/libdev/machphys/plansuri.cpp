/*
 * P L A N S U R I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/internal/plansuri.hpp"
#include "world4d/root.hpp"

PER_DEFINE_PERSISTENT(MachPhysPlanetSurfaceImpl);

MachPhysPlanetSurfaceImpl::MachPhysPlanetSurfaceImpl()
    : minMachineHeight_(-10000.0)
    , pPersistenceRoot_(new W4dRoot(133))
{

    TEST_INVARIANT;
}

MachPhysPlanetSurfaceImpl::~MachPhysPlanetSurfaceImpl()
{
    TEST_INVARIANT;
    delete pPersistenceRoot_;
}

void MachPhysPlanetSurfaceImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysPlanetSurfaceImpl& t)
{

    o << "MachPhysPlanetSurfaceImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysPlanetSurfaceImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

MachPhysPlanetSurfaceImpl::MachPhysPlanetSurfaceImpl(PerConstructor)
    : minMachineHeight_(-10000.0)
{
}

void perWrite(PerOstream& ostr, const MachPhysPlanetSurfaceImpl& t)
{
    ostr << t.minMachineHeight_;
    ostr << t.pPersistenceRoot_;
}

void perRead(PerIstream& istr, MachPhysPlanetSurfaceImpl& t)
{
    istr >> t.minMachineHeight_;
    istr >> t.pPersistenceRoot_;
}

/* End PLANSURI.CPP *************************************************/
