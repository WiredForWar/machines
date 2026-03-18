/*
 * D A M A G W A V . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Effects/BurstWave.hpp"
#include "machphys/Persistence/OtherPersistence.hpp"

#include "machphys/machphys.hpp"

#include "render/TextureManager.hpp"
#include "render/Texture.hpp"
#include "render/Colour.hpp"
#include "render/MaterialVec.hpp"
#include "render/Mesh.hpp"
#include "render/MeshInstance.hpp"
#include "render/Material.hpp"

#include "system/PathName.hpp"

#include "world4d/Entity/Root.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/GeneralUniformScalePlan.hpp"
#include "world4d/Plans/MaterialFramePlan.hpp"
#include "world4d/Plans/SimpleScalePlan.hpp"
#include "world4d/Plans/SimpleAlphaPlan.hpp"

#include "mathex/Transform3d.hpp"

#include "phys/Plans/LinearScalarPlan.hpp"
#include "ctl/Vector.hpp"
#include "ctl/CountedPtr.hpp"

PER_DEFINE_PERSISTENT(MachPhysBurstWave);

// One-time ctor
MachPhysBurstWave::MachPhysBurstWave()
    : W4dEntity(MachPhysOtherPersistence::instance().pRoot(), MexTransform3d(), W4dEntity::NOT_SOLID)
{
    // The current model is all emissive or black.  Hence, it should not need
    // lighting.  This could change if the model changes.
    doNotLight(true);

    // Load the mesh data
    readLODFile(SysPathName("models/destroy/construc/shockwav.lod"));

    TEST_INVARIANT;
}

// public ctor
MachPhysBurstWave::MachPhysBurstWave(W4dEntity* pParent, const MexTransform3d& localTransform)
    : W4dEntity(exemplar(), pParent, localTransform)
{
    // The current model is all emissive or black.  Hence, it should not need
    // lighting.  This could change if the model changes.
    doNotLight(true);

    // make invisible until required
    visible(false);

    TEST_INVARIANT;
}

MachPhysBurstWave::~MachPhysBurstWave()
{
    TEST_INVARIANT;
}

// static
const MachPhysBurstWave& MachPhysBurstWave::exemplar()
{
    return MachPhysOtherPersistence::instance().burstWaveExemplar();
}

void MachPhysBurstWave::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysBurstWave& t)
{

    o << "MachPhysBurstWave " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysBurstWave " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachPhysBurstWave::intersectsLine(const MexLine3d&, MATHEX_SCALAR*, Accuracy) const
{
    return false;
}

void MachPhysBurstWave::startBurstWave(
    const PhysAbsoluteTime& startTime,
    const PhysRelativeTime& duration,
    const MATHEX_SCALAR& fromRadius,
    const MATHEX_SCALAR& toRadius,
    const MATHEX_SCALAR& zScale)
{
    // Get the entity plan for this entity
    W4dEntityPlan& entityPlan = entityPlanForEdit();

    // Add the visibility plan
    W4dVisibilityPlanPtr visibilityPlanPtr = new W4dVisibilityPlan(true);
    visibilityPlanPtr->add(false, duration);
    entityPlan.visibilityPlan(visibilityPlanPtr, startTime);

    // Make a simple scale plan
    const MATHEX_SCALAR iniScale = fromRadius / burstWaveDefaultSize();
    const MATHEX_SCALAR endScale = toRadius / burstWaveDefaultSize();

    RenNonUniformScale a(iniScale, iniScale, zScale);
    RenNonUniformScale b(endScale, endScale, zScale);
    W4dScalePlanPtr planPtr(new W4dSimpleNonUniformScalePlan(a, b, duration));

    propogateScalePlan(planPtr, startTime, 1);

    // fade out in the last quater of duration
    // RenMaterialVec* pFlashMaterialVec =const_cast<const W4dEntity*>(this)->mesh().mesh()->materialVec().release();
    RenMaterialVec* pFlashMaterialVec
        = const_cast<const W4dEntity*>(static_cast<W4dEntity*>(this))->mesh().mesh()->materialVec().release();
    RenMaterial& matFlash = (*pFlashMaterialVec)[0];

    PhysLinearScalarPlan::ScalarVec times;
    times.reserve(4);
    times.push_back(duration * 4.0 / 16.0);
    times.push_back(duration * 7.0 / 16.0);
    times.push_back(duration * 10.0 / 16.0);
    times.push_back(duration * 13.0 / 16.0);
    times.push_back(duration);

    PhysLinearScalarPlan::ScalarVec scales;
    scales.reserve(5);
    scales.push_back(1.0);
    scales.push_back(1.0);
    scales.push_back(0.5625);
    scales.push_back(0.25);
    scales.push_back(0.0625);
    scales.push_back(0.0); // 1 - 2x + x^2

    PhysScalarPlanPtr lineScalarPlanPtr = new PhysLinearScalarPlan(times, scales);

    W4dMaterialPlanPtr alphaPlanPtr
        = new W4dSimpleAlphaPlan(matFlash, pFlashMaterialVec->size(), lineScalarPlanPtr, 1);
    entityPlanForEdit().materialPlan(alphaPlanPtr, startTime);
}

// static
const double& MachPhysBurstWave::burstWaveDefaultSize()
{
    static const double SIZE = 2.0; // defined in its .x file
    return SIZE;
}

void perWrite(PerOstream& ostr, const MachPhysBurstWave& wave)
{
    const W4dEntity& base = wave;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysBurstWave& wave)
{
    W4dEntity& base = wave;

    istr >> base;
}

MachPhysBurstWave::MachPhysBurstWave(PerConstructor c)
    : W4dEntity(c)
{
}

/* End DAMAGWAV.CPP *************************************************/
