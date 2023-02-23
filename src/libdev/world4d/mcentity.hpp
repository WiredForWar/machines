/*
 * M C E N T I T Y . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    W4dMotionControlledEntity

    Adapter class to enable W4dEntity objects to be MotionControlled.
*/

#ifndef _WORLD4D_MCENTITY_HPP
#define _WORLD4D_MCENTITY_HPP

#include "base/base.hpp"
#include "phys/mcmotion.hpp"

class W4dEntity;

class W4dMotionControlledEntity : public PhysMotionControlled
{
public:
    W4dMotionControlledEntity(W4dEntity*);
    ~W4dMotionControlledEntity() override;

    void CLASS_INVARIANT;

    void setPosition(const MexTransform3d&) override;
    void setOrientation(const MexTransform3d&) override;

    MexTransform3d globalTransform() const override;
    void globalTransform(const MexTransform3d&) override;
    MexTransform3d localTransform() const override;
    void localTransform(const MexTransform3d&) override;

private:
    // Operations revoked
    W4dMotionControlledEntity(const W4dMotionControlledEntity&);
    W4dMotionControlledEntity& operator=(const W4dMotionControlledEntity&);
    bool operator==(const W4dMotionControlledEntity&);

    friend ostream& operator<<(ostream& o, const W4dMotionControlledEntity& t);

    // data members
    W4dEntity* pMotionControlledEntity_;
};

#endif

/* End MCENTITY.HPP *************************************************/
