/*
 * M C G R O U N D . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    PhysGroundFlyControl

    Implements the flight control for the disembodied ground level camera
*/

#ifndef _PHYS_MCGROUND_HPP
#define _PHYS_MCGROUND_HPP

#include "base/base.hpp"
#include "phys/mcwtrans.hpp"

#include "gui/gui.hpp"
#include "machgui/cameras.hpp"

// orthodox canonical ( revoked )
class PhysGroundFlyControl : public PhysMotionControlWithTrans
{
public:
    // *All* ctors share these pre- and post-conditions.
    // PRE(PhysMotion::coordinateSystem().isSet()); (from base)
    // PRE( pMotionControlled );
    // POST( metresPerSecond() == 2.0 );
    // POST( degreesPerSecond() == 5.0 );
    // The forwards direction defaults to the +ve x axis.
    PhysGroundFlyControl(PhysMotionControlled*);
    PhysGroundFlyControl(PhysMotionControlled*, const MexVec2& forwards);

    // PRE( pMotionConstraint );
    PhysGroundFlyControl(PhysMotionControlled*, PhysMotionConstraint*);
    // PRE( pMotionConstraint );
    PhysGroundFlyControl(PhysMotionControlled*, PhysMotionConstraint*, const MexVec2& forwards);

    ~PhysGroundFlyControl() override;

    // The speed at which the motionControlled object will accelerate
    UtlProperty<double> metresPerSecond;

    // Controls the rotation acceleration of the motionControlled object
    UtlProperty<MexDegrees> degreesPerSecond;

    void update() override;

    void reversePitchUpDownKeys(bool);
    bool reversePitchUpDownKeys() const;

    void CLASS_INVARIANT;

private:
    // Operations deliberately revoked
    PhysGroundFlyControl(const PhysGroundFlyControl&);
    PhysGroundFlyControl& operator=(const PhysGroundFlyControl&);
    bool operator==(const PhysGroundFlyControl&);

    void setupDefaultKeyboardMapping();
    void updateMotion();
    void ctor();

    // Data members...
    bool reversePitchUpDownKeys_;

    friend void MachCameras::scrollWithWheel(const Gui::ScrollState, const double);
};

class PhysGroundMotionConstraint : public PhysMotionConstraint
{
public:
    PhysGroundMotionConstraint();
    ~PhysGroundMotionConstraint() override { }

    void move(MexTransform3d&, PhysMotion&, double elapsedTime) override;

    // Called when there is a request to move an actor to a new location. Gives the
    // motion constraint the chance to either modify the location slightly or
    // return false if the location is not acceptable.
    bool snapTo(MexPoint3d* location) override;
    bool snapTo(MexTransform3d* trans) override;

    UtlProperty<MATHEX_SCALAR> maxHeight;
    UtlProperty<MATHEX_SCALAR> minHeight;
    UtlProperty<MATHEX_SCALAR> maxSpeed;
    UtlProperty<MATHEX_SCALAR> minSpeed;
    UtlProperty<MexRadians> maxTurnRate;
    UtlProperty<MexRadians> minTurnRate;
    UtlProperty<MexRadians> maxPitchRate;
    UtlProperty<MexRadians> minPitchRate;
    UtlProperty<MexRadians> maxPitch;
    UtlProperty<MexRadians> minPitch;
};

#endif

/* End GRFLYCTL.HPP *************************************************/
