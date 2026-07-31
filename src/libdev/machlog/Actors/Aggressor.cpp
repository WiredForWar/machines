/*
 * A G G R E S S R . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "ctl/Algorithm.hpp"
#include "ctl/List.hpp"

#include "mathex/Transform3d.hpp"
#include "mathex/Polygon2d.hpp"
#include "phys/Plans/MotionChunk.hpp"
#include "world4d/Scene/Domain.hpp"

#include "machphys/Machines/Aggressor.hpp"
#include "machphys/Terrain/PlanetSurface.hpp"

#include "machlog/Actors/Aggressor.hpp"
#include "machphys/Machines/AggressorData.hpp"
#include "machlog/Actors/CanAttack.hpp"
#include "machlog/World/PlanetDomains.hpp"
#include "machlog/Race.hpp"
#include "machlog/Races.hpp"
#include "machlog/Actors/MotionSequencer.hpp"
#include "machlog/World/SpacialManipulation.hpp"
#include "machlog/Combat/Weapon.hpp"
#include "machlog/Combat/Armourer.hpp"
#include "machlog/Actors/Construction.hpp"

PER_DEFINE_PERSISTENT(MachLogAggressor)

/* //////////////////////////////////////////////////////////////// */

MachLogAggressor::MachLogAggressor(
    const MachPhys::AggressorSubType& subType,
    Level hwLevel,
    Level swLevel,
    MachLogRace* pRace,
    const MexPoint3d& location,
    MachPhys::WeaponCombo wc,
    std::optional<UtlId> withId)
    : MachLogMachine(
        MachPhys::AGGRESSOR,
        hwLevel,
        swLevel,
        pRace,
        pNewPhysAggressor(subType, hwLevel, swLevel, pRace, location, wc),
        MachLog::AGGRESSOR,
        withId)
    , MachLogCanAttack(this, &physAggressor(), wc)
    , subType_(subType)
{
    // HAL_STREAM("MLAggressor::CTOR static_cast<const void*>(this) " << static_cast<const void*>(this) << std::endl );
    currentTarget(nullptr);
    //  objectType( MachLog::AGGRESSOR );
    hp(data().hitPoints());
    armour(data().armour());
    MachLogRaces::instance().aggressors(pRace->race()).push_back(this);
    MachLogArmourer::arm(this);
}

/* //////////////////////////////////////////////////////////////// */

