/*
 * P E R S I S T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Persistence/Persistence.hpp"

#include "mathex/Transform3d.hpp"
#include "phys/Plans/RampAcceleration.hpp"
#include "phys/Persistence.hpp"

#include "machphys/Effects/VapourPuff.hpp"

#include "machphys/Machines/Administrator.hpp"
#include "machphys/Machines/Aggressor.hpp"
#include "machphys/Machines/Constructor.hpp"
#include "machphys/Machines/Technician.hpp"
#include "machphys/Machines/ResourceCarrier.hpp"
#include "machphys/Machines/GeoLocator.hpp"
#include "machphys/Machines/SpyLocator.hpp"
#include "machphys/Machines/APC.hpp"

#include "machphys/Constructions/Pod.hpp"
#include "machphys/Constructions/Beacon.hpp"
#include "machphys/Constructions/Factory.hpp"
#include "machphys/Constructions/Garrison.hpp"
#include "machphys/Constructions/HardwareLab.hpp"
#include "machphys/Constructions/Mine.hpp"
#include "machphys/Constructions/MissileEmplacement.hpp"
#include "machphys/Constructions/Smelter.hpp"

#include "machphys/Effects/SmokePlume.hpp"
#include "machphys/Effects/SmokePuff.hpp"
#include "machphys/OreHolograph.hpp"
#include "machphys/Weapons/Weapon.hpp"
#include "machphys/Weapons/LinearProjectile.hpp"

#include "machphys/Locomotion/Wheels.hpp"
#include "machphys/Locomotion/SpiderLegs.hpp"
#include "machphys/Locomotion/Tracks.hpp"
#include "machphys/Locomotion/HoverBoots.hpp"
#include "machphys/Locomotion/Glider.hpp"

#include "machphys/Locomotion/Internal/WheelsImpl.hpp"
#include "machphys/Locomotion/Internal/SpiderLegsImpl.hpp"
#include "machphys/Locomotion/Internal/TracksImpl.hpp"
#include "machphys/Locomotion/Internal/HoverBootsImpl.hpp"
#include "machphys/Locomotion/Internal/GliderImpl.hpp"

#include "machphys/Effects/PulseAura.hpp"
#include "machphys/Effects/PulseSplat.hpp"
#include "machphys/Effects/PlasmaAura.hpp"
#include "machphys/Effects/PlasmaSplat.hpp"
#include "machphys/Weapons/PlasmaWeapon.hpp"
#include "machphys/Weapons/PlasmaBolt.hpp"
#include "machphys/Weapons/Electro.hpp"
#include "machphys/Effects/Flame.hpp"
#include "machphys/Effects/STFFlame.hpp"
#include "machphys/Effects/FlameBall.hpp"
#include "machphys/Weapons/PulseBlob.hpp"
#include "machphys/Weapons/PulseWeapon.hpp"
#include "machphys/Weapons/TreacheryOrb.hpp"
#include "machphys/Effects/Fireball.hpp"
#include "machphys/Effects/RadialDisc.hpp"
#include "machphys/Weapons/FlameThrower.hpp"
#include "machphys/Weapons/ElectroCharger.hpp"
#include "machphys/Weapons/VirusSpreader.hpp"
#include "machphys/Weapons/MultiLauncher.hpp"
#include "machphys/Weapons/TreacheryWeapon.hpp"
#include "machphys/Weapons/Bolter.hpp"

#include "machphys/Effects/Beam.hpp"
#include "machphys/Effects/BlackSphere.hpp"
#include "machphys/Effects/Debris.hpp"
#include "machphys/Effects/DebrisUnit.hpp"
#include "machphys/Effects/FlashDisc.hpp"
#include "machphys/Effects/LightningHalo.hpp"
#include "machphys/Effects/HealAura.hpp"
#include "machphys/Effects/HealHelix.hpp"
#include "machphys/Effects/Helix.hpp"
#include "machphys/Effects/IonBeam.hpp"
#include "machphys/LandMine.hpp"
#include "machphys/Effects/Particles.hpp"
#include "machphys/Effects/RadialDisc.hpp"
#include "machphys/Effects/SmokeCloud.hpp"
#include "machphys/Terrain/TerrainTile.hpp"
#include "machphys/Effects/VortexBomb.hpp"
#include "machphys/Effects/WhiteSphere.hpp"
#include "machphys/Effects/VapourPuff.hpp"
#include "machphys/Effects/VortexSphere.hpp"

#include "machphys/Weapons/Missile.hpp"
#include "machphys/Weapons/LargeMissile.hpp"
#include "machphys/Weapons/IonWeapon.hpp"
#include "machphys/Weapons/NuclearWeapon.hpp"
#include "machphys/Weapons/VortexWeapon.hpp"
#include "machphys/Weapons/SuperCharger.hpp"

#include "machphys/Effects/NuclearBomb.hpp"
#include "machphys/Effects/MushroomTop.hpp"
#include "machphys/Effects/MushroomEdge.hpp"
#include "machphys/Effects/MushroomShaft.hpp"
#include "machphys/Effects/GroundSplat.hpp"
#include "machphys/Effects/ShockWave.hpp"
#include "machphys/Effects/BurstWave.hpp"
#include "machphys/Effects/NukeWave.hpp"

#include "machphys/Effects/JetRing.hpp"
#include "machphys/Effects/HemiSphere.hpp"
#include "machphys/Effects/Sparks.hpp"
#include "machphys/Marker.hpp"

#include "machphys/Effects/DoublesidedFlame.hpp"
#include "machphys/Effects/Waterfall.hpp"

#include "machphys/Weapons/PunchWeapon.hpp"
#include "machphys/Effects/PunchBlast.hpp"
#include "machphys/Effects/Blast.hpp"
#include "machphys/Effects/CrackFire.hpp"
#include "machphys/Effects/Light.hpp"

#include "machphys/Effects/ObjDemolish.hpp"
#include "machphys/Machines/FacePlate.hpp"
#include "machphys/Weapons/BeeBomber.hpp"
#include "machphys/Weapons/BeeBomb.hpp"
#include "machphys/Effects/BeeBombWave.hpp"

#include "machphys/Weapons/LightSting.hpp"
#include "machphys/Weapons/MetalSting.hpp"
#include "machphys/Weapons/LightStingWeapon.hpp"
#include "machphys/Weapons/MetalStingWeapon.hpp"
#include "machphys/Effects/LightStingAura.hpp"
#include "machphys/Effects/LightStingSplat.hpp"

#include "machphys/Effects/ResourceLoading.hpp"
#include "machphys/Effects/LoadingLight.hpp"
#include "machphys/Effects/LoadingUnder.hpp"
#include "machphys/Effects/LoadingOver.hpp"
#include "machphys/Effects/Scavenger.hpp"
#include "machphys/Locator.hpp"

#include "machphys/Persistence/MachinePersistence.hpp"
#include "machphys/Persistence/ConstructionPersistence.hpp"
#include "machphys/Persistence/WeaponPersistence.hpp"
#include "machphys/Persistence/OtherPersistence.hpp"

#include "world4d/Persistence.hpp"

PER_DEFINE_PERSISTENT(MachPhysPersistence);

// static
MachPhysPersistence& MachPhysPersistence::instance()
{
    static MachPhysPersistence instance_;
    return instance_;
}

MachPhysPersistence::MachPhysPersistence()
{
    W4dPersistence::instance();
    PhysPersistence::instance();

    PER_REGISTER_DERIVED_CLASS(MachPhysAggressor);
    PER_REGISTER_DERIVED_CLASS(MachPhysGeoLocator);
    PER_REGISTER_DERIVED_CLASS(MachPhysSpyLocator);
    PER_REGISTER_DERIVED_CLASS(MachPhysResourceCarrier);
    PER_REGISTER_DERIVED_CLASS(MachPhysAPC);
    PER_REGISTER_DERIVED_CLASS(MachPhysTechnician);
    PER_REGISTER_DERIVED_CLASS(MachPhysConstructor);
    PER_REGISTER_DERIVED_CLASS(MachPhysAdministrator);
    PER_REGISTER_DERIVED_CLASS(MachPhysBeacon);
    PER_REGISTER_DERIVED_CLASS(MachPhysFactory);
    PER_REGISTER_DERIVED_CLASS(MachPhysGarrison);
    PER_REGISTER_DERIVED_CLASS(MachPhysMine);
    PER_REGISTER_DERIVED_CLASS(MachPhysMissileEmplacement);
    PER_REGISTER_DERIVED_CLASS(MachPhysPod);
    PER_REGISTER_DERIVED_CLASS(MachPhysSmelter);
    PER_REGISTER_DERIVED_CLASS(MachPhysMissile);
    PER_REGISTER_DERIVED_CLASS(MachPhysFacePlate);
    PER_REGISTER_DERIVED_CLASS(MachPhysHardwareLab);

    PER_REGISTER_DERIVED_CLASS(MachPhysSmokePlume);
    PER_REGISTER_DERIVED_CLASS(MachPhysSmokePuff);
    PER_REGISTER_DERIVED_CLASS(MachPhysOreHolograph);
    PER_REGISTER_DERIVED_CLASS(MachPhysWeapon);

    PER_REGISTER_DERIVED_CLASS(MachPhysWheels);
    PER_REGISTER_DERIVED_CLASS(MachPhysHoverBoots);
    PER_REGISTER_DERIVED_CLASS(MachPhysSpiderLegs);
    PER_REGISTER_DERIVED_CLASS(MachPhysTracks);
    PER_REGISTER_DERIVED_CLASS(MachPhysGlider);

    PER_REGISTER_DERIVED_CLASS(MachPhysWheelsImpl);
    PER_REGISTER_DERIVED_CLASS(MachPhysHoverBootsImpl);
    PER_REGISTER_DERIVED_CLASS(MachPhysSpiderLegsImpl);
    PER_REGISTER_DERIVED_CLASS(MachPhysTracksImpl);
    PER_REGISTER_DERIVED_CLASS(MachPhysGliderImpl);

    PER_REGISTER_DERIVED_CLASS(MachPhysLinearWeapon);
    PER_REGISTER_DERIVED_CLASS(MachPhysLinearProjectile);
    PER_REGISTER_DERIVED_CLASS(MachPhysWeapon);

    PER_REGISTER_DERIVED_CLASS(MachPhysPlasmaWeapon);
    PER_REGISTER_DERIVED_CLASS(MachPhysPlasmaBolt);
    PER_REGISTER_DERIVED_CLASS(MachPhysElectro);
    PER_REGISTER_DERIVED_CLASS(MachPhysFlameBall);
    PER_REGISTER_DERIVED_CLASS(MachPhysPulseBlob);
    PER_REGISTER_DERIVED_CLASS(MachPhysTreacheryOrb);
    PER_REGISTER_DERIVED_CLASS(MachPhysFireball);
    PER_REGISTER_DERIVED_CLASS(MachPhysRadialDisc);
    PER_REGISTER_DERIVED_CLASS(MachPhysPlasmaSplat);
    PER_REGISTER_DERIVED_CLASS(MachPhysPlasmaAura);
    PER_REGISTER_DERIVED_CLASS(MachPhysPulseSplat);
    PER_REGISTER_DERIVED_CLASS(MachPhysPulseAura);

    PER_REGISTER_DERIVED_CLASS(MachPhysElectroCharger);
    PER_REGISTER_DERIVED_CLASS(MachPhysBolter);
    PER_REGISTER_DERIVED_CLASS(MachPhysFlameThrower);
    PER_REGISTER_DERIVED_CLASS(MachPhysMultiLauncher);
    PER_REGISTER_DERIVED_CLASS(MachPhysPulseWeapon);
    PER_REGISTER_DERIVED_CLASS(MachPhysPulseRifle);
    PER_REGISTER_DERIVED_CLASS(MachPhysPulseCannon);
    PER_REGISTER_DERIVED_CLASS(MachPhysVirusSpreader);
    PER_REGISTER_DERIVED_CLASS(MachPhysTreacheryWeapon);
    PER_REGISTER_DERIVED_CLASS(MachPhysLargeMissile);
    PER_REGISTER_DERIVED_CLASS(MachPhysIonWeapon);
    PER_REGISTER_DERIVED_CLASS(MachPhysVortexWeapon);
    PER_REGISTER_DERIVED_CLASS(MachPhysNuclearWeapon);
    PER_REGISTER_DERIVED_CLASS(MachPhysSuperCharger);
    PER_REGISTER_DERIVED_CLASS(MachPhysPunchWeapon);

    PER_REGISTER_DERIVED_CLASS(MachPhysLight);
    PER_REGISTER_DERIVED_CLASS(MachPhysWhiteSphere);
    PER_REGISTER_DERIVED_CLASS(MachPhysVortexSphere);
    PER_REGISTER_DERIVED_CLASS(MachPhysVortexBomb);
    PER_REGISTER_DERIVED_CLASS(MachPhysTerrainTile);
    PER_REGISTER_DERIVED_CLASS(MachPhysSmokeCloud);
    PER_REGISTER_DERIVED_CLASS(MachPhysRadialDisc);
    PER_REGISTER_DERIVED_CLASS(MachPhysParticles);
    PER_REGISTER_DERIVED_CLASS(MachPhysLandMine);
    PER_REGISTER_DERIVED_CLASS(MachPhysIonBeam);
    PER_REGISTER_DERIVED_CLASS(MachPhysHealHelix);
    PER_REGISTER_DERIVED_CLASS(MachPhysHelix);
    PER_REGISTER_DERIVED_CLASS(MachPhysHealAura);
    PER_REGISTER_DERIVED_CLASS(MachPhysFlashDisc);
    PER_REGISTER_DERIVED_CLASS(MachPhysDebrisUnit);
    PER_REGISTER_DERIVED_CLASS(MachPhysDebris);
    PER_REGISTER_DERIVED_CLASS(MachPhysBeam);
    PER_REGISTER_DERIVED_CLASS(MachPhysBlackSphere);
    PER_REGISTER_DERIVED_CLASS(MachPhysLightningHalo);
    PER_REGISTER_DERIVED_CLASS(MachPhysVapourPuff);

    PER_REGISTER_DERIVED_CLASS(MachPhysNuclearBomb);
    PER_REGISTER_DERIVED_CLASS(MachPhysMushroomTop);
    PER_REGISTER_DERIVED_CLASS(MachPhysMushroomEdge);
    PER_REGISTER_DERIVED_CLASS(MachPhysMushroomShaft);
    PER_REGISTER_DERIVED_CLASS(MachPhysGroundSplat);
    PER_REGISTER_DERIVED_CLASS(MachPhysShockWave);
    PER_REGISTER_DERIVED_CLASS(MachPhysBurstWave);
    PER_REGISTER_DERIVED_CLASS(MachPhysNukeWave);

    PER_REGISTER_DERIVED_CLASS(MachPhysHemiSphere);
    PER_REGISTER_DERIVED_CLASS(MachPhysJetRing);
    PER_REGISTER_DERIVED_CLASS(MachPhysSparks);

    PER_REGISTER_DERIVED_CLASS(MachPhysPunchBlast);
    PER_REGISTER_DERIVED_CLASS(MachPhysBlast);
    PER_REGISTER_DERIVED_CLASS(MachPhysCrackFire);

    PER_REGISTER_DERIVED_CLASS(MachPhysDoublesidedFlame);
    PER_REGISTER_DERIVED_CLASS(MachPhysFlame);
    PER_REGISTER_DERIVED_CLASS(MachPhysSTFFlame);
    PER_REGISTER_DERIVED_CLASS(MachPhysWaterfall);
    PER_REGISTER_DERIVED_CLASS(MachPhysMarker);

    PER_REGISTER_DERIVED_CLASS(MachPhysObjDemolish);

    PER_REGISTER_DERIVED_CLASS(MachPhysBeeBomber);
    PER_REGISTER_DERIVED_CLASS(MachPhysBeeBomb);
    PER_REGISTER_DERIVED_CLASS(MachPhysBeeBombWave);

    PER_REGISTER_DERIVED_CLASS(MachPhysLightSting);
    PER_REGISTER_DERIVED_CLASS(MachPhysLightStingAura);
    PER_REGISTER_DERIVED_CLASS(MachPhysLightStingSplat);
    PER_REGISTER_DERIVED_CLASS(MachPhysLightStingWeapon);

    PER_REGISTER_DERIVED_CLASS(MachPhysMetalSting);
    PER_REGISTER_DERIVED_CLASS(MachPhysMetalStingWeapon);

    PER_REGISTER_DERIVED_CLASS(MachPhysResourceLoading);
    PER_REGISTER_DERIVED_CLASS(MachPhysLoadingLight);
    PER_REGISTER_DERIVED_CLASS(MachPhysLoadingUnder);
    PER_REGISTER_DERIVED_CLASS(MachPhysLoadingOver);
    PER_REGISTER_DERIVED_CLASS(MachPhysScavenger);
    PER_REGISTER_DERIVED_CLASS(MachPhysLocator);

    TEST_INVARIANT;
}

MachPhysPersistence::~MachPhysPersistence()
{
    TEST_INVARIANT;
}

const MachPhysMachinePersistence& MachPhysPersistence::machines() const
{
    return MachPhysMachinePersistence::instance();
}

const MachPhysConstructionPersistence& MachPhysPersistence::constructions() const
{
    return MachPhysConstructionPersistence::instance();
}

const MachPhysWeaponPersistence& MachPhysPersistence::weapons() const
{
    return MachPhysWeaponPersistence::instance();
}

const MachPhysOtherPersistence& MachPhysPersistence::others() const
{
    return MachPhysOtherPersistence::instance();
}

MachPhysOtherPersistence& MachPhysPersistence::others()
{
    return MachPhysOtherPersistence::instance();
}

MachPhysConstructionPersistence& MachPhysPersistence::constructions()
{
    return MachPhysConstructionPersistence::instance();
}

MachPhysWeaponPersistence& MachPhysPersistence::weapons()
{
    return MachPhysWeaponPersistence::instance();
}

MachPhysMachinePersistence& MachPhysPersistence::machines()
{
    return MachPhysMachinePersistence::instance();
}

void MachPhysPersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysPersistence& t)
{

    o << "MachPhysPersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysPersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachPhysPersistence& per)
{
    // PER_LOG_WRITE( true );

    ostr << per.machines();
    ostr << per.constructions();
    ostr << per.weapons();
    ostr << per.others();
}

void perRead(PerIstream& istr, MachPhysPersistence& per)
{
    // PER_LOG_READ( true );

    istr >> per.machines();
    istr >> per.constructions();
    istr >> per.weapons();
    istr >> per.others();
}

/* End PERSIST.CPP **************************************************/
