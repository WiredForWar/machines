/*
 * T I . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

//  Pre instantiate the template mathex classes over the current mathex scalar

#define _INSTANTIATE_TEMPLATE_CLASSES

#include <algorithm>

#include "mathex/mathex.hpp"

#include "mathex/Vec2.hpp"
// #include "mathex/box2.hpp"
#include "mathex/AlignedBox2d.hpp"

void MathexDummyFunction()
{
    static MexVec2<MATHEX_SCALAR> dummyPoint;
    static MexAlignedBox2d<MATHEX_SCALAR> dummyBox2d;
}

/* End TI.CPP *****************************************************/
