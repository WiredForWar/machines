/*
 * O P R E P A I R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogRepairOperation

    This operation is used by constructors to capture enemy buildings.
    The building can be captured if it is in the red zone of health.
*/

#ifndef _MACHLOG_OPREPAIR_HPP
#define _MACHLOG_OPREPAIR_HPP

#include "machlog/oplabour.hpp"

class MachLogConstructor;
class MachLogConstruction;

// orthodox canonical (revoked)

class MachLogRepairOperation : public MachLogLabourOperation
{
public:
    MachLogRepairOperation(MachLogConstructor* pActor, MachLogConstruction*);

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogRepairOperation);
    PER_FRIEND_READ_WRITE(MachLogRepairOperation);

protected:
    bool doIsFinished() const override;

    void doOutputOperator(std::ostream&) const override;

    // inherited from MLLabourOperation
    bool clientSpecificNotification(int clientData) override;

private:
    // Operations deliberately revoked
    MachLogRepairOperation(const MachLogRepairOperation&);
    MachLogRepairOperation& operator=(const MachLogRepairOperation&);
    bool operator==(const MachLogRepairOperation&);

    // inherited from MachLogLabourOperation
    PhysRelativeTime interactWithBuilding() override;
};

PER_DECLARE_PERSISTENT(MachLogRepairOperation);

#endif

/* End OPREPAIR.HPP *************************************************/
