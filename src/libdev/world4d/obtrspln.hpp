/*
 * O B T R S P L N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    W4dObjectTrackerScalePlan

    A non-uniform scale plan using a W4dObjectTracker object
*/

#ifndef _WORLD4D_OBTRSPLN_HPP
#define _WORLD4D_OBTRSPLN_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "world4d/scalplan.hpp"
#include "world4d/objtrack.hpp"

class W4dObjectTrackerScalePlan : public W4dScalePlan
// Canonical form revoked
{
public:
    // ctor.
    W4dObjectTrackerScalePlan(W4dObjectTrackerPtr objectTrackerPtr, const PhysRelativeTime& duration);

    // dtor
    ~W4dObjectTrackerScalePlan() override;

    //////////////////////////////////////////////////
    // Inherited from PhysScalePlan

    // true iff the scale being applied is non-uniform
    bool isNonUniform() const override;
    //////////////////////////////////////////////////

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const W4dObjectTrackerScalePlan& t);

    PER_MEMBER_PERSISTENT_VIRTUAL(W4dObjectTrackerScalePlan);
    PER_FRIEND_READ_WRITE(W4dObjectTrackerScalePlan);

private:
    W4dObjectTrackerScalePlan(const W4dObjectTrackerScalePlan&);
    W4dObjectTrackerScalePlan& operator=(const W4dObjectTrackerScalePlan&);
    bool operator==(const W4dObjectTrackerScalePlan&);

    //////////////////////////////////////////////////
    // Inherited from PhysScalePlan

    // Illegal to call
    void doScale(const PhysRelativeTime& timeOffset, RenUniformScale* pScale) const override;

    // return the defined non-uniform scale at timeOffset in pScale
    void doScale(const PhysRelativeTime& timeOffset, RenNonUniformScale* pScale) const override;

    // Return a new plan allocated on the heap but modified to take account of the offset
    // transform.
    W4dScalePlan* doTransformClone(const MexTransform3d& offsetTransform) const override;
    //////////////////////////////////////////////////

    // data members
    W4dObjectTrackerPtr objectTrackerPtr_;
};

PER_DECLARE_PERSISTENT(W4dObjectTrackerScalePlan);

#endif

/* End OBTRSPLN.HPP *************************************************/
