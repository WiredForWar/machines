/*
 * T I 1 2. C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

// #define _INSTANTIATE_TEMPLATE_CLASSES

#include "ctl/Vector.hpp"
#include "ctl/Vector.ctp"

#include "machphys/Constructions/Station.hpp"
#include "machphys/Terrain/PlanetSurface.hpp"
#include "mathex/Transform3d.hpp"
#include "mathex/Point3d.hpp"
#include "render/Texture.hpp"
#include <memory>
#include <memory.ctp>

void MachDummyFunction12()
{
    static ctl_vector<MachPhysStation> dummy1;
    static ctl_vector<MexTransform3d> dummy2;
    static ctl_vector<MexPoint3d> dummy3;
    static ctl_vector<RenTexture> dummy4;
    static ctl_vector<ctl_vector<RenTexture>> dummy5;
    static unique_ptr<istream> dummy6;
    static MachPhysPlanetSurface::Floors dummy7;
}

/* End TI12.CPP *****************************************************/
