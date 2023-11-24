/*
 * P U N C H I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/internal/punchi.hpp"

#include "machphys/machphys.hpp"
#include "machphys/mphydata.hpp"
// #include "machlog/race.hpp"
// #include "machlog/races.hpp"
// #include "machlog/actor.hpp"

MachLogPunchBlastImpl::MachLogPunchBlastImpl(const MachPhysWeaponData* const pWeaponData)
    : pWeaponData_(pWeaponData)
    , pPhysPunchBlast_(nullptr)
{
    TEST_INVARIANT;
}

MachLogPunchBlastImpl::~MachLogPunchBlastImpl()
{
    TEST_INVARIANT;
}

void MachLogPunchBlastImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogPunchBlastImpl& t)
{

    o << "MachLogPunchBlastImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogPunchBlastImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End PUNCHI.CPP *************************************************/
