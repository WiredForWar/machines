/*
 * T I S I M . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#define _INSTANTIATE_TEMPLATE_CLASSES
#include "ctl/deque.hpp"
#undef _INSTANTIATE_TEMPLATE_CLASSES

#include "phys/Plans/RampAcceleration.hpp"

#include "ctl/vector.hpp"
#include "ctl/vector.ctp"

#include "ctl/countptr.hpp"
#include "ctl/countptr.ctp"

#include "mathex/transf3d.hpp"
#include "phys/Plans/MotionPlan.hpp"
#include "render/mesh.hpp"
#include "render/matvec.hpp"
#include "render/uvanim.hpp"
#include "world4d/Plans/MeshPlan.hpp"
#include "world4d/Plans/MaterialPlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/ScalePlan.hpp"
#include "world4d/Plans/UVPlan.hpp"
#include "world4d/Entity/CompositePlan.hpp"
#include "world4d/Entity/Entity.hpp"

#include "sim/disevent.hpp"
#include "sim/evdiary.hpp"
#include "sim/process.hpp"
#include "sim/sim.hpp"
#include "internal/manageri.hpp"

void SimDummyFunction1()
{
    static SimDiscreteEventPtr eventPtr;
    static SimEventDiary::SimEventQueue eventQueue;
    static SimManagerImpl::SimProcesses processes;
    static SimActors actors;
    static CtlConstCountedPtr<W4dEntity> dummy7;
}

/* End TISIM.CPP *****************************************************/
