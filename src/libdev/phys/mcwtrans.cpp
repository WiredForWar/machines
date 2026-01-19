/*
 * M C W T R A N S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "phys/mcwtrans.hpp"
#include "device/keytrans.hpp"
#include "mathex/vec2.hpp"
#include "mathex/point3d.hpp"
#include "mathex/eulerang.hpp"

#include "machgui/IInputRegistry.hpp"

namespace
{

PhysMotionConstraint* GetUnconstrainedMotion()
{
    static PhysUnconstrainedMotion motion;
    return &motion;
}

} // namespace

PhysMotionControlWithTrans::PhysMotionControlWithTrans(std::unique_ptr<PhysMotionControlled> target, const MexVec2& forwards)
    : PhysMotionControl(std::move(target))
    , pKeyTranslator_(std::make_unique<DevKeyToCommandTranslator>())
    , pMotionConstraint_(GetUnconstrainedMotion())
    , motion_(forwards)
{
    commandList_.reserve(NUM_COMMANDS);

    for (size_t i = 0; i < NUM_COMMANDS; ++i)
    {
        commandList_.push_back(DevKeyToCommandTranslator::Command());
    }

    TEST_INVARIANT;
}

PhysMotionControlWithTrans::~PhysMotionControlWithTrans() = default;

void PhysMotionControlWithTrans::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void PhysMotionControlWithTrans::disableInput()
{
    PhysMotionControl::disableInput();

    pKeyTranslator_->resetCommands(&commandList_, true);
}

void PhysMotionControlWithTrans::resetCommands()
{
    pKeyTranslator_->resetCommands(&commandList_);
}

void PhysMotionControlWithTrans::initEventQueue()
{
    pKeyTranslator_->initEventQueue();
}

void PhysMotionControlWithTrans::forceCommandOn(Command commandId)
{
    PRE(commandId < commandList_.size());

    commandList_[commandId].forceOn();
}

bool PhysMotionControlWithTrans::isCommandOn(Command commandId) const
{
    PRE(commandId < commandList_.size());
    return commandList_[commandId].on();
}

// virtual
bool PhysMotionControlWithTrans::doProcessButtonEvent(const DevButtonEvent& buttonEvent)
{
    return pKeyTranslator_->translate(buttonEvent, &commandList_);
}

void PhysMotionControlWithTrans::addBind(Command command, MachGui::BindId bindId)
{
    const auto& trigger = MachGui::inputRegistry()->getBinds(bindId);
    pKeyTranslator_->addTranslation(DevKeyToCommand(command, &trigger));
}

void PhysMotionControlWithTrans::setConstraint(PhysMotionConstraint* pMotionConstraint)
{
    PRE(pMotionConstraint);

    pMotionConstraint_ = pMotionConstraint;
}

// virtual
void PhysMotionControlWithTrans::snapTo(const MexPoint3d& location)
{
    PRE(pMotionConstraint_);

    MexPoint3d newLocation(location);

    if (pMotionConstraint_->snapTo(&newLocation))
    {
        // Snap to new location but retain old orientation
        MexTransform3d oldTrans = pMotionControlled_->globalTransform();
        MexEulerAngles oldAngles;
        oldTrans.rotation(&oldAngles);
        pMotionControlled_->globalTransform(MexTransform3d(oldAngles, newLocation));
    }

    // Stop all motion
    motion_.climb(0.0);
    motion_.speed(0.0);
    motion_.drift(0.0);
    motion_.heading(0.0);
    motion_.pitch(0.0);
    motion_.roll(0.0);

    // Begin timing the next frame.
    frameTimer_.time(0);
}

// virtual
void PhysMotionControlWithTrans::snapTo(const MexTransform3d& trans)
{
    PRE(pMotionConstraint_);

    MexTransform3d newTrans(trans);

    if (pMotionConstraint_->snapTo(&newTrans))
    {
        pMotionControlled_->globalTransform(newTrans);
    }

    // Stop all motion
    motion_.climb(0.0);
    motion_.speed(0.0);
    motion_.drift(0.0);
    motion_.heading(0.0);
    motion_.pitch(0.0);
    motion_.roll(0.0);

    // Begin timing the next frame.
    frameTimer_.time(0);
}

std::ostream& operator<<(std::ostream& o, const PhysMotionControlWithTrans& t)
{

    o << "PhysMotionControlWithTrans " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "PhysMotionControlWithTrans " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End MCWTRANS.CPP *************************************************/
