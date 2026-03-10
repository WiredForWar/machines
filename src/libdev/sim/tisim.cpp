/*
 * T I S I M . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#define _INSTANTIATE_TEMPLATE_CLASSES
#include "ctl/Deque.hpp"
#undef _INSTANTIATE_TEMPLATE_CLASSES

#include "phys/Plans/RampAcceleration.hpp"

#include "ctl/Vector.hpp"
#include "ctl/Vector.ctp"

#include "ctl/CountedPtr.hpp"
#include "ctl/CountedPtr.ctp"

#include "mathex/Transform3d.hpp"
#include "phys/Plans/MotionPlan.hpp"
#include "render/Mesh.hpp"
#include "render/MaterialVec.hpp"
#include "render/UVTransform.hpp"
#include "world4d/Plans/MeshPlan.hpp"
#include "world4d/Plans/MaterialPlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/ScalePlan.hpp"
#include "world4d/Plans/UVPlan.hpp"
#include "world4d/Entity/CompositePlan.hpp"
#include "world4d/Entity/Entity.hpp"

#include "sim/DiscreteEvent.hpp"
#include "sim/EventDiary.hpp"
#include "sim/Process.hpp"
#include "sim/sim.hpp"
#include "internal/ManagerImpl.hpp"

void SimDummyFunction1()
{
    static SimDiscreteEventPtr eventPtr;
    static SimEventDiary::SimEventQueue eventQueue;
    static SimManagerImpl::SimProcesses processes;
    static SimActors actors;
    static CtlConstCountedPtr<W4dEntity> dummy7;
}

/* End TISIM.CPP *****************************************************/
