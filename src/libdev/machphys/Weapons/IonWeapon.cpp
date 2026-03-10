/*
 * B O L T E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Weapons/IonWeapon.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machphys/Persistence/WeaponPersistence.hpp"
#include "machphys/Data/SoundData.hpp"

#include "world4d/Sound/SoundManager.hpp"
#include "world4d/Entity/Root.hpp"
#include "world4d/Entity/Generic.hpp"
#include "world4d/Entity/GenericComposite.hpp"
#include "world4d/Scene/GarbageCollector.hpp"
#include "world4d/Entity/Composite.hpp"
#include "world4d/Plans/SimpleScalePlan.hpp"
#include "world4d/Plans/SimpleAlphaPlan.hpp"
#include "world4d/Plans/GeneralUniformScalePlan.hpp"
#include "world4d/Entity/EntityIter.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Entity/EntityPlan.hpp"

#include "phys/Plans/LinearScalarPlan.hpp"

#include "render/Mesh.hpp"
#include "render/MeshInstance.hpp"
#include "render/Scale.hpp"
#include "render/MaterialVec.hpp"
#include "render/Material.hpp"
#include "render/Colour.hpp"

#include "system/PathName.hpp"

PER_DEFINE_PERSISTENT(MachPhysIonWeapon);

MachPhysIonWeapon::MachPhysIonWeapon(
    W4dEntity* pParent,
    const MexTransform3d& localTransform,
    MachPhys::WeaponType type,
    MachPhys::Mounting mounting)
    : MachPhysWeapon(exemplar(type), mounting, pParent, localTransform)
{

    TEST_INVARIANT;
}

// One-time ctor
MachPhysIonWeapon::MachPhysIonWeapon(MachPhys::WeaponType type)
    : MachPhysWeapon(
        MachPhysWeaponPersistence::instance().pRoot(),
        MexTransform3d(),
        SysPathName(compositeFilePath(type)),
        type,
        MachPhys::LEFT)
{
    TEST_INVARIANT;
}

MachPhysIonWeapon::~MachPhysIonWeapon()
{
    TEST_INVARIANT;
}

// static
const MachPhysIonWeapon& MachPhysIonWeapon::exemplar(MachPhys::WeaponType type)
{
    return MachPhysWeaponPersistence::instance().ionExemplar(type);
}

void MachPhysIonWeapon::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysIonWeapon& t)
{

    o << "MachPhysIonWeapon " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysIonWeapon " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// static
const char* MachPhysIonWeapon::compositeFilePath(MachPhys::WeaponType)
{
    return "models/weapons/nmissile/point.cdf";
}

// virtual
PhysRelativeTime MachPhysIonWeapon::fire(const PhysAbsoluteTime&, int)
{
    return 0;
}

// static
PhysRelativeTime MachPhysIonWeapon::destroy(W4dEntity* pVictim, const PhysAbsoluteTime& startTime)
{
    // create a copy of the object being destroyed  through a  generic
    W4dEntity* pCopyTarget;

    if (pVictim->isComposite())
        pCopyTarget = new W4dGenericComposite(pVictim->asComposite(), pVictim->pParent(), pVictim->localTransform());
    else
        pCopyTarget = new W4dGeneric(*pVictim, pVictim->pParent(), pVictim->localTransform());

    PhysRelativeTime duration = 1.5;

    //  make plans for the original

    // Make a simple scale plan
    PhysLinearScalarPlan::ScalarVec intTimes;
    intTimes.reserve(1);
    intTimes.push_back(duration);

    PhysLinearScalarPlan::ScalarVec intensities;
    intensities.reserve(2);
    intensities.push_back(1);
    intensities.push_back(3);

    PhysLinearScalarPlan* pScalePlan = new PhysLinearScalarPlan(intTimes, intensities);
    W4dScalePlanPtr planPtr(new W4dGeneralUniformScalePlan(pScalePlan));

    // Propogate thru the current model
    pVictim->propogateScalePlan(planPtr, startTime, 1);

    // make a visibility plan

    W4dVisibilityPlanPtr wVisibilityPlanPtr(new W4dVisibilityPlan(true));
    wVisibilityPlanPtr->add(false, duration);

    pVictim->entityPlanForEdit().visibilityPlan(wVisibilityPlanPtr, startTime);

    // alpha plan. Ensure construct material only once, since any change to a shared material involves
    // searching the entire set of materials for a match.
    static RenMaterial glowingWhite;
    static bool firstTime = true;
    if (firstTime)
    {
        glowingWhite.diffuse(RenColour::black());
        glowingWhite.emissive(RenColour::white());
        firstTime = false;
    }

    const int reasonableSize = 100;
    RenMaterialVec* pMaterialVec = new RenMaterialVec(reasonableSize);
    for (int i = reasonableSize; i--;)
        pMaterialVec->push_back(glowingWhite);

    PhysLinearScalarPlan::ScalarVec linearTimes;
    linearTimes.reserve(1);
    linearTimes.push_back(duration);

    PhysLinearScalarPlan::ScalarVec scales;
    scales.reserve(2);
    scales.push_back(1);
    scales.push_back(0);

    PhysLinearScalarPlan* pAlphaPlan = new PhysLinearScalarPlan(linearTimes, scales);
    PhysScalarPlanPtr alphaPlanPtr(pAlphaPlan);

    W4dSimpleAlphaPlan* pPlan = new W4dSimpleAlphaPlan(glowingWhite, reasonableSize, alphaPlanPtr, 5);

    W4dMaterialPlanPtr pMaterialPlanPtr(pPlan);

    for (W4dEntityIter it(pVictim); ! it.isFinished(); ++it)
    {
        (*it).entityPlanForEdit().materialPlan(pMaterialPlanPtr, startTime);
        (*it).entityPlanForEdit().visibilityPlan(wVisibilityPlanPtr, startTime);
    }

    //  make plans for the copy

    // apply the same material and visibility plan  to the copy
    for (W4dEntityIter it(pCopyTarget); ! it.isFinished(); ++it)
    {
        (*it).entityPlanForEdit().materialPlan(pMaterialPlanPtr, startTime);
        (*it).entityPlanForEdit().visibilityPlan(wVisibilityPlanPtr, startTime);
    }

    // but a different scale plan

    PhysLinearScalarPlan::ScalarVec copyScales;
    copyScales.reserve(2);
    copyScales.push_back(1);
    copyScales.push_back(0.001);

    PhysLinearScalarPlan* pCopyScalePlan = new PhysLinearScalarPlan(intTimes, copyScales);
    W4dScalePlanPtr copyPlanPtr(new W4dGeneralUniformScalePlan(pCopyScalePlan));

    // Propogate thru the current model
    pCopyTarget->propogateScalePlan(copyPlanPtr, startTime, 1);

    // destroy the copytarget when it becomes invisible
    W4dGarbageCollector::instance().add(pCopyTarget, startTime + duration);

    return duration;
}

MachPhysIonWeapon::MachPhysIonWeapon(PerConstructor con)
    : MachPhysWeapon(con)
{
}

void perWrite(PerOstream& ostr, const MachPhysIonWeapon& weapon)
{
    const MachPhysWeapon& base = weapon;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysIonWeapon& weapon)
{
    MachPhysWeapon& base = weapon;

    istr >> base;
}

/* End ionbeam.CPP ****************************************************/
