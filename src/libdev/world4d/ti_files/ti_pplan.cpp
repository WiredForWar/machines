#include "ctl/CountedPtr.hpp"
#include "ctl/CountedPtr.ctp"

#include "ctl/List.hpp"
#include "ctl/List.ctp"

// The files marked "accidental" shouldn't be necessary.  They must be included
// because Watcom is trying to instantiate everything it sees.
#include "mathex/Transform3d.hpp" // accidental
#include "phys/Plans/RampAcceleration.hpp" // accidental
#include "phys/Plans/MotionPlan.hpp" // accidental
#include "render/Mesh.hpp" // accidental
#include "render/MaterialVec.hpp" // accidental
#include "render/UVTransform.hpp" // accidental

#include "world4d/Entity/CompositePlan.hpp"
#include "world4d/Plans/FloatValuePlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/MaterialPlan.hpp"
#include "world4d/Plans/UVPlan.hpp"
#include "world4d/Plans/ScalePlan.hpp"
#include "world4d/Plans/MeshPlan.hpp"

#include "internal/pendplan.hpp"

typedef W4dPendingPlan<PhysMotionPlanPtr> PendingMotionPlan;
typedef W4dPendingPlan<W4dVisibilityPlanPtr> PendingVisibilityPlan;
typedef W4dPendingPlan<W4dMeshPlanPtr> PendingMeshPlan;
typedef W4dPendingPlan<W4dScalePlanPtr> PendingScalePlan;
typedef W4dPendingPlan<W4dUVPlanPtr> PendingUVPlan;
typedef W4dPendingPlan<W4dMaterialPlanPtr> PendingMaterialPlan;

typedef ctl_list<PendingMotionPlan> PendingMotionPlans;
typedef ctl_list<PendingMeshPlan> PendingMeshPlans;
typedef ctl_list<PendingVisibilityPlan> PendingVisibilityPlans;
typedef ctl_list<PendingScalePlan> PendingScalePlans;
typedef ctl_list<PendingUVPlan> PendingUVPlans;
typedef ctl_list<PendingMaterialPlan> PendingMaterialPlans;

W4dDummyFunctionPlanPtrs()
{
    static CtlCountedPtr<W4dCompositePlan> dummy3;
    static CtlCountedPtr<W4dFloatValuePlan> dummy4;
    static CtlCountedPtr<W4dVisibilityPlan> dummy2;
    static CtlCountedPtr<W4dMaterialPlan> dummy1;
    static CtlCountedPtr<W4dUVPlan> dummy5;
    static CtlCountedPtr<W4dScalePlan> dummy7;
    static CtlCountedPtr<W4dMeshPlan> dummy6;

    static PendingMotionPlans absoluteMotionPlans_;
    static PendingMeshPlans meshPlans_;
    static PendingVisibilityPlans visibilityPlans_;
    static PendingScalePlans scalePlans_;
    static PendingUVPlans uvPlans_;
    static PendingMaterialPlans materialPlans_;
}
