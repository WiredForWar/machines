/*
 * T I . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#define _INSTANTIATE_TEMPLATE_CLASSES
#include "ctl/FixedVector.hpp"
#undef _INSTANTIATE_TEMPLATE_CLASSES

#include "ctl/Vector.hpp"
#include "ctl/Vector.ctp"
#include "ctl/PtrVector.hpp"
// #include "ctl/List.hpp"

// #include "ctl/Vector.ctp"
// #include <ospace/stl/vector.cc>

#include "mathex/Transform3d.hpp"
#include "mathex/Point2d.hpp"
#include "world4d/Entity/CompositePlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/FloatValuePlan.hpp"

#include "render/Material.hpp"
#include "machphys/Terrain/TileBoundary.hpp"
#include "machphys/Terrain/TileData.hpp"
#include "machphys/machphys.hpp"
#include "machphys/Terrain/PlanetSurfaceChecker.hpp"

class W4dDomain;
class W4dLink;
class MachPhysTerrainTile;
class MachPhysFireball;

void MachDummyFunction1()
{
    static ctl_vector<W4dDomain*> dummyVectorW4dDomain;
    static ctl_vector<ctl_vector<W4dDomain*>> dummyVectorVectorW4dDomain;
    static ctl_vector<MachPhysTerrainTile*> dummyVectorTerrainTile;
    static ctl_vector<ctl_vector<MachPhysTerrainTile*>> dummyVectorVectorTerrainTile;
    static ctl_vector<MachPhysFireball*> dummyVectorFireballs;

    static ctl_vector<MATHEX_SCALAR> dummyVectorMATHEX_SCALAR;
    static ctl_vector<MachPhysTileBoundary::TriangleData> dummyVectorTriangleData;
    static ctl_vector<MachPhysTileBoundary::CellPair> dummyVectorCellPair;
    static ctl_vector<W4dCompositePlanPtr> dummyCompositePlanPtrs;

    static ctl_vector<MachPhys::Race> dummy1;
    static ctl_fixed_vector<W4dLink*> dummy3(2, NULL);

    static MachPhysTileData::IntersectingTriangles dummy2;

    static MachPhysPlanetSurfaceChecker::Triangles dummy10;

    static ctl_pvector<W4dDomain> dummyPVectorW4dDomain;

    static ctl_vector<MexPoint2d> dummyPoints;
}

/* End TI.CPP *****************************************************/
