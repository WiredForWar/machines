/*
 * O P N U K E A T . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
 * Mixin for logical objects that support an
 * attack operation.
 */

#ifndef _MACHLOG_OPNUKEAT_HPP
#define _MACHLOG_OPNUKEAT_HPP

#include "phys/phys.hpp"

#include "machlog/animatio.hpp"
#include "machlog/operatio.hpp"

class MachActor;
/* //////////////////////////////////////////////////////////////// */

/* //////////////////////////////////////////////////////////////// */

// forward refs
class MachLogMissileEmplacement;
class MachLogCanAttack;
class MachActor;

// canonical form revoked

class MachLogNukeAttackOperation : public MachLogOperation
{
public:
    MachLogNukeAttackOperation(MachLogMissileEmplacement* pActor, MachActor* pDirectObject);

    MachLogNukeAttackOperation(MachLogMissileEmplacement* pActor, const MexPoint3d& targetPosition);

    ~MachLogNukeAttackOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogNukeAttackOperation);
    PER_FRIEND_READ_WRITE(MachLogNukeAttackOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;

    MachActor& directObject();
    const MachActor& directObject() const;

    bool doBeInterrupted() override;

private:
    // Operations deliberately revoked
    MachLogNukeAttackOperation(const MachLogNukeAttackOperation&);
    MachLogNukeAttackOperation& operator=(const MachLogNukeAttackOperation&);
    bool operator==(const MachLogNukeAttackOperation&);

    MachLogMissileEmplacement* pActor_;
    MexPoint3d targetPosition_;
};

PER_DECLARE_PERSISTENT(MachLogNukeAttackOperation);

/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef     _MACHLOG_OPNUKEAT_HPP   */

/* End OPNUKEAT.HPP ***************************************************/
