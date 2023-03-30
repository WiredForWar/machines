/*
 * O P C O N S T R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogConstructOperation

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_OPCONSTR_HPP
#define _MACHLOG_OPCONSTR_HPP

#include "machlog/oplabour.hpp"

class MachLogConstructor;
class MachLogConstruction;

// orthodox canonical (revoked)

class MachLogConstructOperation : public MachLogLabourOperation
{
public:
    MachLogConstructOperation(MachLogConstructor* pActor, MachLogConstruction*);

    // Useful for evade logic to know
    bool constructingMissileEmplacement() const;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogConstructOperation);
    PER_FRIEND_READ_WRITE(MachLogConstructOperation);

protected:
    bool doIsFinished() const override;

    void doOutputOperator(std::ostream&) const override;

    // inherited from MLLabourOperation
    bool clientSpecificNotification(int clientData) override;

private:
    // Operations deliberately revoked
    MachLogConstructOperation(const MachLogConstructOperation&);
    MachLogConstructOperation& operator=(const MachLogConstructOperation&);
    bool operator==(const MachLogConstructOperation&);

    // inherited from MachLogLabourOperation
    PhysRelativeTime interactWithBuilding() override;
};

PER_DECLARE_PERSISTENT(MachLogConstructOperation);

#endif

/* End OPCONSTR.HPP *************************************************/
