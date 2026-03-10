#include "ctl/CountedPtr.hpp"
// #include "ctl/CountedPtr.ctp"
#include "phys/Plans/ScalarPlan.hpp"

// These included to prevent compiler complaining
#include "mathex/mathex.hpp"
#include "mathex/Transform3d.hpp"
#include "phys/Plans/MotionPlan.hpp"
#include "phys/Plans/RampAcceleration.hpp"
#include "ctl/Vector.hpp"

PhysDummyFunction19()
{
    static CtlCountedPtr<PhysScalarPlan> sPtr;
}
