/*
 * A T T A C K I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/internal/attacki.hpp"

#include "world4d/link.hpp"
#include "machphys/machphys.hpp"
#include "machphys/turntrak.hpp"

PER_DEFINE_PERSISTENT(MachPhysCanAttackImpl);

MachPhysCanAttackImpl::MachPhysCanAttackImpl()
    : mountingLinks_(MachPhys::N_MOUNTINGS, nullptr)
    , lastSoundWeapon_(_STATIC_CAST(MachPhys::WeaponType, 0))
    , lastLightWeapon_(_STATIC_CAST(MachPhys::WeaponType, 0))
{

    TEST_INVARIANT;
}

MachPhysCanAttackImpl::~MachPhysCanAttackImpl()
{
    TEST_INVARIANT;

    for (TurnerTrackers::iterator i = turnerTrackers_.begin(); i != turnerTrackers_.end(); ++i)
        delete *i;
}

void MachPhysCanAttackImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysCanAttackImpl& t)
{

    o << "MachPhysCanAttackImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysCanAttackImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachPhysCanAttackImpl& attack)
{
    ostr << attack.mountingLinks_;
    //    ostr << attack.weapons_;
}

void perRead(PerIstream& istr, MachPhysCanAttackImpl& attack)
{
    istr >> attack.mountingLinks_;
    //    istr >> attack.weapons_;
}

/* End ATTACKI.CPP **************************************************/
