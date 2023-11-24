/*
 * I O N B E A M I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/internal/ionbeami.hpp"

#include "machphys/machphys.hpp"
#include "machphys/mphydata.hpp"

MachLogIonBeamImpl::MachLogIonBeamImpl(const MachPhysWeaponData* const pWeaponData)
    : pWeaponData_(pWeaponData)
    , pPhysIonBeam_(nullptr)
{
    TEST_INVARIANT;
}

MachLogIonBeamImpl::~MachLogIonBeamImpl()
{
    TEST_INVARIANT;
}

void MachLogIonBeamImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogIonBeamImpl& t)
{

    o << "MachLogIonBeamImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogIonBeamImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End IONBEAMI.CPP *************************************************/
