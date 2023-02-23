/*
 * O P S C A V . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogScavengeOperation

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_OPSCAV_HPP
#define _MACHLOG_OPSCAV_HPP

#include "base/base.hpp"

#include "machlog/operatio.hpp"

class MachLogDebris;
class MachLogResourceCarrier;
class MachLogScavengeOperationImpl;
template <class X> class ctl_pvector;

// canonical form revoked

class MachLogScavengeOperation : public MachLogOperation
{
public:
    using Suppliers = ctl_pvector<MachLogDebris>;

    // PRE( targetActor != NULL );
    MachLogScavengeOperation(MachLogResourceCarrier*, MachLogDebris*);

    // PRE( listOfSuppliers.size() > 0 );
    MachLogScavengeOperation(MachLogResourceCarrier*, const Suppliers&);

    ~MachLogScavengeOperation() override;

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogScavengeOperation);
    PER_FRIEND_READ_WRITE(MachLogScavengeOperation);

protected:
    bool doStart() override;
    // PRE( not isFinished() );
    void doFinish() override;
    // PRE( isFinished() );

    bool doIsFinished() const override;

    void doOutputOperator(ostream&) const override;

    PhysRelativeTime doUpdate() override;

    bool doBeInterrupted() override;
    ///////////////////////////////

private:
    // Operations deliberately revoked
    MachLogScavengeOperation(const MachLogScavengeOperation&);
    MachLogScavengeOperation& operator=(const MachLogScavengeOperation&);
    bool operator==(const MachLogScavengeOperation&);

    Suppliers& suppliers();

    MachLogScavengeOperationImpl* pImpl_;
};

PER_DECLARE_PERSISTENT(MachLogScavengeOperation);

#endif

/* End OPPICKUP.HPP *************************************************/
