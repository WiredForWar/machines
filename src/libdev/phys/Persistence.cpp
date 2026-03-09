/*
 * P E R S I S T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "phys/Persistence.hpp"
#include "phys/Plans/AccelerateTumblePlan.hpp"
#include "phys/Plans/LinearMotionPlan.hpp"
#include "phys/Plans/LinearTravelPlan.hpp"
#include "phys/Plans/TimedAnglePlan.hpp"
#include "phys/Plans/TimedSpinPlan.hpp"
#include "phys/Plans/MoveSpinPlan.hpp"

// static
PhysPersistence& PhysPersistence::instance()
{
    static PhysPersistence instance_;
    return instance_;
}

PhysPersistence::PhysPersistence()
{
    PER_REGISTER_DERIVED_CLASS(PhysLinearTravelPlan);
    PER_REGISTER_DERIVED_CLASS(PhysLinearMotionPlan);
    PER_REGISTER_DERIVED_CLASS(PhysAccelerateTumblePlan);
    PER_REGISTER_DERIVED_CLASS(PhysTimedAnglePlan);
    PER_REGISTER_DERIVED_CLASS(PhysTimedSpinPlan);
    PER_REGISTER_DERIVED_CLASS(PhysMoveSpinPlan);

    TEST_INVARIANT;
}

PhysPersistence::~PhysPersistence()
{
    TEST_INVARIANT;
}

void PhysPersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const PhysPersistence& t)
{

    o << "PhysPersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "PhysPersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End PERSIST.CPP **************************************************/
