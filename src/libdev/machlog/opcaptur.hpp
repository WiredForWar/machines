/*
 * O P C A P T U R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogCaptureOperation

    This operation is used by constructors to capture enemy buildings.
    The building can be captured if it is in the red zone of health.
*/

#ifndef _MACHLOG_OPCAPTUR_HPP
#define _MACHLOG_OPCAPTUR_HPP

#include "machlog/oplabour.hpp"

class MachLogConstructor;
class MachLogConstruction;

// orthodox canonical (revoked)

class MachLogCaptureOperation : public MachLogLabourOperation
{
public:
    MachLogCaptureOperation(MachLogConstructor* pActor, MachLogConstruction*);
    ~MachLogCaptureOperation() override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogCaptureOperation);
    PER_FRIEND_READ_WRITE(MachLogCaptureOperation);

protected:
    bool doIsFinished() const override;

    void doOutputOperator(ostream&) const override;

    // inherited from MLLabourOperation
    bool clientSpecificNotification(int clientData) override;

private:
    // Operations deliberately revoked
    MachLogCaptureOperation(const MachLogCaptureOperation&);
    MachLogCaptureOperation& operator=(const MachLogCaptureOperation&);
    bool operator==(const MachLogCaptureOperation&);

    // inherited from MachLogLabourOperation
    PhysRelativeTime interactWithBuilding() override;
};

PER_DECLARE_PERSISTENT(MachLogCaptureOperation);

#endif

/* End OPCAPTUR.HPP *************************************************/
