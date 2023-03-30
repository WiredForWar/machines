/*
 * S I M P L S C A . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    W4dSimpleUniformScalePlan
    W4dSimpleNonUniformScalePlan

    Simple scale plan classes defining scale as function of time.
*/

#ifndef _WORLD4D_SIMPLSCA_HPP
#define _WORLD4D_SIMPLSCA_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "mathex/mathex.hpp"
#include "phys/phys.hpp"
#include "render/scale.hpp"
#include "world4d/scalplan.hpp"

// Forward refs

// orthodox canonical( revoked );
class W4dSimpleUniformScalePlan : public W4dScalePlan
{
public:
    // scales uniformly from startScale to endScale over the duration
    W4dSimpleUniformScalePlan(MATHEX_SCALAR startScale, MATHEX_SCALAR endScale, const PhysRelativeTime& duration);

    ~W4dSimpleUniformScalePlan() override;

    // Inherited from W4dScalePlan
    bool isNonUniform() const override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const W4dSimpleUniformScalePlan& t);

    PER_MEMBER_PERSISTENT_VIRTUAL(W4dSimpleUniformScalePlan);
    PER_FRIEND_READ_WRITE(W4dSimpleUniformScalePlan);

private:
    // Operation deliberately revoked
    W4dSimpleUniformScalePlan(const W4dSimpleUniformScalePlan&);
    W4dSimpleUniformScalePlan& operator=(const W4dSimpleUniformScalePlan&);
    bool operator==(const W4dSimpleUniformScalePlan&);

    //////////////////////////////////////////////////
    // Inherited from W4dScalePlan
    void doScale(const PhysRelativeTime& timeOffset, RenUniformScale* pScale) const override;

    // Illegal
    void doScale(const PhysRelativeTime& timeOffset, RenNonUniformScale* pScale) const override;

    // Illegal
    W4dScalePlan* doTransformClone(const MexTransform3d& offsetTransform) const override;
    //////////////////////////////////////////////////

    // data members
    MATHEX_SCALAR startScale_;
    MATHEX_SCALAR endScale_;
};

PER_DECLARE_PERSISTENT(W4dSimpleUniformScalePlan);

// orthodox canonical( revoked );
class W4dSimpleNonUniformScalePlan : public W4dScalePlan
{
public:
    // scales uniformly from startScale to endScale over the duration
    W4dSimpleNonUniformScalePlan(
        const RenNonUniformScale& startScale,
        const RenNonUniformScale& endScale,
        const PhysRelativeTime& duration);

    ~W4dSimpleNonUniformScalePlan() override;

    // Inherited from W4dScalePlan
    bool isNonUniform() const override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const W4dSimpleUniformScalePlan& t);

    PER_MEMBER_PERSISTENT_VIRTUAL(W4dSimpleNonUniformScalePlan);
    PER_FRIEND_READ_WRITE(W4dSimpleNonUniformScalePlan);

private:
    // Operation deliberately revoked
    W4dSimpleNonUniformScalePlan(const W4dSimpleNonUniformScalePlan&);
    W4dSimpleNonUniformScalePlan& operator=(const W4dSimpleNonUniformScalePlan&);
    bool operator==(const W4dSimpleNonUniformScalePlan&);

    //////////////////////////////////////////////////
    // Inherited from W4dScalePlan
    void doScale(const PhysRelativeTime& timeOffset, RenNonUniformScale* pScale) const override;

    // Illegal
    void doScale(const PhysRelativeTime& timeOffset, RenUniformScale* pScale) const override;

    // Return a new plan allocated on the heap but modified to take account of the offset
    // transform.
    W4dScalePlan* doTransformClone(const MexTransform3d& offsetTransform) const override;
    //////////////////////////////////////////////////

    // data members
    RenNonUniformScale startScale_;
    RenNonUniformScale endScale_;
};

PER_DECLARE_PERSISTENT(W4dSimpleNonUniformScalePlan);

#endif

/* End SIMPLSCA.HPP *************************************************/
