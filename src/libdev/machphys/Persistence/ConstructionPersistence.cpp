/*
 * C O N S P E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Persistence/ConstructionPersistence.hpp"

#include "machphys/Constructions/Beacon.hpp"
#include "machphys/Constructions/Factory.hpp"
#include "machphys/Constructions/Garrison.hpp"
#include "machphys/Constructions/Mine.hpp"
#include "machphys/Constructions/MissileEmplacement.hpp"
#include "machphys/Constructions/Pod.hpp"
#include "machphys/Constructions/Smelter.hpp"
#include "machphys/Constructions/HardwareLab.hpp"
#include "machphys/Weapons/Missile.hpp"

PER_DEFINE_PERSISTENT(MachPhysConstructionPersistence);

// static
MachPhysConstructionPersistence& MachPhysConstructionPersistence::instance()
{
    static MachPhysConstructionPersistence instance_;
    return instance_;
}

MachPhysConstructionPersistence::MachPhysConstructionPersistence()
{

    TEST_INVARIANT;
}

MachPhysConstructionPersistence::~MachPhysConstructionPersistence()
{
    TEST_INVARIANT;
}

void MachPhysConstructionPersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysConstructionPersistence& t)
{

    o << "MachPhysConstructionPersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysConstructionPersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachPhysConstructionPersistence&)
{
    ostr << MachPhysBeacon::factory();
    ostr << MachPhysFactory::factory();
    ostr << MachPhysGarrison::factory();
    ostr << MachPhysMine::factory();
    ostr << MachPhysMissileEmplacement::factory();
    ostr << MachPhysPod::factory();
    ostr << MachPhysSmelter::factory();
    ostr << MachPhysHardwareLab::factory();
    ostr << MachPhysMissile::factory();
}

void perRead(PerIstream& istr, MachPhysConstructionPersistence&)
{
    istr >> MachPhysBeacon::factory();
    istr >> MachPhysFactory::factory();
    istr >> MachPhysGarrison::factory();
    istr >> MachPhysMine::factory();
    istr >> MachPhysMissileEmplacement::factory();
    istr >> MachPhysPod::factory();
    istr >> MachPhysSmelter::factory();
    istr >> MachPhysHardwareLab::factory();
    istr >> MachPhysMissile::factory();
}

/* End CONSPER.CPP **************************************************/
