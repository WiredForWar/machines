#include "ctl/countptr.hpp"
// #include "ctl/countptr.ctp"
#include "phys/Plans/ScalarPlan.hpp"

// These included to prevent compiler complaining
#include "mathex/mathex.hpp"
#include "mathex/transf3d.hpp"
#include "phys/Plans/MotionPlan.hpp"
#include "phys/Plans/RampAcceleration.hpp"
#include "ctl/vector.hpp"

PhysDummyFunction19()
{
    static CtlCountedPtr<PhysScalarPlan> sPtr;
}
