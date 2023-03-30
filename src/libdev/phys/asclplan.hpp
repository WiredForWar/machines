/*
 * A S C L P L A N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PhysAcceleratedScalarPlan is
    a derived class from PhysScalarPlan. the scalar -time  relation is defined by
    RampAccelerations

*/

#ifndef _PHYS_ASCLPLAN_HPP
#define _PHYS_ASCLPLAN_HPP

#include "base/base.hpp"
#include "phys/sclplan.hpp"

class PhysRampAcceleration;
class PhysAcceleratedScalarPlanImpl;

template <class T> class ctl_vector;

class PhysAcceleratedScalarPlan : public PhysScalarPlan
// Canonical form revoked
{
public:
    using RampAccelerations = ctl_vector<PhysRampAcceleration>;
    using Distances = ctl_vector<MATHEX_SCALAR>;

    PhysAcceleratedScalarPlan(const RampAccelerations& rampAccelerations, MATHEX_SCALAR scale);
    PhysAcceleratedScalarPlan(const PhysAcceleratedScalarPlan&);

    ~PhysAcceleratedScalarPlan() override;

    const RampAccelerations& rampAccelerations() const;
    const Distances& distances() const;
    const MATHEX_SCALAR& initialScale() const;

    void initialScale(const MATHEX_SCALAR&);

    //  returns the scale value at timeOffset
    MATHEX_SCALAR scalar(const PhysRelativeTime& timeOffset) const override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const PhysAcceleratedScalarPlan& t);

private:
    PhysAcceleratedScalarPlan& operator=(const PhysAcceleratedScalarPlan&);
    bool operator==(const PhysAcceleratedScalarPlan&);

    PhysAcceleratedScalarPlanImpl* pImpl_;
};

#endif

/* End ASCLPLAN.HPP *************************************************/
