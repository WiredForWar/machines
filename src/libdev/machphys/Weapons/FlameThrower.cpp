/*
 * F L A M E T H R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Weapons/FlameThrower.hpp"
#include "render/Texture.hpp" //must be included  here!!!
#include "machphys/Effects/FlameBall.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machphys/Effects/Effects.hpp"
#include "machphys/Effects/SmokeCloud.hpp"
#include "machphys/Random.hpp"
#include "machphys/Machines/Machine.hpp"
#include "machphys/Constructions/Construction.hpp"
#include "machphys/Effects/Flame.hpp"
#include "machphys/Persistence/WeaponPersistence.hpp"
#include "machphys/Data/SoundData.hpp"
#include "machphys/Weapons/Weapon.hpp"

#include "world4d/Entity/Root.hpp"
#include "world4d/Entity/Generic.hpp"
#include "world4d/Entity/Entity.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/GeneralUniformScalePlan.hpp"
#include "world4d/Scene/GarbageCollector.hpp"
#include "world4d/Sound/SoundManager.hpp"

#include "phys/Plans/LinearScalarPlan.hpp"
#include "phys/Plans/TimedAnglePlan.hpp"
#include "phys/Plans/MotionPlan.hpp"
#include "phys/Plans/LinearMotionPlan.hpp"

#include "render/Mesh.hpp"
#include "render/MeshInstance.hpp"
#include "render/Colour.hpp"
#include "render/TextureManager.hpp"

#include "mathex/Vec2.hpp"
#include "mathex/Line3d.hpp"
#include "mathex/Point3d.hpp"
#include "mathex/Vec3.hpp"

#include "system/PathName.hpp"

PER_DEFINE_PERSISTENT(MachPhysFlameThrower);

MachPhysFlameThrower::MachPhysFlameThrower(
    W4dEntity* pParent,
    const MexTransform3d& localTransform,
    MachPhys::WeaponType type,
    MachPhys::Mounting mounting)
    : MachPhysLinearWeapon(exemplar(type), mounting, pParent, localTransform)
{

    TEST_INVARIANT;
}

// One-time ctor
MachPhysFlameThrower::MachPhysFlameThrower(MachPhys::WeaponType type)
    : MachPhysLinearWeapon(
        MachPhysWeaponPersistence::instance().pRoot(),
        MexTransform3d(),
        SysPathName(compositeFilePath(type)),
        type,
        MachPhys::LEFT)
{
    TEST_INVARIANT;
}

MachPhysFlameThrower::MachPhysFlameThrower(PerConstructor con)
    : MachPhysLinearWeapon(con)
{
}

MachPhysFlameThrower::~MachPhysFlameThrower()
{
    TEST_INVARIANT;
}

// static
const MachPhysFlameThrower& MachPhysFlameThrower::exemplar(MachPhys::WeaponType type)
{
    return MachPhysWeaponPersistence::instance().flameThrowerExemplar(type);
}

void MachPhysFlameThrower::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysFlameThrower& t)
{

    o << "MachPhysFlameThrower " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysFlameThrower " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// static
const char* MachPhysFlameThrower::compositeFilePath(MachPhys::WeaponType type)
{
    switch (type)
    {
        case MachPhys::FLAME_THROWER1:
            return "models/weapons/flameth1/flameth1.cdf";
        case MachPhys::FLAME_THROWER2:
            return "models/weapons/flameth2/flameth2.cdf";
    }
    return "";
}
/*
MachPhysFlameBall* MachPhysFlameThrower::createFlameBall(const PhysAbsoluteTime& burstTime,
                                   uint flameIndex,
                                   W4dEntity* pParent,
                                   const W4dEntity& target,
                                   const MexPoint3d& targetOffset )
{
    MexTransform3d startTransform;
    MATHEX_SCALAR distance;

    PhysAbsoluteTime launchTime = launchData(burstTime, flameIndex, pParent, target, targetOffset, &startTransform,
&distance);

    //Compute end position
    MexTransform3d relativeMotion( MexPoint3d( weaponData().range(), 0.0, 0.0 ) );
    MexTransform3d endTransform;
    startTransform.transform( relativeMotion, &endTransform );

    const uint nFlames = weaponData().nRoundsPerBurst();

    PhysRelativeTime waitingTime[5];
    PhysRelativeTime truncateTime[5];
    MATHEX_SCALAR PropotionEnd[5];
    MATHEX_SCALAR propotionMiddle[5];
    MATHEX_SCALAR pathDip[5];
    for(int i=0; i<nFlames; ++i)
    {
        waitingTime[i] = weaponData().extras()[i];
        truncateTime[i] = weaponData().extras()[nFlames+i];
        PropotionEnd[i] = weaponData().extras()[2*nFlames+i];
        propotionMiddle[i] = weaponData().extras()[3*nFlames+i];
        pathDip[i] = weaponData().extras()[4*nFlames+i];
    }

    MATHEX_SCALAR flameBallSize = weaponData().extras()[5*nFlames];
    MachPhysFlameBall* pFlameBall = new MachPhysFlameBall(pParent, MexTransform3d(), flameBallSize ) ;

    pFlameBall->move(startTransform, endTransform,
                     launchTime, waitingTime[flameIndex], truncateTime[flameIndex],
                     pathDip[flameIndex], PropotionEnd[flameIndex], propotionMiddle[flameIndex],flameBallSize
);

    return pFlameBall;
 }
*/

