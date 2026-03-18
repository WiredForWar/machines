/*
 * P L A S A U R A . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "mathex/Point2d.hpp"

#include "machphys/Effects/PlasmaAura.hpp"
#include "machphys/Persistence/OtherPersistence.hpp"

#include "ctl/Vector.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/GeneralUniformScalePlan.hpp"

#include "render/Mesh.hpp"
#include "render/MeshInstance.hpp"
#include "render/Material.hpp"
#include "render/MaterialVec.hpp"
#include "render/Colour.hpp"

#include "phys/Plans/LinearScalarPlan.hpp"

#include "system/PathName.hpp"

PER_DEFINE_PERSISTENT(MachPhysPlasmaAura);

// One-time ctor
MachPhysPlasmaAura::MachPhysPlasmaAura()
    : W4dEntity(MachPhysOtherPersistence::instance().pRoot(), MexTransform3d(), W4dEntity::NOT_SOLID)
{
    // Load the mesh data
    readLODFile(SysPathName("models/weapons/splat/aura.lod"));
    TEST_INVARIANT;
}

// public ctor
MachPhysPlasmaAura::MachPhysPlasmaAura(W4dEntity* pParent, const MexTransform3d& localTransform)
    : W4dEntity(exemplar(), pParent, localTransform)
{
    // make invisible until required
    visible(false);
    TEST_INVARIANT;
}

MachPhysPlasmaAura::MachPhysPlasmaAura(PerConstructor con)
    : W4dEntity(con)
{
}

MachPhysPlasmaAura::~MachPhysPlasmaAura()
{
    TEST_INVARIANT;
}

// static
const MachPhysPlasmaAura& MachPhysPlasmaAura::exemplar()
{
    return MachPhysOtherPersistence::instance().plasmaAuraExemplar();
}

void MachPhysPlasmaAura::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysPlasmaAura& t)
{

    o << "MachPhysPlasmaAura " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysPlasmaAura " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachPhysPlasmaAura::intersectsLine(const MexLine3d&, MATHEX_SCALAR*, Accuracy) const
{
    return false;
}

void perWrite(PerOstream& ostr, const MachPhysPlasmaAura& scorch)
{
    const W4dEntity& base = scorch;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysPlasmaAura& scorch)
{
    W4dEntity& base = scorch;

    istr >> base;
}

void MachPhysPlasmaAura::startPlasmaAura(const PhysAbsoluteTime& startTime, const PhysRelativeTime& duration)
{
    /*
    //set the colour
    Ren::ConstMeshPtr animMeshPtr = const_cast<const W4dEntity*>(this)->mesh().mesh();
    RenMaterialVec* pAnimMaterialVec =animMeshPtr->materialVec().release();

    (*pAnimMaterialVec)[0].diffuse( RenColour( 0, 188.0/255.0, 0) );
*/
    // Counted pointers etc for standard plans
    static W4dVisibilityPlanPtr visibilityPlanPtr;
    static W4dScalePlanPtr scalePlanPtr;

    // First time set up standard plans
    static bool firstTime = true;
    if (firstTime)
    {
        firstTime = false;

        // Create the visibility plan
        W4dVisibilityPlan* pVisibilityPlan = new W4dVisibilityPlan(true);
        pVisibilityPlan->add(false, duration);
        visibilityPlanPtr = pVisibilityPlan;

        // scale plan
        PhysRelativeTime interval = duration / 3.0;
        PhysLinearScalarPlan::ScalarVec times;
        times.reserve(3);
        times.push_back(interval);
        times.push_back(interval * 2.0);
        times.push_back(duration);

        PhysLinearScalarPlan::ScalarVec scales;
        scales.reserve(4);
        scales.push_back(1);
        scales.push_back(2);
        scales.push_back(1);
        scales.push_back(0.5);

        scalePlanPtr = new W4dGeneralUniformScalePlan(new PhysLinearScalarPlan(times, scales));
    }

    // Set its visibility plan
    W4dEntityPlan& entityPlan = entityPlanForEdit();
    entityPlan.visibilityPlan(visibilityPlanPtr, startTime);

    // Set the scaling plan
    entityPlan.scalePlan(scalePlanPtr, startTime);
}
/* End PLASAURA.CPP *************************************************/
