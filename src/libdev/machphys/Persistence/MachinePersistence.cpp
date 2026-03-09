/*
 * M A C H P E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Persistence/MachinePersistence.hpp"

#include "machphys/Machines/Administrator.hpp"
#include "machphys/Machines/Aggressor.hpp"
#include "machphys/Machines/APC.hpp"
#include "machphys/Machines/Constructor.hpp"
#include "machphys/Machines/GeoLocator.hpp"
#include "machphys/Machines/ResourceCarrier.hpp"
#include "machphys/Machines/SpyLocator.hpp"
#include "machphys/Machines/Technician.hpp"

#include "machphys/ObjectFactory.hpp"

PER_DEFINE_PERSISTENT(MachPhysMachinePersistence);

// static
MachPhysMachinePersistence& MachPhysMachinePersistence::instance()
{
    static MachPhysMachinePersistence instance_;
    return instance_;
}

MachPhysMachinePersistence::MachPhysMachinePersistence()
{

    TEST_INVARIANT;
}

MachPhysMachinePersistence::~MachPhysMachinePersistence()
{
    TEST_INVARIANT;
}

void MachPhysMachinePersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysMachinePersistence& t)
{

    o << "MachPhysMachinePersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysMachinePersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachPhysMachinePersistence&)
{
    ostr << MachPhysAdministrator::factory();
    ostr << MachPhysAggressor::factory();
    ostr << MachPhysAPC::factory();
    ostr << MachPhysConstructor::factory();
    ostr << MachPhysGeoLocator::factory();
    ostr << MachPhysResourceCarrier::factory();
    ostr << MachPhysSpyLocator::factory();
    ostr << MachPhysTechnician::factory();
}

void perRead(PerIstream& istr, MachPhysMachinePersistence&)
{
    istr >> MachPhysAdministrator::factory();
    istr >> MachPhysAggressor::factory();
    istr >> MachPhysAPC::factory();
    istr >> MachPhysConstructor::factory();
    istr >> MachPhysGeoLocator::factory();
    istr >> MachPhysResourceCarrier::factory();
    istr >> MachPhysSpyLocator::factory();
    istr >> MachPhysTechnician::factory();
}

/* End MACHPER.CPP **************************************************/
