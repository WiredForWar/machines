/*
 * F W R D P L A N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#ifndef _WORLD4D_FWRDPLAN_HPP
#define _WORLD4D_FWRDPLAN_HPP

// The miniumum forward declarations required for all plan pointers.
// These are required in more than one header file, so they're grouped
// together simply to avoid duplicating code.
template <class T> class CtlCountedPtr;

class PhysMotionPlan;
class W4dMeshPlan;
class W4dMaterialPlan;
class W4dVisibilityPlan;
class W4dScalePlan;
class W4dUVPlan;
class W4dCompositePlan;

using W4dMeshPlanPtr = CtlCountedPtr<W4dMeshPlan>;
using W4dMaterialPlanPtr = CtlCountedPtr<W4dMaterialPlan>;
using W4dVisibilityPlanPtr = CtlCountedPtr<W4dVisibilityPlan>;
using W4dScalePlanPtr = CtlCountedPtr<W4dScalePlan>;
using W4dUVPlanPtr = CtlCountedPtr<W4dUVPlan>;
using W4dCompositePlanPtr = CtlCountedPtr<W4dCompositePlan>;
using PhysMotionPlanPtr = CtlCountedPtr<PhysMotionPlan>;

#endif

/* End FWRDPLAN.HPP *************************************************/
