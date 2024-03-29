/*
 * M A P P E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/mapper.hpp"

MachLogMapper::MachLogMapper()
{

    TEST_INVARIANT;
}

MachLogMapper::~MachLogMapper()
{
    TEST_INVARIANT;
}

// static
MachPhys::MachineType MachLogMapper::mapToPhysMachine(MachLog::ObjectType type)
{
    PRE(isMachine(type));

    MachPhys::MachineType result = MachPhys::ADMINISTRATOR;

    switch (type)
    {
        case MachLog::ADMINISTRATOR:
            result = MachPhys::ADMINISTRATOR;
            break;
        case MachLog::AGGRESSOR:
            result = MachPhys::AGGRESSOR;
            break;
        case MachLog::CONSTRUCTOR:
            result = MachPhys::CONSTRUCTOR;
            break;
        case MachLog::GEO_LOCATOR:
            result = MachPhys::GEO_LOCATOR;
            break;
        case MachLog::SPY_LOCATOR:
            result = MachPhys::SPY_LOCATOR;
            break;
        case MachLog::TECHNICIAN:
            result = MachPhys::TECHNICIAN;
            break;
        case MachLog::RESOURCE_CARRIER:
            result = MachPhys::RESOURCE_CARRIER;
            break;
        case MachLog::APC:
            result = MachPhys::APC;
            break;
            DEFAULT_ASSERT_BAD_CASE(type);
    }

    return result;
}

// static
MachPhys::ConstructionType MachLogMapper::mapToPhysConstruction(MachLog::ObjectType type)
{
    PRE(isConstruction(type));

    MachPhys::ConstructionType result = MachPhys::BEACON;

    switch (type)
    {
        case MachLog::BEACON:
            result = MachPhys::BEACON;
            break;
        case MachLog::FACTORY:
            result = MachPhys::FACTORY;
            break;
        case MachLog::GARRISON:
            result = MachPhys::GARRISON;
            break;
        case MachLog::HARDWARE_LAB:
            result = MachPhys::HARDWARE_LAB;
            break;
        case MachLog::POD:
            result = MachPhys::POD;
            break;
        case MachLog::MINE:
            result = MachPhys::MINE;
            break;
        case MachLog::MISSILE_EMPLACEMENT:
            result = MachPhys::MISSILE_EMPLACEMENT;
            break;
        case MachLog::SMELTER:
            result = MachPhys::SMELTER;
            break;
            DEFAULT_ASSERT_BAD_CASE(type);
    }

    return result;
}

// static
bool MachLogMapper::isMachine(MachLog::ObjectType type)
{
    bool result = type == MachLog::ADMINISTRATOR || type == MachLog::AGGRESSOR || type == MachLog::CONSTRUCTOR
        || type == MachLog::GEO_LOCATOR || type == MachLog::SPY_LOCATOR || type == MachLog::TECHNICIAN
        || type == MachLog::RESOURCE_CARRIER || type == MachLog::APC;

    return result;
}

// static
bool MachLogMapper::isConstruction(MachLog::ObjectType type)
{
    bool result = type == MachLog::BEACON || type == MachLog::FACTORY || type == MachLog::GARRISON
        || type == MachLog::HARDWARE_LAB || type == MachLog::POD || type == MachLog::MINE
        || type == MachLog::MISSILE_EMPLACEMENT || type == MachLog::SMELTER;

    return result;
}

void MachLogMapper::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogMapper& t)
{

    o << "MachLogMapper " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogMapper " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End MAPPER.CPP ***************************************************/
