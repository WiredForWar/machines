/*
 * L O A D L I G H . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/loadligh.hpp"
#include "machphys/private/otherper.hpp"
#include "system/pathname.hpp"
#include "mathex/transf3d.hpp"

PER_DEFINE_PERSISTENT(MachPhysLoadingLight);

// One-time ctor
MachPhysLoadingLight::MachPhysLoadingLight()
    : W4dEntity(MachPhysOtherPersistence::instance().pRoot(), MexTransform3d(), W4dEntity::NOT_SOLID)
{
    // Load the mesh data
    readLODFile(SysPathName("models/weapons/load/light.lod"));
    TEST_INVARIANT;
}

// public ctor
MachPhysLoadingLight::MachPhysLoadingLight(W4dEntity* pParent, const MexTransform3d& localTransform)
    : W4dEntity(exemplar(), pParent, localTransform)
{
    // make invisible until required
    visible(false);
    TEST_INVARIANT;
}

MachPhysLoadingLight::MachPhysLoadingLight(PerConstructor con)
    : W4dEntity(con)
{
}

MachPhysLoadingLight::~MachPhysLoadingLight()
{
    TEST_INVARIANT;
}

// static
const MachPhysLoadingLight& MachPhysLoadingLight::exemplar()
{
    return MachPhysOtherPersistence::instance().lightLoadingExemplar();
}

void MachPhysLoadingLight::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysLoadingLight& t)
{

    o << "MachPhysLoadingLight " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysLoadingLight " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachPhysLoadingLight::intersectsLine(const MexLine3d&, MATHEX_SCALAR*, Accuracy) const
{
    return false;
}

void perWrite(PerOstream& ostr, const MachPhysLoadingLight& light)
{
    const W4dEntity& base = light;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysLoadingLight& light)
{
    W4dEntity& base = light;

    istr >> base;
}

/* End LOADLIGH.CPP *************************************************/
