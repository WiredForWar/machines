/*
 * V O R T B O M B . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Effects/GroundScorch.hpp"


#include "machphys/Persistence/WeaponPersistence.hpp"

#include "world4d/Entity/Root.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/GeneralUniformScalePlan.hpp"

#include "mathex/Transform3d.hpp"

#include "system/PathName.hpp"

#include "ctl/Vector.hpp"

#include "phys/Plans/RampAcceleration.hpp"
#include "phys/Plans/AcceleratedScalarPlan.hpp"

#include "machphys/Persistence/OtherPersistence.hpp"

PER_DEFINE_PERSISTENT(MachPhysGroundScorch);

// One-time ctor
MachPhysGroundScorch::MachPhysGroundScorch()
    : W4dEntity(MachPhysOtherPersistence::instance().pRoot(), MexTransform3d(), W4dEntity::NOT_SOLID)
{
    // The current model is all emissive or black.  Hence, it should not need
    // lighting.  This could change if the model changes.
    doNotLight(true);

    // Load the mesh data
    readLODFile(SysPathName("models/weapons/vortex/vxfloor.lod"));
    TEST_INVARIANT;
}

// public ctor
MachPhysGroundScorch::MachPhysGroundScorch(W4dEntity* pParent, const MexTransform3d& localTransform)
    : W4dEntity(exemplar(), pParent, localTransform)
{
    // The current model is all emissive or black.  Hence, it should not need
    // lighting.  This could change if the model changes.
    doNotLight(true);

    // make invisible until required
    visible(false);
    TEST_INVARIANT;
}

MachPhysGroundScorch::MachPhysGroundScorch(PerConstructor con)
    : W4dEntity(con)
{
}

MachPhysGroundScorch::~MachPhysGroundScorch()
{
    TEST_INVARIANT;
}

// static
const MachPhysGroundScorch& MachPhysGroundScorch::exemplar()
{
    return MachPhysOtherPersistence::instance().groundScorchExemplar();
}

void MachPhysGroundScorch::CLASS_INVARIANT
{
    INVARIANT(this != NULL);
}

std::ostream& operator<<(std::ostream& o, const MachPhysGroundScorch& t)
{

    o << "MachPhysGroundScorch " << (void*)&t << " start" << std::endl;
    o << "MachPhysGroundScorch " << (void*)&t << " end" << std::endl;

    return o;
}

// virtual
bool MachPhysGroundScorch::intersectsLine(const MexLine3d&, MATHEX_SCALAR*, Accuracy) const
{
    return false;
}

void perWrite(PerOstream& ostr, const MachPhysGroundScorch& scorch)
{
    const W4dEntity& base = scorch;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysGroundScorch& scorch)
{
    W4dEntity& base = scorch;

    istr >> base;
}

/* End VORTBOMB.CPP *************************************************/
