#include "ctl/CountedPtr.hpp"
#include "ctl/CountedPtr.ctp"

// The files marked "accidental" shouldn't be necessary.  They must be included
// because Watcom is trying to instantiate everything it sees.
#include "ctl/Vector.hpp" // accidental
#include "mathex/Transform3d.hpp" // accidental
#include "phys/Plans/RampAcceleration.hpp" // accidental
#include "phys/Plans/MotionPlan.hpp" // accidental
#include "render/Mesh.hpp" // accidental
#include "render/MaterialVec.hpp" // accidental
#include "render/UVTransform.hpp" // accidental

#include "world4d/Entity/CompositePlan.hpp" // accidental
#include "world4d/Plans/FloatValuePlan.hpp" // accidental
#include "world4d/Plans/VisibilityPlan.hpp" // accidental
#include "world4d/Plans/MaterialPlan.hpp" // accidental
#include "world4d/Plans/UVPlan.hpp" // accidental
#include "world4d/Plans/ScalePlan.hpp" // accidental
#include "world4d/Plans/MeshPlan.hpp" // accidental

#include "world4d/Entity/Entity.hpp"

W4dDummyFunctionEntityPtrs()
{
    static CtlCountedPtr<W4dEntity> ptr; // subject.hpp
    static CtlConstCountedPtr<W4dEntity> cPtr; // subject.hpp
}
