/*
 * M C F L Y . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include <iostream>
#include <iomanip>
#include <algorithm>

#include "base/diag.hpp"

#include "mathex/eulerang.hpp"
#include "mathex/point3d.hpp"
#include "mathex/vec3.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/quatern.hpp"
#include "device/keyboard.hpp"
#include "phys/MotionControl/FlyControl.hpp"

PhysFlyControl::PhysFlyControl(std::unique_ptr<PhysMotionControlled> target, const MexVec2& forwards)
    : PhysMotionControlWithTrans(std::move(target), forwards)
{
    ctor();
}

// Do the initialisation work common to all the constructors.
void PhysFlyControl::ctor()
{
    metresPerSecond(8.0);
    degreesPerSecond(3.0);

    // Two new commands required for motion/rotation stop
    commandList_.push_back(DevKeyToCommandTranslator::Command());
    commandList_.push_back(DevKeyToCommandTranslator::Command());

    useDefaultKeyboardMapping();
}

PhysFlyControl::~PhysFlyControl()
{
}

void PhysFlyControl::useDefaultKeyboardMapping()
{
    addBind(Command::FOWARD, "free-camera-forward"_bind);
    addBind(Command::BACKWARD, "free-camera-backward"_bind);
    addBind(Command::SLIDE_LEFT, "free-camera-slide-left"_bind);
    addBind(Command::SLIDE_RIGHT, "free-camera-slide-right"_bind);
    addBind(Command::ROTATE_LEFT, "free-camera-rotate-left"_bind);
    addBind(Command::ROTATE_RIGHT, "free-camera-rotate-right"_bind);
    addBind(Command::UP, "free-camera-up"_bind);
    addBind(Command::DOWN, "free-camera-down"_bind);
    addBind(Command::PITCH_UP, "free-camera-pitch-up"_bind);
    addBind(Command::PITCH_DOWN, "free-camera-pitch-down"_bind);
    addBind(Command::ROLL_LEFT, "free-camera-roll-left"_bind);
    addBind(Command::ROLL_RIGHT, "free-camera-roll-right"_bind);

    addBind(Command::RESET_POS, "free-camera-reset-position"_bind);
    addBind(Command::RESET_ORIENTATION, "free-camera-reset-orientation"_bind);

    addBind(Command::STOP_MOVING, "free-camera-stop-moving"_bind);
    addBind(Command::STOP_ROTATING, "free-camera-stop-rotating"_bind);
}

void PhysFlyControl::updateMotion()
{
    if (inputEnabled())
    {
        double elapsedTime = keyTimer_.time() * 10.0;

        if (commandList_[STOP_MOVING].on())
            stopMoving();
        if (commandList_[STOP_ROTATING].on())
            stopRotating();

        if (commandList_[PhysMotionControlWithTrans::FOWARD].on())
            motion_.deltaSpeed(metresPerSecond() * elapsedTime);
        if (commandList_[PhysMotionControlWithTrans::BACKWARD].on())
            motion_.deltaSpeed(-metresPerSecond() * elapsedTime);

        if (commandList_[PhysMotionControlWithTrans::SLIDE_RIGHT].on())
            motion_.deltaDrift(metresPerSecond() * elapsedTime);
        if (commandList_[PhysMotionControlWithTrans::SLIDE_LEFT].on())
            motion_.deltaDrift(-metresPerSecond() * elapsedTime);

        if (commandList_[PhysMotionControlWithTrans::DOWN].on())
            motion_.deltaClimb(-metresPerSecond() * elapsedTime);
        if (commandList_[PhysMotionControlWithTrans::UP].on())
            motion_.deltaClimb(metresPerSecond() * elapsedTime);

        if (commandList_[PhysMotionControlWithTrans::ROTATE_RIGHT].on())
            motion_.deltaHeading(radiansPerSecond() * elapsedTime);
        if (commandList_[PhysMotionControlWithTrans::ROTATE_LEFT].on())
            motion_.deltaHeading(-radiansPerSecond() * elapsedTime);

        if (commandList_[PhysMotionControlWithTrans::PITCH_DOWN].on())
            motion_.deltaPitch(-radiansPerSecond() * elapsedTime);
        if (commandList_[PhysMotionControlWithTrans::PITCH_UP].on())
            motion_.deltaPitch(radiansPerSecond() * elapsedTime);

        if (commandList_[PhysMotionControlWithTrans::ROLL_LEFT].on())
            motion_.deltaRoll(-radiansPerSecond() * elapsedTime);
        if (commandList_[PhysMotionControlWithTrans::ROLL_RIGHT].on())
            motion_.deltaRoll(radiansPerSecond() * elapsedTime);

        if (commandList_[PhysMotionControlWithTrans::RESET_POS].on())
            resetPosition();

        if (commandList_[PhysMotionControlWithTrans::RESET_ORIENTATION].on())
            resetOrientation();
    }
    keyTimer_.time(0);
}

// Look for keypresses and modify motion as appropriate.
// virtual
void PhysFlyControl::update()
{
    if (motionFrozen())
        return;

    updateMotion();

    // Allow the motion constraint to modify our movement for collision
    // detection etc.
    const double elapsed = frameTimer_.time();

    MexTransform3d xform = pMotionControlled_->globalTransform();
    pMotionConstraint_->move(xform, motion_, elapsed);
    pMotionControlled_->globalTransform(xform);

    IAIN_STREAM(std::setprecision(3) << std::setw(4) << motion_ << "\n");
    IAIN_STREAM("frame time=" << elapsed << "\n");

    resetCommands();

    // Begin timing the next frame.
    frameTimer_.time(0);
}

void PhysFlyControl::stopMoving()
{
    motion_.stopMoving();
}

void PhysFlyControl::stopRotating()
{
    motion_.stopRotating();
}

double PhysFlyControl::radiansPerSecond() const
{
    return degreesPerSecond() / 180 * Mathex::PI;
}
