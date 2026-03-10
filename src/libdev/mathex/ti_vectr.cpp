/*
 * T I . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#define _INSTANTIATE_TEMPLATE_CLASSES

#include "base/base.hpp"
#include <vector>
#include "ctl/Vector.hpp"

#include "mathex/Outcode.hpp"
#include "mathex/mathex.hpp"
#include "mathex/Grid2d.hpp"
#include "mathex/Point2d.hpp"
#include "mathex/Point3d.hpp"
#include "mathex/Vec2.hpp"
#include "mathex/ConvexPolygon2d.hpp"
#include "mathex/Transform3d.hpp"

void MathexDummyFunction1()
{
    static std::vector<OutCode> outcodeVec;
    static ctl_vector<MATHEX_SCALAR> d3;
    static ctl_vector<MexPoint2d> d4;
    static ctl_vector<MexPoint2d*> d5;
    static ctl_vector<MexVec2> d6;
    static ctl_vector<uint> d7;
    static std::unique_ptr<MexConvexPolygon2d::Points> d8;

    static MexXCoord<double> dummyX(0);
    static MexYCoord<double> dummyY(0);

    static ctl_vector<MexGrid2d::CellIndex> d9;
    static ctl_vector<int> d10;
    static ctl_vector<MexPoint3d> d11;
    static ctl_vector<MexTransform3d> d12;
}

PER_DEFINE_PERSISTENT_T1(MexXCoord, double);
PER_DEFINE_PERSISTENT_T1(MexYCoord, double);

/* End TI.CPP *****************************************************/
