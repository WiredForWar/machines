/*
 * P E R W E A P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "base/Persistence.hpp"

#include "machphys/machphys.hpp"

#include "machlog/World/MachLog.hpp"
#include "machlog/Persistence/PersistWeapons.hpp"
#include "machlog/Combat/ElectroCharger.hpp"
#include "machlog/Combat/Bolter.hpp"
#include "machlog/Combat/FlameThrower.hpp"
#include "machlog/Combat/MultiLaunch.hpp"
#include "machlog/Combat/PlasmaWeapon.hpp"
#include "machlog/Combat/PulseWeapon.hpp"
#include "machlog/Combat/VortexWeapon.hpp"
#include "machlog/Combat/IonWeapon.hpp"
#include "machlog/Combat/SuperCharger.hpp"
#include "machlog/Operations/TreacheryOperation.hpp"
#include "machlog/Combat/NuclearWeapon.hpp"
#include "machlog/Combat/PunchWeapon.hpp"
#include "machlog/Combat/MultiLauncher.hpp"
#include "machlog/Combat/BeeWeapon.hpp"
#include "machlog/Combat/LightStingWeapon.hpp"
#include "machlog/Combat/MetalStingWeapon.hpp"

MachLogWeaponPersistence::MachLogWeaponPersistence()
{

    TEST_INVARIANT;
}

MachLogWeaponPersistence::~MachLogWeaponPersistence()
{
    TEST_INVARIANT;
}

void MachLogWeaponPersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogWeaponPersistence& t)
{

    o << "MachLogWeaponPersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogWeaponPersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// static
void MachLogWeaponPersistence::registerDerivedClasses()
{
    PER_REGISTER_DERIVED_CLASS(MachLogBolterWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogElectroCharger);
    PER_REGISTER_DERIVED_CLASS(MachLogIonWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogVortexWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogMultiLauncher);
    PER_REGISTER_DERIVED_CLASS(MachLogPulseWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogPlasmaWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogFlameThrower);
    PER_REGISTER_DERIVED_CLASS(MachLogSuperCharger);
    PER_REGISTER_DERIVED_CLASS(MachLogTreacheryWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogNuclearWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogPunchWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogLargeMissileLauncher);
    PER_REGISTER_DERIVED_CLASS(MachLogBeeBomber);
    PER_REGISTER_DERIVED_CLASS(MachLogLightStingWeapon);
    PER_REGISTER_DERIVED_CLASS(MachLogMetalStingWeapon);
}
/* End PERWEAP.CPP **************************************************/
