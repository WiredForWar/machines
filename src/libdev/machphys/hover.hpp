/*
 * H O V E R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPhysHoverBoots

    A brief description of the class should go in here
*/

#ifndef _HOVER_HPP
#define _HOVER_HPP

#include "base/base.hpp"
#include "base/persist.hpp"

#include "machphys/locomoti.hpp"

class W4dLink;
class MachPhysMachine;
class MachPhysMachineMoveInfo;
class MachPhysHoverBootsImpl;

class MachPhysHoverBoots : public MachPhysLocomotionMethod
{
public:
    //  bobHeight is the amount by which the machine will bob up and down.
    //  pBobbingLink is the link to which the bobbing up and down animation
    //  is attached

    MachPhysHoverBoots(
        MachPhysMachine* pMachine,
        W4dLink* pLHoverBoot,
        W4dLink* pRHoverBoot,
        W4dLink* pBobbingLink,
        MATHEX_SCALAR bobHeight);

    ~MachPhysHoverBoots() override;

    MachPhysLocomotionMethod* clone(MachPhysMachine* pMachine, const W4dLinks& links) override;

    void moveAnimations(
        const PhysLinearTravelPlan& linearTravelPlan,
        const TurnAngles& turnAngles,
        const MachPhysMachineMoveInfo& info) override;

    //  Stop the move dead. Apply appropriate animations.
    void stopDead() override;

    //  Revise a movement profile so that hover boot machines
    //  don't tip up when going up slopes.
    void reviseProfile(TransformsPtr transformsPtr) override;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPhysHoverBoots& t);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachPhysHoverBoots);
    PER_FRIEND_READ_WRITE(MachPhysHoverBoots);

private:
    friend class MachPhysHoverBootsImpl;

    // Operation deliberately revoked
    MachPhysHoverBoots(const MachPhysHoverBoots&);

    // Operation deliberately revoked
    MachPhysHoverBoots& operator=(const MachPhysHoverBoots&);

    // Operation deliberately revoked
    bool operator==(const MachPhysHoverBoots&);

    void startBobbing();

    // Callback to do the work of displaying the locomotion animations
    void doFirstPersonMotionAnimations(MachPhysLocomotionMethod::FirstPersonMotionState state);

    MachPhysHoverBootsImpl* pImpl_;
};

PER_DECLARE_PERSISTENT(MachPhysHoverBoots);

#endif

/* End HOVER.HPP ****************************************************/
