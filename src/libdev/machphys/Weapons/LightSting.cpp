/*
 * S T I N G L . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "mathex/Point2d.hpp"

#include "machphys/Weapons/LightSting.hpp"

#include "base/Diag.hpp"
#include "machphys/Persistence/OtherPersistence.hpp"
#include "ctl/Vector.hpp"
#include "system/PathName.hpp"
#include "mathex/Transform3d.hpp"
#include "mathex/Random.hpp"

#include "world4d/Entity/Composite.hpp"
#include "world4d/Entity/Link.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Scene/GarbageCollector.hpp"
#include "world4d/Sound/SoundManager.hpp"
#include "world4d/Entity/Generic.hpp"

#include "machphys/Data/SoundData.hpp"
#include "machphys/Effects/LightStingAura.hpp"
#include "machphys/Effects/LightStingSplat.hpp"

PER_DEFINE_PERSISTENT(MachPhysLightSting);

// public constructor
MachPhysLightSting::MachPhysLightSting(W4dEntity* pParent, const MexTransform3d& localTransform)
    : MachPhysLinearProjectile(exemplar(), pParent, localTransform)
{

    TEST_INVARIANT;
}

// one time constructor
MachPhysLightSting::MachPhysLightSting()
    : MachPhysLinearProjectile(MachPhysOtherPersistence::instance().pRoot(), MexTransform3d())
{
    readCompositeFile(SysPathName("models/weapons/wasp/light/bullet.cdf"));

    TEST_INVARIANT;
}

MachPhysLightSting::~MachPhysLightSting()
{
    TEST_INVARIANT;
}

// static
const MachPhysLightSting& MachPhysLightSting::exemplar()
{
    return MachPhysOtherPersistence::instance().lightStingExemplar();
}

void MachPhysLightSting::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysLightSting& t)
{

    o << "MachPhysLightSting " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysLightSting " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
PhysRelativeTime MachPhysLightSting::doBeDestroyedAt(const PhysAbsoluteTime& time, MachPhys::StrikeType strikeType)
{
    PhysRelativeTime animationDuration = 0.0;

    switch (strikeType)
    {
        case MachPhys::ON_OBJECT:
        case MachPhys::ON_TERRAIN:
            {
                // get the local position of the bolt at the destruct time
                MexTransform3d destructPosition;
                uint n;
                const W4dEntityPlan& myEntityPlan = entityPlan();
                if (myEntityPlan.hasMotionPlan())
                    entityPlan().transform(time, &destructPosition, &n);
                else
                    destructPosition = localTransform();

                // Create an impact flash TTF at this position
                animationDuration = createImpactSplat(pParent(), destructPosition, time);

                SoundId explosionId = SID_XPLODE1_MISSILE;
                MexBasicRandom tempRandom(MexBasicRandom::constructSeededFromTime());
                int randomNumber = mexRandomInt(&tempRandom, 0, 100);
                if (randomNumber < 50)
                {
                    explosionId = SID_XPLODE1_MISSILE;
                }
                else
                {
                    explosionId = SID_XPLODE2_MISSILE;
                }
                SOUND_STREAM("Playing sting explosion at " << time << std::endl);
                W4dGeneric* pExplosionSite = new W4dGeneric(pParent(), destructPosition);
                W4dGarbageCollector::instance().add(pExplosionSite, time + 10);
                W4dSoundManager::instance().play(pExplosionSite, explosionId, time, 1);

                break;
            }
    }

    return animationDuration;
}

PhysRelativeTime MachPhysLightSting::createImpactSplat(
    W4dEntity* pParent,
    const MexTransform3d& localPosition,
    const PhysAbsoluteTime& startTime)
{
    PhysRelativeTime duration = 0.33;

    // splat and aura
    MachPhysLightStingAura* pAura = new MachPhysLightStingAura(pParent, localPosition);
    pAura->startLightStingAura(startTime, duration);

    MachPhysLightStingSplat* pSplat = new MachPhysLightStingSplat(pParent, localPosition);
    pSplat->startLightStingSplat(startTime, duration);

    W4dGarbageCollector::instance().add(pSplat, startTime + duration);
    W4dGarbageCollector::instance().add(pAura, startTime + duration);

    return duration;
}

MachPhysLightSting::MachPhysLightSting(PerConstructor con)
    : MachPhysLinearProjectile(con)
{
}

void perWrite(PerOstream& ostr, const MachPhysLightSting& sting)
{
    const MachPhysLinearProjectile& base = sting;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysLightSting& sting)
{
    MachPhysLinearProjectile& base = sting;

    istr >> base;
}

/* End STINGL.CPP ***************************************************/
