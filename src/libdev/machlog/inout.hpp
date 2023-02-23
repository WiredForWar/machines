/*
 * I N O U T . H P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#ifndef _MACH_INOUT_HPP
#define _MACH_INOUT_HPP

#include "machlog/operatio.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachLogMachine;
class MachLogConstruction;
class MachPhysStation;

class MachLogEnterBuildingOperation : public MachLogOperation
{
public:
    MachLogEnterBuildingOperation(MachLogMachine* pActor, MachLogConstruction* pConstr, MachPhysStation* pStation);
    // PRE( pActor->canEnterConstruction( *pConstr ) );

    MachLogEnterBuildingOperation(
        MachLogMachine* pActor,
        MachLogConstruction* pConstr,
        MachPhysStation* pStation,
        PhysPathFindingPriority);
    // PRE( pActor->canEnterConstruction( *pConstr ) );

    ~MachLogEnterBuildingOperation() override;

    const MachLogConstruction& destination() const;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogEnterBuildingOperation);
    PER_FRIEND_READ_WRITE(MachLogEnterBuildingOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(ostream&) const override;
    bool doBeInterrupted() override;
    bool isInterruptable() override;

private:
    MachLogMachine* pActor_;
    MachLogConstruction* pConstruction_;
    MachPhysStation* pStation_;
    enum Status
    {
        STATION,
        GOING_TO_STATION,
        GOING_TO_INTERIOR,
        INTERNAL_POINT,
        GOING_TO_INTERNAL_POINT,
        EXTERNAL_POINT,
        GOING_TO_EXTERNAL_POINT,
        CLEAR,
        TERMINATE_EARLY
    };

    friend ostream& operator<<(ostream& o, Status);

    // true if the machine's clearance circle intersects the entrance interior polygon
    bool onInternalEntrancePolygon() const;

    PER_FRIEND_ENUM_PERSISTENT(Status);
    Status status_;
    bool buildingGone_;
    bool currentlyAttached_;
    bool needToGetAStation_; // obsolete
    bool needToLockAStation_; // obsolete
    int stationType_;
    bool ignoreNeedToClearEntrancePolygon_;
    bool needToIgnoreInteriorPolygon_; // obsolete
};

PER_DECLARE_PERSISTENT(MachLogEnterBuildingOperation);
/* //////////////////////////////////////////////////////////////// */

class MachLogLeaveBuildingOperation : public MachLogOperation
{
public:
    MachLogLeaveBuildingOperation(MachLogMachine* pActor, MachLogConstruction*, MachPhysStation*);

    ~MachLogLeaveBuildingOperation() override;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogLeaveBuildingOperation);
    PER_FRIEND_READ_WRITE(MachLogLeaveBuildingOperation);

protected:
    bool doStart() override;
    void doFinish() override;

    bool doIsFinished() const override;
    PhysRelativeTime doUpdate() override;

    void doOutputOperator(ostream&) const override;
    bool doBeInterrupted() override;
    bool isInterruptable() override;

private:
    MachLogMachine* pActor_;
    MachLogConstruction* pConstruction_;
    MachPhysStation* pStation_;
    enum Status
    {
        STATION,
        GOING_TO_INTERNAL_POINT,
        INTERNAL_POINT,
        GOING_TO_EXTERNAL_POINT,
        EXTERNAL_POINT,
        GOING_CLEAR,
        CLEAR,
        TERMINATE_EARLY
    };

    // True if the machine is on the external entrance polygon of the construction
    bool onExternalEntrancePolygon() const;

    PER_FRIEND_ENUM_PERSISTENT(Status);
    Status status_;
    bool buildingGone_;
    bool currentlyAttached_;
    bool needToGetAStation_; // obsolete
    bool needToLockAStation_; // obsolete
    int stationType_;
    bool needToIgnoreExteriorPolygon_; // obsolete
    bool needToIgnoreInteriorPolygon_; // obsolete
};

PER_DECLARE_PERSISTENT(MachLogLeaveBuildingOperation);

/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef _MACH_INOUT_HPP */

/* End INOUT.HPP ****************************************************/