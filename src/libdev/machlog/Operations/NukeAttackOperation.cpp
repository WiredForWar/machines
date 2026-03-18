/*
 * O P N U K E A T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved.
 */

#include <iostream>

#include "phys/ConfigSpace/ConfigSpace2d.hpp"

#include "mathex/ConvexPolygon2d.hpp"
#include "mathex/Vec3.hpp"
#include "mathex/Point3d.hpp"
#include "mathex/AlignedBox3d.hpp"

#include "sim/Manager.hpp"

#include "world4d/Subject/Subject.hpp"
#include "world4d/Entity/Entity.hpp"

#include "phys/Plans/MotionChunk.hpp"

#include "machphys/Weapons/NuclearWeapon.hpp"

#include "machlog/Actors/CanAttack.hpp"
#include "machlog/Actors/Machine.hpp"
#include "machlog/Actors/MotionSequencer.hpp"
#include "machlog/Actors/MissileEmplacement.hpp"
#include "machlog/Operations/NukeAttackOperation.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Combat/NuclearWeapon.hpp"
#include "machlog/Races.hpp"
#include "machlog/Combat/Weapon.hpp"

PER_DEFINE_PERSISTENT(MachLogNukeAttackOperation);

/* //////////////////////////////////////////////////////////////// */
MachLogNukeAttackOperation::MachLogNukeAttackOperation(
    MachLogMissileEmplacement* pActor,
    const MexPoint3d& targetPosition)
    : MachLogOperation("NUKE_ATTACK_OPERATION", MachLogOperation::NUKE_ATTACK_OPERATION)
    , pActor_(pActor)
    , targetPosition_(targetPosition)
{
    ASSERT(pActor_->objectIsCanAttack(), "Object passed to Attack operation is not a CanAttack\n");
}

MachLogNukeAttackOperation::MachLogNukeAttackOperation(MachLogMissileEmplacement* pActor, MachActor* pDirectObject)
    : MachLogOperation("NUKE_ATTACK_OPERATION", MachLogOperation::NUKE_ATTACK_OPERATION)
    , pActor_(pActor)
    , targetPosition_(pDirectObject->position())
{
    ASSERT(pActor_->objectIsCanAttack(), "Object passed to Attack operation is not a CanAttack\n");
}

MachLogNukeAttackOperation::~MachLogNukeAttackOperation()
{
    // deliberately left blank
}

void MachLogNukeAttackOperation::doOutputOperator(std::ostream& o) const
{
    o << "MachLogNukeAttackOperation\n";
}

///////////////////////////////////

bool MachLogNukeAttackOperation::doStart()
{
    return true;
}

/* //////////////////////////////////////////////////////////////// */

PhysRelativeTime MachLogNukeAttackOperation::doUpdate()
{
    PRE(! isFinished());
    PRE(pActor_ != nullptr);
    if (pSubOperation())
    {
        return 0.5;
    }

    PhysRelativeTime interval = 2.0;

    if (pActor_->inAngleRange(targetPosition_))
        pActor_->attack(targetPosition_);
    else
    {
        MexRadians turnBy = pActor_->angleToTurnToFace(targetPosition_);
        subOperation(pActor_, std::make_unique<MachLogTurnAnimation>(pActor_, turnBy));
    }

    return interval;
}

void MachLogNukeAttackOperation::doFinish()
{
    // deliberately blank
}

bool MachLogNukeAttackOperation::doIsFinished() const
{
    bool result = !(const_cast<const MachLogMissileEmplacement&>(*pActor_).weapons().front()->recharged());

    return result;
}

bool MachLogNukeAttackOperation::doBeInterrupted()
{
    return true;
}

/////////////////////////////////////////////////// persistence ////////////////////////////////////////////////////

void perWrite(PerOstream& ostr, const MachLogNukeAttackOperation& op)
{
    const MachLogOperation& base1 = op;

    ostr << base1;
    ostr << op.pActor_;
    ostr << op.targetPosition_;
}

void perRead(PerIstream& istr, MachLogNukeAttackOperation& op)
{
    MachLogOperation& base1 = op;

    istr >> base1;
    istr >> op.pActor_;
    istr >> op.targetPosition_;
}

MachLogNukeAttackOperation::MachLogNukeAttackOperation(PerConstructor con)
    : MachLogOperation(con)
{
}

/* End OPNUKEAT.CPP *************************************************/
