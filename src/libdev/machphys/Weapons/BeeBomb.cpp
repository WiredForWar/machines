/*
 * B E E B O M B . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Weapons/BeeBomb.hpp"
#include "machphys/Persistence/OtherPersistence.hpp"

#include "base/diag.hpp"
#include "system/pathname.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/line2d.hpp"
#include "mathex/line3d.hpp"

#include "phys/Plans/RampAcceleration.hpp"
#include "ctl/vector.hpp"
#include "phys/Plans/MotionPlan.hpp"
#include "phys/Plans/LinearMotionPlan.hpp"
#include "phys/Plans/TimedAnglePlan.hpp"
#include "phys/Plans/LinearScalarPlan.hpp"

#include "render/colour.hpp"

#include "world4d/Entity/Composite.hpp"
#include "world4d/Entity/Link.hpp"
#include "world4d/Scene/Domain.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Plans/GeneralUniformScalePlan.hpp"
#include "world4d/Scene/GarbageCollector.hpp"

#include "machphys/Effects/BeeBombWave.hpp"
#include "machphys/Effects/BeeBombExplosion.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machphys/Data/Data.hpp"
#include "machphys/Random.hpp"
#include "machphys/Terrain/PlanetSurface.hpp"
#include "machphys/Effects/VapourTrail.hpp"

#include "sim/manager.hpp"

PER_DEFINE_PERSISTENT(MachPhysBeeBomb);

// public constructor
MachPhysBeeBomb::MachPhysBeeBomb(W4dEntity* pParent, const MexTransform3d& localTransform)
    : MachPhysTrailedProjectile(exemplar(), pParent, localTransform)
{
    // pFlame_ = links()[exemplar().pFlame_->id()];

    if (!findLink("flame", &pFlame_))
        pFlame_ = nullptr;

    ASSERT(pFlame_ != nullptr, " flame not exist");
    pFlame_->visible(false); // only be visible when the missile is launched

    TEST_INVARIANT;
}

// one time constructor
MachPhysBeeBomb::MachPhysBeeBomb()
    : MachPhysTrailedProjectile(MachPhysOtherPersistence::instance().pRoot(), MexTransform3d(), BEE_BOMB)
{
    readCompositeFile(SysPathName("models/weapons/bee/bomb.cdf"));

    if (!findLink("flame", &pFlame_))
        pFlame_ = nullptr;

    ASSERT(pFlame_ != nullptr, " flame not found");

    TEST_INVARIANT;
}

MachPhysBeeBomb::~MachPhysBeeBomb()
{
    TEST_INVARIANT;
}

void MachPhysBeeBomb::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// static
const MachPhysBeeBomb& MachPhysBeeBomb::exemplar()
{
    return MachPhysOtherPersistence::instance().beeBombExemplar();
}

std::ostream& operator<<(std::ostream& o, const MachPhysBeeBomb& t)
{

    o << "MachPhysBeeBomb " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysBeeBomb " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
PhysRelativeTime MachPhysBeeBomb::doBeDestroyedAt(const PhysAbsoluteTime& time, MachPhys::StrikeType)
{
    destructionTime_ = time;
    PhysRelativeTime animationDuration
        = 6.0; // can not be zero nor even s smaller value, otherwise crash. don't understand. Yueai

    // Make sure the vapour trail gets garbage collected
    if (pVapourTrail_ != nullptr)
        pVapourTrail_->finish(time + animationDuration);

    RICHARD_STREAM("Creating bomb blast" << std::endl);

    // Create the explosion
    // createBombExplosion( time );

    return animationDuration;
}

MachPhysBeeBomb::MachPhysBeeBomb(PerConstructor con)
    : MachPhysTrailedProjectile(con)
{
}

void perWrite(PerOstream& ostr, const MachPhysBeeBomb& beeBomb)
{
    const MachPhysTrailedProjectile& base = beeBomb;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysBeeBomb& beeBomb)
{
    MachPhysTrailedProjectile& base = beeBomb;

    istr >> base;
}

void MachPhysBeeBomb::createBombExplosion(const PhysAbsoluteTime& time)
{
    // Kick off a bomb explosion animation at the appropriate time, provided teh client has provided us
    // with a planet surface.
    if (hasPlanetSurface())
    {
        // Get the coordinates of the bomb when it explodes
        MexPoint3d explodePosition = flightPosition(time);

        // Project down onto the planet surface - get terrain height
        MachPhysPlanetSurface& surface = planetSurface();
        MATHEX_SCALAR z = surface.terrainHeight(explodePosition.x(), explodePosition.y());
        explodePosition.z(z);

        // Create the bomb explosion
        W4dDomain* pParent = surface.domainAt(explodePosition);
        pParent->globalTransform().transformInverse(&explodePosition);

        MachPhysBeeBombExplosion* pExplosion = new MachPhysBeeBombExplosion(pParent, MexTransform3d(explodePosition));

        PhysRelativeTime explosionDuration = pExplosion->startExplosion(time, surface);

        // Have it garbage collected
        W4dGarbageCollector::instance().add(pExplosion, time + explosionDuration);
    }
}
/* End BEEBOMB.CPP **************************************************/
