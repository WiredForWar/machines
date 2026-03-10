/*
 * O P P O D A T T . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#include <iostream>

#include "ctl/PtrVector.hpp"

#include "mathex/ConvexPolygon2d.hpp"
#include "mathex/Vec3.hpp"
#include "mathex/Point3d.hpp"
#include "mathex/AlignedBox3d.hpp"

#include "sim/Manager.hpp"

#include "world4d/Subject/Subject.hpp"
#include "world4d/Entity/Entity.hpp"

#include "phys/ConfigSpace/ConfigSpace2d.hpp"
#include "phys/Plans/MotionChunk.hpp"

#include "machphys/Machines/MachineMoveInfo.hpp"
#include "machphys/Constructions/Pod.hpp"

#include "machlog/Actors/CanAttack.hpp"
#include "machlog/Actors/Machine.hpp"
#include "machlog/Actors/MotionSequencer.hpp"
#include "machlog/Messaging/MessageBroker.hpp"
#include "machlog/Messaging/Network.hpp"
#include "machlog/Operations/PodAttackOperation.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Actors/Pod.hpp"
#include "machlog/Races.hpp"
#include "machlog/Messaging/VoiceMailData.hpp"
#include "machlog/Messaging/VoiceMailManager.hpp"
#include "machlog/Combat/Weapon.hpp"

PER_DEFINE_PERSISTENT(MachLogPodAttackOperation);
PER_DEFINE_PERSISTENT(MachLogPodAttackAnimation);

/* //////////////////////////////////////////////////////////////// */
MachLogPodAttackOperation::MachLogPodAttackOperation(MachLogPod* pActor, const MexPoint3d& targetPosition)
    : MachLogOperation("POD_ATTACK_OPERATION", MachLogOperation::POD_ATTACK_OPERATION)
    , pActor_(pActor)
    , targetPosition_(targetPosition)
{
    ASSERT(pActor_->objectIsCanAttack(), "Object passed to Attack operation is not a CanAttack\n");
    dealWithVoiceMails();
}

MachLogPodAttackOperation::MachLogPodAttackOperation(MachLogPod* pActor, MachActor* pDirectObject)
    : MachLogOperation("POD_ATTACK_OPERATION", MachLogOperation::POD_ATTACK_OPERATION)
    , pActor_(pActor)
    , targetPosition_(pDirectObject->position())
{
    ASSERT(pActor_->objectIsCanAttack(), "Object passed to Attack operation is not a CanAttack\n");
    dealWithVoiceMails();
}

MachLogPodAttackOperation::~MachLogPodAttackOperation()
{
    // deliberately left blank
}

void MachLogPodAttackOperation::dealWithVoiceMails()
{
    // now for the voicemail alerts - on this node if this is an AI race
    // send voicemail if it wasn't me initiating this launch and I have the capability to detect it

    if (MachLogRaces::instance().playerRace() != pActor_->race())
    {
        MachLogVoiceMailManager::instance().postNewMail(
            VID_POD_ENEMY_ION_CANNON,
            MachLogRaces::instance().playerRace());
    }

    // ........and whizz the warning round the network if it's a network game.
    if (MachLogNetwork::instance().isNetworkGame())
        MachLogNetwork::instance().messageBroker().sendWeaponInformationMessage(
            MachLogMessageBroker::ION_CANNON_FIRED,
            pActor_->race());
}

void MachLogPodAttackOperation::doOutputOperator(std::ostream& o) const
{
    o << "MachLogPodAttackOperation\n";
}

///////////////////////////////////

bool MachLogPodAttackOperation::doStart()
{
    return true;
}

MexRadians angleToTurnToFace(const MachLogPod& actor, const MexPoint3d& pos);

/* //////////////////////////////////////////////////////////////// */

PhysRelativeTime MachLogPodAttackOperation::doUpdate()
{
    PRE(! isFinished());
    PRE(pActor_ != nullptr);
    if (pSubOperation())
    {
        return 0.5;
    }

    PhysRelativeTime interval = 0.1;

    /*
    //The owning aggressor is busy shooting something else right now.
    if( pActor_->hasCurrentTarget() and &pActor_->currentTarget() != &directObject() )
    {
        return 0.5;
    }
    */

    subOperation(pActor_, std::make_unique<MachLogPodAttackAnimation>(pActor_, targetPosition_));

    return interval;
}

void MachLogPodAttackOperation::doFinish()
{
}

bool MachLogPodAttackOperation::doIsFinished() const
{
    bool result = ! _CONST_CAST(const MachLogPod&, *pActor_).weapons().front()->recharged();

    return result;
}

bool MachLogPodAttackOperation::doBeInterrupted()
{
    return true;
}

/* //////////////////////////////////////////////////////////////// */

MachLogPodAttackAnimation::MachLogPodAttackAnimation(MachLogPod* pActor, const MexPoint3d& targetPosition)
    : pActor_(pActor)
    , targetPosition_(targetPosition)
{
    // deliberately left blank
}

void MachLogPodAttackAnimation::doOutputOperator(std::ostream& o) const
{
    o << "MachLogPodAttackAnimation ";
}

MachLogPodAttackAnimation::~MachLogPodAttackAnimation()
{
    // deliberately left blank
}

///////////////////////////////

PhysRelativeTime MachLogPodAttackAnimation::doStartAnimation()
{
    return pActor_->attack(targetPosition_);
}

MexRadians angleToTurnToFace(const MachLogPod& actor, const MexPoint3d& pos)
{
    MexPoint2d actorPos(actor.position());

    MexVec2 bearingToTarget(pos.x() - actorPos.x(), pos.y() - actorPos.y());

    MexVec3 xBasis3;
    actor.pPhysPod()->globalTransform().xBasis(&xBasis3);

    MexVec2 xBasis2(xBasis3);
    return xBasis2.angleBetween(bearingToTarget);
}

/////////////////////////////////////////////////// persistence /////////////////////////////////////////////////////

void perWrite(PerOstream& ostr, const MachLogPodAttackOperation& op)
{
    const MachLogOperation& base1 = op;

    ostr << base1;
    ostr << op.pActor_;
    ostr << op.targetPosition_;
    ostr << op.lastTargetPosition_;
}

void perRead(PerIstream& istr, MachLogPodAttackOperation& op)
{
    MachLogOperation& base1 = op;

    istr >> base1;
    istr >> op.pActor_;
    istr >> op.targetPosition_;
    istr >> op.lastTargetPosition_;
}

MachLogPodAttackOperation::MachLogPodAttackOperation(PerConstructor con)
    : MachLogOperation(con)
{
}

void perWrite(PerOstream& ostr, const MachLogPodAttackAnimation& op)
{
    const MachLogAnimation& base1 = op;

    ostr << base1;
    ostr << op.pActor_;
    ostr << op.targetPosition_;
}

void perRead(PerIstream& istr, MachLogPodAttackAnimation& op)
{
    MachLogAnimation& base1 = op;

    istr >> base1;
    istr >> op.pActor_;
    istr >> op.targetPosition_;
}

MachLogPodAttackAnimation::MachLogPodAttackAnimation(PerConstructor con)
    : MachLogAnimation(con)
{
}

/* End OPPODATT.CPP *************************************************/