// virtual
MachPhysLinearProjectile* MachPhysFlameThrower::createProjectile(
    const PhysAbsoluteTime& burstStartTime,
    uint flameIndex,
    W4dEntity* pParent,
    const W4dEntity& target,
    const MexPoint3d& targetOffset)
{
    MachPhysFlameBall* pFlameBall = createFlameBall(burstStartTime, flameIndex, pParent, target, targetOffset);
    MachPhysLinearProjectile* pProjectile = pFlameBall;
    return pProjectile;
}

// virtual
PhysRelativeTime MachPhysFlameThrower::fire(const PhysAbsoluteTime& startTime, int)
{
    // get weapon data
    const MachPhysWeaponData& data = weaponData();
    MexPoint3d smokeOffset = data.launchOffsets()[0];
    MATHEX_SCALAR wispSize = 1.5;
    MexPoint3d endPosition(0.0, 0.0, 2.0);
    MexVec3 randomOffset(0.1, 0.1, 0.0);

    uint nRounds = data.nRoundsPerBurst();
    PhysRelativeTime burstDuration = data.burstDuration();
    PhysRelativeTime recoilBackTime = data.recoilBackTime();

    // Set up timing
    PhysAbsoluteTime roundTime = burstDuration / nRounds;
    PhysRelativeTime endTime = startTime + burstDuration;

    // Add the exhaust smoke cloud
    MachPhysSmokeCloud* pSmokeCloud = new MachPhysSmokeCloud(this, smokeOffset);
    uint nWisps = 3;
    PhysRelativeTime wispInterval = 0.08;
    MATHEX_SCALAR zDepthOffset = 0.0;
    PhysRelativeTime smokeDuration = pSmokeCloud->startSmoking(
        nWisps,
        endPosition,
        randomOffset,
        startTime,
        wispInterval,
        wispSize,
        zDepthOffset,
        MachPhysEffects::smokeMaterialPlan(MachPhysEffects::GREY));

    // Garbage collect the  exhaust
    W4dGarbageCollector::instance().add(pSmokeCloud, endTime + smokeDuration);

    // lighting up the machine and the weapon
    lighting(RenColour(1., 1., 165.0 / 255.0), startTime, 5);

    // Play sound
    W4dSoundManager::instance().play(this, SID_FLAMETHROWER, startTime, 1);
    //    W4dSoundManager::instance().play( this, SysPathName( "sounds/flamethr.wav" ), startTime,
    //                                      100.0, 100.0, W4dSoundManager::PLAY_ONCE );

    return burstDuration;
}

