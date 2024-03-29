/*
 * P 1 M C D R I V . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhys1stPersonMachineDriver

    Phsyical driver for a machine in 1st person
*/

#ifndef _MACHPHYS_P1MCDRIV_HPP
#define _MACHPHYS_P1MCDRIV_HPP

#include "base/base.hpp"
#include "mathex/mathex.hpp"
#include "machphys/p1driver.hpp"

class MexTransform3d;
class MachPhysMachine;
class MachPhysPlanetSurface;

class MachPhys1stPersonMachineDriver : public MachPhys1stPersonDriver
// Canonical form revoked
{
public:
    // ctor specifies machine being driven (including view of as can attack - maybe NULL)
    // and the planet surface to drive over.
    // If remoteNode is true, this is being used to echo 1st person from another node.
    MachPhys1stPersonMachineDriver(
        MachPhysMachine* pMachine,
        MachPhysCanAttack* pCanAttack,
        const MachPhysPlanetSurface& planetSurface,
        bool remoteNode);

    ~MachPhys1stPersonMachineDriver() override;

    ///////////////////////////////////////////////////////////
    // Inherited from MachPhys1stPersonDriver

    // Returns the entity to become the camera parent, and the local transform to give it
    // in pOffsetTransform
    W4dEntity& cameraAttachment(MexTransform3d* pOffsetTransform) const override;

    // Make the entity jump to the new position specified by transform, with
    // forward/backward signed speed at specified time.
    void snapTo(const MexTransform3d& transform, MATHEX_SCALAR speed, const PhysAbsoluteTime& time) override;

    ////////////
    // Head turning controls
    bool canTurnHead() const override;

    PhysRelativeTime turnHeadBy(MexRadians angle) override;
    // PRE( canTurnHead() );

    PhysRelativeTime turnHeadTo(MexRadians angle) override;
    // PRE( canTurnHead() );

    MexRadians currentHeadAngle() const override;

    void snapHeadTo(MexRadians angle, const PhysAbsoluteTime& time) override;
    // PRE( canTurnHead() );

    // The angle trying to move to
    MexRadians targetHeadAngle() const override;

    ////////////

    // Return speed currently moving at
    MATHEX_SCALAR currentSpeed() const override;

    ///////////////////////////////////////////////////////////

    // Compute the machine's next position based on state vector and time elapsed since last update.
    // True if position would change. If so returns new global transform in pNextPosition.
    bool nextPosition(MexTransform3d* pNextPosition);

    // Return in pTransform the actual position for the machine with its origin at location, and
    // facing the direction defined by xBasis.
    void surfacePosition(const MexPoint2d& location, const MexVec3& xBasis, MexTransform3d* pTransform);

    // Time last updated
    const PhysAbsoluteTime& lastUpdateTime() const;

    // Makes it stop dead at last position
    void stopDead();

    // get the machine's locomotion animations correct
    void updateLocomotionAnimations();

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachPhys1stPersonMachineDriver& t);

    // revoked
    MachPhys1stPersonMachineDriver(const MachPhys1stPersonMachineDriver&);
    MachPhys1stPersonMachineDriver& operator=(const MachPhys1stPersonMachineDriver&);

    // data members
    MachPhysMachine* pMachine_; // the machine being controlled
    const MachPhysPlanetSurface* pPlanetSurface_; // the surface over which the machine is driving
    MATHEX_SCALAR lastSpeed_; // Speed used on last update (signed for reversing context)
    PhysAbsoluteTime lastUpdateTime_; // Time position last updated
};

#endif

/* End P1MCDRIV.HPP *************************************************/
