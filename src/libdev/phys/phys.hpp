/*
 * M A T H E X . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _PHYS_PHYS_HPP
#define _PHYS_PHYS_HPP

#include "base/base.hpp"
#include "mathex/mathex.hpp"

// Represents a level of detail value for use in eg bounding volume
// intersection tests.
using PhysLOD = uint;

// Represents the time of an event
using PhysRelativeTime = MATHEX_SCALAR;
using PhysAbsoluteTime = MATHEX_SCALAR;

using PhysCompressedRelativeTime = float;
using PhysCompressedAbsoluteTime = float;

using PhysPathFindingPriority = int;

class Phys
{
public:
    static PhysAbsoluteTime time();
    static PhysPathFindingPriority defaultPathFindingPriority();
};

// Forward declarations
class PhysMotionPlan;
template <class T> class CtlCountedPtr;
using PhysMotionPlanPtr = CtlCountedPtr<PhysMotionPlan>;

//////////////////////////////////////////////////////////////////////

#ifdef _INLINE
//    #include "phys/phys.ipp"
//    #include "phys/phys.itf"
#endif

#ifdef _INSTANTIATE_TEMPLATE_FUNCTIONS
//    #include "phys/phys.ctf"
#endif

//////////////////////////////////////////////////////////////////////

#endif /*  #ifndef _MEX_MATHEX_HPP    */

/* End MATHEX.HPP *****************************************************/