MachPhysFlameBall* MachPhysFlameThrower::createFlameBall(
    const PhysAbsoluteTime& burstTime,
    uint flameIndex,
    W4dEntity* pParent,
    const W4dEntity& target,
    const MexPoint3d& targetOffset)
{
    // Compute the basic path info using standard helper function
    MexTransform3d startTransform;
    MATHEX_SCALAR distance;
    PhysAbsoluteTime launchTime
        = launchData(burstTime, flameIndex, pParent, target, targetOffset, &startTransform, &distance);

    // Get relevant info from data object
    const MachPhysWeaponData& data = weaponData();
    const uint nFlames = weaponData().nRoundsPerBurst();
    PhysRelativeTime burstDuration = data.burstDuration();
    MATHEX_SCALAR range = weaponData().range();

    // Extras data per flameball as follows:
    // Time offset from start of burst to launch of the flameball
    // Additional time delay before starts cycling through material plan
    // Distance adjustment from standard range (in x direction)
    // Vertical position adjustment (in z direction)
    // Final scale for the flameball
    const MachPhysWeaponData::Extras& extras = data.extras();
    uint i = flameIndex * 5;
    PhysRelativeTime startTimeOffset = extras[i++];
    PhysRelativeTime materialPlanCycleStartDelay = extras[i++];
    MATHEX_SCALAR xOffset = extras[i++];
    MATHEX_SCALAR zOffset = extras[i++];
    MATHEX_SCALAR endScale = extras[i++];

    // Recompute the actual start time for the flameball. This is supplied via the extras data
    // as an offset from the burst start time.
    launchTime = burstTime + startTimeOffset;
    MATHEX_SCALAR flightDistance = range + xOffset;

    // Compute the end position from the basic range, x offset and vertical displacement
    MexTransform3d relativeMotion(MexPoint3d(flightDistance, 0.0, zOffset));
    MexTransform3d endTransform;
    startTransform.transform(relativeMotion, &endTransform);

    /*
    //decide the flame ball type
    MachPhysFlameBall::FlameBallType flameBallType;

    if( type() == MachPhys::FLAME_THROWER2 )
    {
        flameBallType = MachPhysFlameBall::GRUNT_FLAME_BALL;
    }
    else if ( type() == MachPhys::FLAME_THROWER1 )
    {
        flameBallType = MachPhysFlameBall::TURRET_FLAME_BALL;
    }
    else
    {
        ASSERT( true, " flame ball type not defined." );
    }
*/
    MachPhysFlameBall* pFlameBall
        = new MachPhysFlameBall(pParent, startTransform, MachPhysFlameBall::GRUNT_FLAME_BALL);

    // Make it fly
    pFlameBall->move(launchTime, startTransform, endTransform, materialPlanCycleStartDelay, endScale);

    return pFlameBall;
}

// virtual
PhysRelativeTime MachPhysFlameThrower::victimAnimation(
    const PhysAbsoluteTime& startTime,
    const MexLine3d& fromDirection,
    MachPhysMachine* pMachine) const
{
    return applyVictimAnimation(startTime, fromDirection, pMachine);
}

// virtual
PhysRelativeTime MachPhysFlameThrower::victimAnimation(
    const PhysAbsoluteTime& startTime,
    const MexLine3d& fromDirection,
    MachPhysConstruction* pConstruction) const
{
    return applyVictimAnimation(startTime, fromDirection, pConstruction);
}

// static
PhysRelativeTime MachPhysFlameThrower::applyVictimAnimation(
    const PhysAbsoluteTime& startTime,
    const MexLine3d&,
    MachPhysMachine* pMachine)
{
    // Set fire to the links for a period of time
    PhysRelativeTime duration = 15.0;
    pMachine->burn(startTime, duration, 100.0);

    return duration;
}

// static
PhysRelativeTime MachPhysFlameThrower::applyVictimAnimation(
    const PhysAbsoluteTime& startTime,
    const MexLine3d& fromDirection,
    MachPhysConstruction* pConstruction)
{
    return 0.0;
}

void perWrite(PerOstream& ostr, const MachPhysFlameThrower& weapon)
{
    const MachPhysLinearWeapon& base = weapon;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysFlameThrower& weapon)
{
    MachPhysLinearWeapon& base = weapon;

    istr >> base;
}

// static
PhysRelativeTime MachPhysFlameThrower::applyVictimAnimation(
    const PhysAbsoluteTime& startTime,
    const MexLine3d& fromDirection,
    MachPhysArtefact* pArtefact)
{
    return 0.0;
}

/* End FLAMETHR.CPP ****************************************************/
