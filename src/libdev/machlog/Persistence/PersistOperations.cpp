/*
 * P E R W E A P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "base/Persistence.hpp"
#include "machphys/machphys.hpp"
#include "machlog/World/MachLog.hpp"
#include "machlog/Persistence/PersistOperations.hpp"

#include "machlog/Operations/AttackOperation.hpp"
#include "machlog/Operations/AnimationOperation.hpp"
#include "machlog/Operations/ResearchAnimation.hpp"
#include "machlog/Operations/FollowOperation.hpp"
#include "machlog/Operations/EnterLeaveBuildingOperation.hpp"
#include "machlog/Operations/MoveOperation.hpp"
#include "machlog/Operations/PatrolOperation.hpp"
#include "machlog/Operations/SeekAndDestroyOperation.hpp"
#include "machlog/Operations/AdminAttackOperation.hpp"
#include "machlog/Operations/AdminCaptureOperation.hpp"
#include "machlog/Operations/AdminConstructOperation.hpp"
#include "machlog/Operations/AdminDeconstructOperation.hpp"
#include "machlog/Operations/AdminHealOperation.hpp"
#include "machlog/Operations/AdminLocateOperation.hpp"
#include "machlog/Operations/AdminMoveOperation.hpp"
#include "machlog/Operations/AdminRepairOperation.hpp"
#include "machlog/Operations/AdminSuperConstructOperation.hpp"
#include "machlog/Operations/AutoScavengeOperation.hpp"
#include "machlog/Operations/CaptureOperation.hpp"
#include "machlog/Operations/ConstructOperation.hpp"
#include "machlog/Operations/CounterattackOperation.hpp"
#include "machlog/Operations/DeconstructOperation.hpp"
#include "machlog/Operations/DeployAPCOperation.hpp"
#include "machlog/Operations/EnterAPCOperation.hpp"
#include "machlog/Operations/EvadeOperation.hpp"
#include "machlog/Operations/GotoLabOperation.hpp"
#include "machlog/Operations/HealOperation.hpp"
#include "machlog/Operations/DropLandMineOperation.hpp"
#include "machlog/Operations/LocateOperation.hpp"
#include "machlog/Operations/MissileEmplacementAttackOperation.hpp"
#include "machlog/Operations/PickUpOperation.hpp"
#include "machlog/Operations/PodAttackOperation.hpp"
#include "machlog/Operations/PutDownOperation.hpp"
#include "machlog/Operations/RecycleOperation.hpp"
#include "machlog/Operations/RefillLandMinesOperation.hpp"
#include "machlog/Operations/RepairOperation.hpp"
#include "machlog/Operations/ScavengeOperation.hpp"
#include "machlog/Operations/StandGroundOperation.hpp"
#include "machlog/Operations/SuperConstructOperation.hpp"
#include "machlog/Operations/SuperConstructOperation.hpp"
#include "machlog/Operations/TransportOperation.hpp"
#include "machlog/Operations/TreacheryReachOperation.hpp"
#include "machlog/Operations/TaskAttackOperation.hpp"
#include "machlog/Operations/TaskConstructOperation.hpp"
#include "machlog/Operations/TaskDropLandMineOperation.hpp"
#include "machlog/Operations/TaskLocateOperation.hpp"
#include "machlog/Operations/TaskPatrolOperation.hpp"

MachLogOperationPersistence::MachLogOperationPersistence()
{

    TEST_INVARIANT;
}

MachLogOperationPersistence::~MachLogOperationPersistence()
{
    TEST_INVARIANT;
}

void MachLogOperationPersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogOperationPersistence& t)
{

    o << "MachLogOperationPersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogOperationPersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// static
void MachLogOperationPersistence::registerDerivedClasses()
{
    PER_REGISTER_DERIVED_CLASS(MachLogMoveToOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogMoveAndTurnOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogAttackOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogFollowOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogSeekAndDestroyOperation);
    //  PER_REGISTER_DERIVED_CLASS( MachLogAnimation );
    PER_REGISTER_DERIVED_CLASS(MachLogAdminAttackOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogAdminConstructOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogAdminHealOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogAdminLocateOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogCaptureOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogConstructOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogDeployAPCOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogEnterAPCOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogGotoLabOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogHealOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogDropLandMineOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogLocateOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogMissileEmplacementAttackOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogPickUpOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogPodAttackOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogPodAttackAnimation);
    PER_REGISTER_DERIVED_CLASS(MachLogPutDownOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogRefillLandMinesOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogRepairOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogTransportOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogTreacheryOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogTaskAttackOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogTaskConstructOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogTaskDropLandMineOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogTaskLocateOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogTaskPatrolOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogPatrolOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogEnterBuildingOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogLeaveBuildingOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogMoveAndTurnOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogDeconstructOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogRecycleOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogAdminDeconstructOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogAdminCaptureOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogSuperConstructOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogAdminSuperConstructOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogStandGroundOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogTurnAnimation);
    // PER_REGISTER_DERIVED_CLASS( MachLogConstructAnimation );
    PER_REGISTER_DERIVED_CLASS(MachLogBeDestroyedAnimation);
    PER_REGISTER_DERIVED_CLASS(MachLogResearchAnimation);
    PER_REGISTER_DERIVED_CLASS(MachLogCounterattackOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogScavengeOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogAutoScavengeOperation);
    PER_REGISTER_DERIVED_CLASS(MachLogEvadeOperation);
}

// Forced recompile 12/1/99 CPS

/* End PERWEAP.CPP **************************************************/
