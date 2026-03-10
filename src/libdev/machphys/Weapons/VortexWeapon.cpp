/*
 * B O L T E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Weapons/VortexWeapon.hpp"
#include "machphys/Data/SoundData.hpp"

#include "ctl/CountedPtr.hpp"
#include "mathex/Transform3d.hpp"
#include "phys/Plans/RampAcceleration.hpp"
#include "world4d/Entity/Root.hpp"
#include "world4d/Sound/SoundManager.hpp"
#include "world4d/Entity/Generic.hpp"
#include "world4d/Scene/GarbageCollector.hpp"
#include "world4d/Entity/Composite.hpp"
#include "world4d/Plans/SimpleScalePlan.hpp"
#include "world4d/Scene/Light.hpp"
#include "world4d/Scene/GarbageCollector.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Entity/EntityPlan.hpp"

#include "phys/Plans/LinearScalarPlan.hpp"
#include "phys/Plans/TimedAnglePlan.hpp"

#include "render/Mesh.hpp"
#include "render/MeshInstance.hpp"
#include "render/Scale.hpp"

#include "machphys/Persistence/WeaponPersistence.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "system/PathName.hpp"

PER_DEFINE_PERSISTENT(MachPhysVortexWeapon);

MachPhysVortexWeapon::MachPhysVortexWeapon(
    W4dEntity* pParent,
    const MexTransform3d& localTransform,
    MachPhys::Mounting mounting)
    : MachPhysWeapon(exemplar(), mounting, pParent, localTransform)
{

    TEST_INVARIANT;
}

// One-time ctor
MachPhysVortexWeapon::MachPhysVortexWeapon()
    : MachPhysWeapon(
        MachPhysWeaponPersistence::instance().pRoot(),
        MexTransform3d(),
        SysPathName(compositeFilePath()),
        MachPhys::VORTEX,
        MachPhys::TOP)
{
    TEST_INVARIANT;
}

MachPhysVortexWeapon::~MachPhysVortexWeapon()
{
    TEST_INVARIANT;
}

// static
const MachPhysVortexWeapon& MachPhysVortexWeapon::exemplar()
{
    return MachPhysWeaponPersistence::instance().vortexExemplar();
}

void MachPhysVortexWeapon::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysVortexWeapon& t)
{

    o << "MachPhysVortexWeapon " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysVortexWeapon " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// static
const char* MachPhysVortexWeapon::compositeFilePath()
{
    return "models/weapons/vortex/vrtx.cdf";
}

// virtual
PhysRelativeTime MachPhysVortexWeapon::fire(const PhysAbsoluteTime&, int)
{

    return 0;
}

PhysRelativeTime MachPhysVortexWeapon::destroy(W4dEntity* pVictim, const PhysAbsoluteTime& startTime)
{
    PhysRelativeTime duration = 0.5;
    // Make a simple scale plan
    RenNonUniformScale a(1, 1, 1);
    RenNonUniformScale b(0.125, 0.125, 20);
    W4dScalePlanPtr planPtr(new W4dSimpleNonUniformScalePlan(a, b, duration));

    // Propogate thru the current model
    pVictim->propogateScalePlan(planPtr, startTime, 1);

    // make a visibility plan

    W4dVisibilityPlanPtr wVisibilityPlanPtr(new W4dVisibilityPlan(true));
    wVisibilityPlanPtr->add(false, duration);

    pVictim->entityPlanForEdit().visibilityPlan(wVisibilityPlanPtr, startTime);

    // Add a brief light so that he victim is lit up
    const MATHEX_SCALAR lightRange = 20.0;
    W4dUniformLight* pLight = new W4dUniformLight(pVictim, MexVec3(0, 0, 1), lightRange);
    pLight->colour(RenColour(2, 2, 2.3));
    pLight->constantAttenuation(0);
    pLight->linearAttenuation(0.177);
    pLight->quadraticAttenuation(0.823);
    pLight->scope(W4dLight::DYNAMIC);
    pLight->visible(false);

    PhysAbsoluteTime lightStartTime = startTime - 0.5;
    PhysRelativeTime lightDuration = duration + 0.5;
    W4dVisibilityPlanPtr visibilityPlanPtr(new W4dVisibilityPlan(true));
    visibilityPlanPtr->add(false, duration);
    pLight->entityPlanForEdit().visibilityPlan(visibilityPlanPtr, lightStartTime);

    PhysLinearScalarPlan::ScalarVec lightTimes;
    lightTimes.reserve(2);
    lightTimes.push_back(0.3);
    lightTimes.push_back(lightDuration);

    PhysLinearScalarPlan::ScalarVec intensities;
    intensities.reserve(3);
    intensities.push_back(1);
    intensities.push_back(1);
    intensities.push_back(0);

    PhysLinearScalarPlan* plightIntensityPlan = new PhysLinearScalarPlan(lightTimes, intensities);

    PhysScalarPlanPtr intensityPlanPtr = plightIntensityPlan;
    pLight->intensityPlan(intensityPlanPtr, lightStartTime);

    W4dGarbageCollector::instance().add(pLight, lightStartTime + lightDuration);

    return duration;
}

MachPhysVortexWeapon::MachPhysVortexWeapon(PerConstructor con)
    : MachPhysWeapon(con)
{
}

void perWrite(PerOstream& ostr, const MachPhysVortexWeapon& weapon)
{
    const MachPhysWeapon& base = weapon;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysVortexWeapon& weapon)
{
    MachPhysWeapon& base = weapon;

    istr >> base;
}

/* End BOLTER.CPP ****************************************************/
