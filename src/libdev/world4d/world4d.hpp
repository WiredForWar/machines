/*
 * W O R L D 4 D . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _WORLD4D_WORLD4D_HPP
#define _WORLD4D_WORLD4D_HPP

#include "base/base.hpp"
#include "base/diag.hpp"
#include "mathex/mathex.hpp"
#include "utility/id.hpp"

class MexDouble;
class MexTransform3d;

// A link identifier
using W4dLinkId = uint;

// An entity id
using W4dId = UtlId;

// A level of detail id
using W4dLOD = int;

using W4dTransform3d = MexTransform3d;

// Namespace declarations
class W4d
{
public:
    // The explicit values allow enumeration objects to be used to index into
    // vectors, points etc, and are guaranteed not to change.
    enum Axis
    {
        X_AXIS = 0,
        Y_AXIS = 1,
        Z_AXIS = 2
    };
};

#endif