PhysRelativeTime MachLogAggressor::update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR junk)
{
    HAL_STREAM("(" << id() << ") MachLogAggressor::update\n");

    MachLogCanAttack::update();

    if (!(isDead() || isDying() || evading() || insideAPC()) && ! MachLogRaces::instance().inSpecialActorUpdate()
        && willCheckForTargets())
        checkAndAttackCloserTarget(this);

    diminishAlertnessAndInaccuracy();

    // if moving, maintain medium alert
    if (motionSeq().hasDestination())
    {
        setMinimumAlertness(45);
    }

    // Do the main work
    // PhysRelativeTime alertnessResponse = ( 4.0 - ( const_cast< PhysRelativeTime>(std::min( 100, alertness() ) )
    // / 30.0 ) );
    PhysRelativeTime alertnessResponse = (4.0 - (std::min(100, alertness()) / 30.0));
    PhysRelativeTime result = std::min(MachLogMachine::update(maxCPUTime, junk), alertnessResponse);

    HAL_STREAM("(" << id() << ") MachLogAggressor::update DONE\n");
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachLogAggressor::beHit(
    const int& damage,
    MachPhys::WeaponType byType,
    MachActor* pByActor,
    MexLine3d* pByDirection,
    MachActor::EchoBeHit echo)
{
    int hpBeforeImpact = hp();
    MachLogMachine::beHit(damage, byType, pByActor, pByDirection, echo);
    int hpAfterImpact = hp();

    MATHEX_SCALAR dazing = ((MATHEX_SCALAR)(hpBeforeImpact - hpAfterImpact) / maximumhp()) * 40;
    MachLogCanAttack::increaseCurrentInaccuracy(dazing);

    if (pByActor && allowedToCheckForNewTarget() && ! isDead() && ! evading() && ! isIn1stPersonView()
        && canFireAt(*pByActor) && willTestHitByCandidate(*pByActor))
    {
        MachLogRaces::DispositionToRace disposition
            = MachLogRaces::instance().dispositionToRace(race(), pByActor->race());

        // only take a pop at this schmo if we don't think it's a friend (same race or ally),
        if (disposition == MachLogRaces::NEUTRAL || disposition == MachLogRaces::ENEMY)
        {
            // will try to initiate a counterattack against the actor who hit me.
            checkAndAttackCloserTarget(this, pByActor);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachLogAggressor::beHitWithoutAnimation(
    int damage,
    PhysRelativeTime physicalTimeDelay,
    MachActor* pByActor,
    MachActor::EchoBeHit echo)
{
    int hpBeforeImpact = hp();
    MachLogMachine::beHitWithoutAnimation(damage, physicalTimeDelay, pByActor, echo);
    int hpAfterImpact = hp();

    MATHEX_SCALAR dazing = ((MATHEX_SCALAR)(hpBeforeImpact - hpAfterImpact) / maximumhp()) / 4.0;
    MachLogCanAttack::increaseCurrentInaccuracy(dazing);

    if (pByActor && allowedToCheckForNewTarget() && ! isDead() && ! evading() && ! isIn1stPersonView()
        && canFireAt(*pByActor) && willTestHitByCandidate(*pByActor))
    {
        MachLogRaces::DispositionToRace disposition
            = MachLogRaces::instance().dispositionToRace(race(), pByActor->race());

        // only take a pop at this schmo if we don't think it's a friend (same race or ally),
        if (disposition == MachLogRaces::NEUTRAL || disposition == MachLogRaces::ENEMY)
        {
            // will try to initiate a counterattack against the actor who hit me.
            checkAndAttackCloserTarget(this, pByActor);
        }
    }
}

/* //////////////////////////////////////////////////////////////// */

/* //////////////////////////////////////////////////////////////// */

/*
void MachLogAggressor::doActOnClassification()
{
    if( subjects().empty() )
    {
        //do nothing
    }
    else
    {
        // respond to the most important stimulus
        MachClassification c = *min_element( subjects().begin(), subjects().end() );
        MachLogMachine * pTarget = MachLogMachines::instance()[ c.subjectId() ];

        strategy().newOperation( new MachAttackOperation( this, pTarget ) );
    }
}
*/

/* //////////////////////////////////////////////////////////////// */

MachPhysAggressor& MachLogAggressor::physAggressor()
{
    return static_cast<MachPhysAggressor&>(physObject());
}

const MachPhysAggressor& MachLogAggressor::physAggressor() const
{
    return static_cast<const MachPhysAggressor&>(physObject());
}

/* //////////////////////////////////////////////////////////////// */

// static
MachPhysAggressor* MachLogAggressor::pNewPhysAggressor(
    const MachPhys::AggressorSubType& subType,
    Level hwLevel,
    Level swLevel,
    MachLogRace* pRace,
    const MexPoint3d& location,
    MachPhys::WeaponCombo wc)
{
    // get domain and transform to use
    MexTransform3d localTransform;
    W4dDomain* pDomain = MachLogPlanetDomains::pDomainPosition(location, 0, &localTransform);

    // Construct the physical machine
    return new MachPhysAggressor(pDomain, localTransform, subType, hwLevel, swLevel, pRace->race(), wc);
}
// virtual
const MachPhysMachineData& MachLogAggressor::machineData() const
{
    return data();
}

const MachPhysAggressorData& MachLogAggressor::data() const
{
    // return static_cast< MachPhysAggressorData>(physMachine().machineData() );
    return static_cast<const MachPhysAggressorData&>(physMachine().machineData());
    //  return physMachine().machinedata();
}

const MachPhys::AggressorSubType& MachLogAggressor::subType() const
{
    return subType_;
}

// virtual
bool MachLogAggressor::fearsThisActor(const MachActor& otherActor) const
{
    ASSERT(otherActor.objectIsCanAttack(), "Shouldn't be testing for fear on an actor that isn't attack-capable.");

    // automatically brick myself if he can attack me but I can't attack him,
    // otherwise, only turn tail and run if the other actor is at least 200% stronger than me and my nearby mates
    // put together

    return (
        otherActor.asCanAttack().canFireAt(*this) && !(canFireAt(otherActor))
        || otherActor.militaryStrength() > (localStrength() * 2));
}

// virtual
int MachLogAggressor::militaryStrength() const
{
    return static_cast<int>(hpRatio() * static_cast<MATHEX_SCALAR>(objectData().cost()));
}

/////////////////////////////////////////////////// persistence /////////////////////////////////////////////////////

void perWrite(PerOstream& ostr, const MachLogAggressor& actor)
{
    const MachLogMachine& base1 = actor;
    const MachLogCanAttack& base2 = actor;

    ostr << base1;
    ostr << base2;

    ostr << actor.subType_;

    MexTransform3d trans = actor.globalTransform();

    // If we don't do this, the large coords of the transform cause are used when read back in to find
    // a planet domain, which of course falls over big time.
    if (actor.insideAPC())
        trans.position(MexPoint3d(10, 10, 0));

    PER_WRITE_RAW_OBJECT(ostr, trans);
    const W4dId id = actor.id();
    PER_WRITE_RAW_OBJECT(ostr, id);
    actor.persistenceWriteStrategy(ostr);
}

void perRead(PerIstream& istr, MachLogAggressor& actor)
{
    MachLogMachine& base1 = actor;
    MachLogCanAttack& base2 = actor;

    istr >> base1;
    istr >> base2;

    istr >> actor.subType_;

    MexTransform3d trans;
    W4dId id;
    PER_READ_RAW_OBJECT(istr, trans);
    PER_READ_RAW_OBJECT(istr, id);

    MachPhysAggressor* pPhysAggressor = MachLogAggressor::pNewPhysAggressor(
        actor.subType_,
        base1.hwLevel(),
        base1.swLevel(),
        &actor.logRace(),
        trans.position(),
        base2.weaponCombo());
    actor.setObjectPtr(pPhysAggressor, trans);
    actor.setPhysCanAttack(pPhysAggressor);
    actor.id(id);
    MachLogArmourer::rearm(&actor);
    actor.machineCreated();
    actor.persistenceReadStrategy(istr);
}

MachLogAggressor::MachLogAggressor(PerConstructor con)
    : MachLogMachine(con)
    , MachLogCanAttack(con)
{
}

bool MachLogAggressor::isEradicator() const
{
    return subType() == MachPhys::BALLISTA && hwLevel() == 3 && weaponCombo() == MachPhys::T_VORTEX;
}

// Forced recompile 16/2/99 CPS
/* //////////////////////////////////////////////////////////////// */

/* End AGGRESSR.CPP **************************************************/
