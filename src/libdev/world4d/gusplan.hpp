/*
 * G U S P L A N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    W4dGeneralUniformScalePlan

    A brief description of the class should go in here
*/

#ifndef _WORLD4D_GUSPLAN_HPP
#define _WORLD4D_GUSPLAN_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "mathex/mathex.hpp"
#include "phys/phys.hpp"
#include "render/scale.hpp"
#include "world4d/scalplan.hpp"
#include "phys/sclplan.hpp"
#include "ctl/countptr.hpp"

class W4dGeneralUniformScalePlan : public W4dScalePlan
// Canonical form revoked
{
public:
    using PhysScalarPlanPtr = CtlCountedPtr<PhysScalarPlan>;
    W4dGeneralUniformScalePlan(const PhysScalarPlanPtr& scalarPlanPtr);

    ~W4dGeneralUniformScalePlan() override;

    bool isNonUniform() const override;

    const PhysScalarPlanPtr& physScalarPlan() const;
    void physScalarPlan(const PhysScalarPlanPtr&);
    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const W4dGeneralUniformScalePlan& t);

    PER_MEMBER_PERSISTENT(W4dGeneralUniformScalePlan);
    PER_FRIEND_READ_WRITE(W4dGeneralUniformScalePlan);

private:
    W4dGeneralUniformScalePlan(const W4dGeneralUniformScalePlan&);
    W4dGeneralUniformScalePlan& operator=(const W4dGeneralUniformScalePlan&);
    bool operator==(const W4dGeneralUniformScalePlan&);

    // return the defined uniform scale at timeOffset in pScale
    void doScale(const PhysRelativeTime& timeOffset, RenUniformScale* pScale) const override;

    void doScale(const PhysRelativeTime&, RenNonUniformScale*) const override;
    // Return a new plan allocated on the heap but modified to take account of the offset
    // transform.
    W4dScalePlan* doTransformClone(const MexTransform3d& offsetTransform) const override;

    PhysScalarPlanPtr physScalarPlanPtr_;
};

PER_DECLARE_PERSISTENT(W4dGeneralUniformScalePlan);

#endif

/* End GUSPLAN.HPP **************************************************/
