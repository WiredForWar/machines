/*
 * D O M A S I G N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "machphys/Terrain/DomainAssignor.hpp"
#include "ctl/List.hpp"
#include "world4d/Scene/Light.hpp"
#include "world4d/Scene/Domain.hpp"
#include "mathex/AlignedBox3d.hpp"
#include "mathex/Point3d.hpp"
#include "machphys/Terrain/PlanetSurface.hpp"
#include <algorithm>

MachPhysDomainAssignor::MachPhysDomainAssignor(const MachPhysPlanetSurface* p)
    : pPlanet_(p)
{
    PRE(p);
}

// virtual
MachPhysDomainAssignor::~MachPhysDomainAssignor()
{
}

// virtual
void MachPhysDomainAssignor::assignDomains(W4dRoot* root, W4dLight* light)
{
    // ASSERT(light->findRoot() == root, "wrong tree; roots don't match");

    if (light->hasBoundingSphere())
    {
        pPlanet_->domainsAt(light);
    }
}

/* End DOMASIGN.CPP *************************************************/
