/*
 * A R M O U R E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/aggressr.hpp"
#include "machphys/administ.hpp"
#include "machphys/missemp.hpp"
#include "machphys/nuclweap.hpp"
#include "machphys/weapon.hpp"
#include "machphys/wepline.hpp"
#include "machphys/pod.hpp"
#include "machphys/charger.hpp"
#include "machphys/mulaunch.hpp"
#include "machphys/punweap.hpp"
#include "machphys/largemsl.hpp"

// can attack classes
#include "machlog/armourer.hpp"
#include "machlog/aggressr.hpp"
#include "machlog/administ.hpp"
#include "machlog/missilem.hpp"
#include "machlog/pod.hpp"

// weapon classes
#include "machlog/pulsewep.hpp"
#include "machlog/bolter.hpp"
#include "machlog/charger.hpp"
#include "machlog/vortweap.hpp"
#include "machlog/ionweap.hpp"
#include "machlog/punweap.hpp"
#include "machlog/treachry.hpp"
#include "machlog/nuclweap.hpp"
#include "machlog/lmisslch.hpp"
#include "machlog/mulaunch.hpp"
#include "machlog/plasmawe.hpp"
#include "machlog/flamethr.hpp"
#include "machlog/elecharg.hpp"
#include "machlog/beeweap.hpp"
#include "machlog/stglwep.hpp"
#include "machlog/stgmwep.hpp"

// static
void MachLogArmourer::arm(MachLogAggressor* pActor)
{
    ASSERT(pActor->objectType() == MachLog::AGGRESSOR, "Not aggressor in armourer::arm\n");
    const MachLogAggressor& constActor = *pActor;
    const MachPhysAggressor& physActor = constActor.physAggressor();
    MachPhysAggressor& nonConstPhysActor = (MachPhysAggressor&)physActor;
    for (int i = 0; i < nonConstPhysActor.nMountedWeapons(); ++i)
    {
        MachPhysWeapon* pPhysWeapon = &nonConstPhysActor.weapon(i);
        pActor->addWeapon(createWeapon((MachLogRace*)&constActor.logRace(), pPhysWeapon, pActor));
    }
}

// static
void MachLogArmourer::rearm(MachLogAggressor* pActor)
{
    ASSERT(pActor->objectType() == MachLog::AGGRESSOR, "Not aggressor in armourer::rearm\n");
    const MachLogAggressor& constActor = *pActor;
    const MachPhysAggressor& physActor = constActor.physAggressor();
    MachPhysAggressor& nonConstPhysActor = (MachPhysAggressor&)physActor;
    for (int i = 0; i < nonConstPhysActor.nMountedWeapons(); ++i)
    {
        MachPhysWeapon* pPhysWeapon = &nonConstPhysActor.weapon(i);
        pActor->weapons()[i]->setPhysicalWeapon(pPhysWeapon);
        checkAndSetSpecialWeapons(pActor->weapons()[i], pPhysWeapon);
    }
}

// static
void MachLogArmourer::arm(MachLogAdministrator* pActor)
{
    ASSERT(pActor->objectType() == MachLog::ADMINISTRATOR, "Not administrator in armourer::arm\n");
    const MachLogAdministrator& constActor = *pActor;
    const MachPhysAdministrator& physActor = constActor.physAdministrator();
    MachPhysAdministrator& nonConstPhysActor = (MachPhysAdministrator&)physActor;
    for (int i = 0; i < nonConstPhysActor.nMountedWeapons(); ++i)
    {
        MachPhysWeapon* pPhysWeapon = &nonConstPhysActor.weapon(i);
        pActor->addWeapon(createWeapon((MachLogRace*)&constActor.logRace(), pPhysWeapon, pActor));
    }
}

// static
void MachLogArmourer::rearm(MachLogAdministrator* pActor)
{
    ASSERT(pActor->objectType() == MachLog::ADMINISTRATOR, "Not administrator in armourer::rearm\n");
    const MachLogAdministrator& constActor = *pActor;
    const MachPhysAdministrator& physActor = constActor.physAdministrator();
    MachPhysAdministrator& nonConstPhysActor = (MachPhysAdministrator&)physActor;
    for (int i = 0; i < nonConstPhysActor.nMountedWeapons(); ++i)
    {
        MachPhysWeapon* pPhysWeapon = &nonConstPhysActor.weapon(i);
        pActor->weapons()[i]->setPhysicalWeapon(pPhysWeapon);
        checkAndSetSpecialWeapons(pActor->weapons()[i], pPhysWeapon);
    }
}

// static
void MachLogArmourer::arm(MachLogMissileEmplacement* pActor)
{
    ASSERT(pActor->objectType() == MachLog::MISSILE_EMPLACEMENT, "Not missile emplacement in armourer::arm\n");
    const MachLogMissileEmplacement& constActor = *pActor;
    const MachPhysMissileEmplacement& physActor = *constActor.pPhysMissileEmplacement();
    MachPhysMissileEmplacement& nonConstPhysActor = (MachPhysMissileEmplacement&)physActor;
    for (int i = 0; i < nonConstPhysActor.nMountedWeapons(); ++i)
    {
        MachPhysWeapon* pPhysWeapon = &nonConstPhysActor.weapon(i);
        pActor->addWeapon(createWeapon((MachLogRace*)&constActor.logRace(), pPhysWeapon, pActor));
    }
}

// static
void MachLogArmourer::rearm(MachLogMissileEmplacement* pActor)
{
    ASSERT(pActor->objectType() == MachLog::MISSILE_EMPLACEMENT, "Not missile emplacement in armourer::rearm\n");
    const MachLogMissileEmplacement& constActor = *pActor;
    const MachPhysMissileEmplacement& physActor = *constActor.pPhysMissileEmplacement();
    MachPhysMissileEmplacement& nonConstPhysActor = (MachPhysMissileEmplacement&)physActor;
    for (int i = 0; i < nonConstPhysActor.nMountedWeapons(); ++i)
    {
        MachPhysWeapon* pPhysWeapon = &nonConstPhysActor.weapon(i);
        pActor->weapons()[i]->setPhysicalWeapon(pPhysWeapon);
        checkAndSetSpecialWeapons(pActor->weapons()[i], pPhysWeapon);
    }
}

// static
void MachLogArmourer::arm(MachLogPod* pActor)
{
    ASSERT(pActor->objectType() == MachLog::POD, "Not pod in armourer::arm\n");
    const MachLogPod& constActor = *pActor;
    const MachPhysPod& physActor = *constActor.pPhysPod();
    MachPhysPod& nonConstPhysActor = (MachPhysPod&)physActor;
    for (int i = 0; i < nonConstPhysActor.nMountedWeapons(); ++i)
    {
        MachPhysWeapon* pPhysWeapon = &nonConstPhysActor.weapon(i);
        pActor->addWeapon(createWeapon((MachLogRace*)&constActor.logRace(), pPhysWeapon, pActor));
    }
}

// static
void MachLogArmourer::rearm(MachLogPod* pActor)
{
    ASSERT(pActor->objectType() == MachLog::POD, "Not pod in armourer::rearm\n");
    const MachLogPod& constActor = *pActor;
    const MachPhysPod& physActor = *constActor.pPhysPod();
    MachPhysPod& nonConstPhysActor = (MachPhysPod&)physActor;
    for (int i = 0; i < nonConstPhysActor.nMountedWeapons(); ++i)
    {
        MachPhysWeapon* pPhysWeapon = &nonConstPhysActor.weapon(i);
        pActor->weapons()[i]->setPhysicalWeapon(pPhysWeapon);
        checkAndSetSpecialWeapons(pActor->weapons()[i], pPhysWeapon);
    }
}

// static
MachLogWeapon* MachLogArmourer::createWeapon(MachLogRace* pLogRace, MachPhysWeapon* pPhysWeapon, MachActor* pActor)
{
    MachLogWeapon* pWeapon = nullptr;
    HAL_STREAM("MLArmourer::createWeapon for type " << pPhysWeapon->type() << std::endl);

    switch (pPhysWeapon->type())
    {
        case MachPhys::PULSE_RIFLE:
        case MachPhys::PULSE_CANNON:
            pWeapon = new MachLogPulseWeapon(pLogRace, _STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::FLAME_THROWER1:
        case MachPhys::FLAME_THROWER2:
            pWeapon = new MachLogFlameThrower(pLogRace, _STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::ELECTRIC_CHARGE:
            pWeapon = new MachLogElectroCharger(pLogRace, _STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::PLASMA_RIFLE:
        case MachPhys::PLASMA_CANNON1:
        case MachPhys::PLASMA_CANNON2:
            pWeapon = new MachLogPlasmaWeapon(pLogRace, _STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::BOLTER:
        case MachPhys::AUTO_CANNON:
        case MachPhys::HEAVY_BOLTER1:
        case MachPhys::HEAVY_BOLTER2:
            pWeapon = new MachLogBolterWeapon(pLogRace, pPhysWeapon, pActor);
            break;

        // case MachPhys::SUPERCHARGE_BASIC:  do not exist any more 3/6/98 Yueai
        case MachPhys::SUPERCHARGE_ADVANCED:
        case MachPhys::SUPERCHARGE_SUPER:
            pWeapon = new MachLogSuperCharger(pLogRace, _STATIC_CAST(MachPhysSuperCharger*, pPhysWeapon), pActor);
            break;

        case MachPhys::GORILLA_PUNCH:
            pWeapon = new MachLogPunchWeapon(pLogRace, _STATIC_CAST(MachPhysPunchWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::VORTEX:
            pWeapon = new MachLogVortexWeapon(pLogRace, pPhysWeapon, pActor);
            break;

        case MachPhys::ION_ORBITAL_CANNON:
            pWeapon = new MachLogIonWeapon(pLogRace, pPhysWeapon, pActor);
            break;

        case MachPhys::ORB_OF_TREACHERY:
            pWeapon = new MachLogTreacheryWeapon(pLogRace, _STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::HOMING_MISSILE:
        case MachPhys::MULTI_LAUNCHER1:
        case MachPhys::MULTI_LAUNCHER2:
        case MachPhys::MULTI_LAUNCHER3:
        case MachPhys::MULTI_LAUNCHER4:
        case MachPhys::MULTI_LAUNCHER5:
        case MachPhys::MULTI_LAUNCHER6:
        case MachPhys::MULTI_LAUNCHER7:
            pWeapon = new MachLogMultiLauncher(pLogRace, _STATIC_CAST(MachPhysMultiLauncher*, pPhysWeapon), pActor);
            break;

        case MachPhys::LARGE_MISSILE:
            pWeapon
                = new MachLogLargeMissileLauncher(pLogRace, _STATIC_CAST(MachPhysLargeMissile*, pPhysWeapon), pActor);
            break;

        case MachPhys::NUCLEAR_MISSILE:
            pWeapon = new MachLogNuclearWeapon(pLogRace, _STATIC_CAST(MachPhysNuclearWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::BEE_BOMB:
            pWeapon = new MachLogBeeBomber(pLogRace, _STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::WASP_LIGHT_STING:
            pWeapon = new MachLogLightStingWeapon(pLogRace, _STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon), pActor);
            break;

        case MachPhys::WASP_METAL_STING:
            pWeapon = new MachLogMetalStingWeapon(pLogRace, _STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon), pActor);
            break;

            DEFAULT_ASSERT_BAD_CASE(pPhysWeapon->type());
    }

    POST(pWeapon != nullptr);
    return pWeapon;
}

// static
void MachLogArmourer::checkAndSetSpecialWeapons(MachLogWeapon* pLogWeapon, MachPhysWeapon* pPhysWeapon)
{
    switch (pPhysWeapon->type())
    {
        case MachPhys::PULSE_RIFLE:
        case MachPhys::PULSE_CANNON:
        case MachPhys::FLAME_THROWER1:
        case MachPhys::FLAME_THROWER2:
        case MachPhys::ELECTRIC_CHARGE:
        case MachPhys::PLASMA_RIFLE:
        case MachPhys::PLASMA_CANNON1:
        case MachPhys::PLASMA_CANNON2:
        case MachPhys::ORB_OF_TREACHERY:
        case MachPhys::NUCLEAR_MISSILE:
        case MachPhys::BEE_BOMB:
        case MachPhys::WASP_LIGHT_STING:
        case MachPhys::WASP_METAL_STING:
            _STATIC_CAST(MachLogLinearWeapon*, pLogWeapon)
                ->setPhysicalLinearWeapon(_STATIC_CAST(MachPhysLinearWeapon*, pPhysWeapon));
            break;

        case MachPhys::SUPERCHARGE_ADVANCED:
        case MachPhys::SUPERCHARGE_SUPER:
            MachLogSuperCharger::asSuperCharger(pLogWeapon)
                .setPhysicalSuperCharger(_STATIC_CAST(MachPhysSuperCharger*, pPhysWeapon));
            break;

        case MachPhys::HOMING_MISSILE:
        case MachPhys::LARGE_MISSILE:
        case MachPhys::MULTI_LAUNCHER1:
        case MachPhys::MULTI_LAUNCHER2:
        case MachPhys::MULTI_LAUNCHER3:
        case MachPhys::MULTI_LAUNCHER4:
        case MachPhys::MULTI_LAUNCHER5:
        case MachPhys::MULTI_LAUNCHER6:
        case MachPhys::MULTI_LAUNCHER7:
            _STATIC_CAST(MachLogMultiLauncher*, pLogWeapon)
                ->setPhysicalMultiLauncher(_STATIC_CAST(MachPhysMultiLauncher*, pPhysWeapon));
            break;

        case MachPhys::GORILLA_PUNCH:
            _STATIC_CAST(MachLogPunchWeapon*, pLogWeapon)
                ->setPhysicalPunchWeapon(_STATIC_CAST(MachPhysPunchWeapon*, pPhysWeapon));
            break;
    }
}

MachLogArmourer::MachLogArmourer()
{

    TEST_INVARIANT;
}

MachLogArmourer::~MachLogArmourer()
{
    TEST_INVARIANT;
}

void MachLogArmourer::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogArmourer& t)
{

    o << "MachLogArmourer " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogArmourer " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End ARMOURER.CPP *************************************************/
