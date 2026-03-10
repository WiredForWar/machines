/*
 * A C C E T U M I . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PhysAccelerateTumblePlanImp

    A brief description of the class should go in here
*/

#ifndef _PHYS_ACCETUMI_HPP
#define _PHYS_ACCETUMI_HPP

#include "base/base.hpp"
#include "base/Persistence.hpp"

#include "mathex/mathex.hpp"
#include "mathex/Vec3.hpp"
#include "mathex/EulerAngles.hpp"

#include "phys/Plans/EulerTransform3d.hpp"
#include "phys/Plans/MotionPlan.hpp"
#include "phys/Plans/AccelerateTumblePlan.hpp"

#include "ctl/CountedPtr.hpp"
#include "ctl/Vector.hpp"

class PhysAccelerateTumblePlanImpl
// Canonical form revoked
{
public:
    PER_MEMBER_PERSISTENT(PhysAccelerateTumblePlanImpl);
    PER_FRIEND_READ_WRITE(PhysAccelerateTumblePlanImpl);

private:
    friend class PhysAccelerateTumblePlan;

    PhysAccelerateTumblePlanImpl(
        const PhysAccelerateTumblePlan::EulerTransformsPtr& transformsPtr,
        const PhysMotionPlan::TimesPtr& segmentTimesPtr,
        const MexVec3& acceleration);

    ~PhysAccelerateTumblePlanImpl();

    void CLASS_INVARIANT;

    PhysAccelerateTumblePlanImpl(const PhysAccelerateTumblePlanImpl&);
    PhysAccelerateTumblePlanImpl& operator=(const PhysAccelerateTumblePlanImpl&);

    friend std::ostream& operator<<(std::ostream& o, const PhysAccelerateTumblePlanImpl& t);

    PhysAccelerateTumblePlan::EulerTransformsPtr transformsPtr_;
    PhysMotionPlan::TimesPtr segmentTimesPtr_;
    MexVec3 acceleration_;

    //  Cached data - updated as we change segments
    size_t currentSegmentIndex_;
    MexEulerAngles deltaOrientation_;
    MexEulerAngles initialOrientation_;
    MexVec3 initialVelocity_;
};

PER_DECLARE_PERSISTENT(PhysAccelerateTumblePlanImpl);

#endif

/* End ACCETUMI.HPP *************************************************/
