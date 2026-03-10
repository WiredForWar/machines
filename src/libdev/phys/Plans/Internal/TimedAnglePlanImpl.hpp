/*
 * T I M E A N G I . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PhysTimedAnglePlanImpl

    A brief description of the class should go in here
*/

#ifndef _PHYS_TIMEANGI_HPP
#define _PHYS_TIMEANGI_HPP

#include "base/base.hpp"
#include "base/Persistence.hpp"

#include "mathex/Point3d.hpp"
#include "mathex/Vec3.hpp"
#include "mathex/Radians.hpp"

#include "ctl/Vector.hpp"
#include "ctl/CountedPtr.hpp"

#include "phys/Plans/MotionPlan.hpp"

class PhysTimedAnglePlanImpl
// Canonical form revoked
{
public:
    PER_MEMBER_PERSISTENT(PhysTimedAnglePlanImpl);
    PER_FRIEND_READ_WRITE(PhysTimedAnglePlanImpl);

private:
    PhysTimedAnglePlanImpl(
        const PhysMotionPlan::AnglesPtr& anglesPtr,
        const PhysMotionPlan::TimesPtr& timesPtr,
        const MexVec3& axis,
        const MexVec3& position);

    PhysTimedAnglePlanImpl();
    ~PhysTimedAnglePlanImpl();

    PhysTimedAnglePlanImpl(const PhysTimedAnglePlanImpl&);
    PhysTimedAnglePlanImpl& operator=(const PhysTimedAnglePlanImpl&);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const PhysTimedAnglePlanImpl& t);
    friend class PhysTimedAnglePlan;

    // data members
    MexPoint3d position_; // The constant transform position
    MexVec3 axis_; // The axis of rotation
    PhysMotionPlan::TimesPtr timesPtr_; // times for angles( first angle is at
                                        // time 0 and is not in the collection)
    PhysMotionPlan::AnglesPtr anglesPtr_; // the angles
    size_t cacheIndex_; // Used as quick check into the collections
};

PER_DECLARE_PERSISTENT(PhysTimedAnglePlanImpl);

#endif

/* End TIMEANGI.HPP *************************************************/
