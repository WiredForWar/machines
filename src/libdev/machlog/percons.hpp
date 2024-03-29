/*
 * P E R C O N S . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogConstructionPersistence

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_PERCONS_HPP
#define _MACHLOG_PERCONS_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "ctl/pvector.hpp"

class MachLogConstruction;

class MachLogConstructionPersistence
// Canonical form revoked
{
public:
    //  Singleton class
    static MachLogConstructionPersistence& instance();
    ~MachLogConstructionPersistence();

    void CLASS_INVARIANT;
    PER_MEMBER_PERSISTENT_DEFAULT(MachLogConstructionPersistence);
    PER_FRIEND_READ_WRITE(MachLogConstructionPersistence);

private:
    void registerDerivedClasses();
    // This const method is really non-const as it will set up the constructions data member
    // takes MLRaces views and converts to local vector for persistence
    void setUpLocalVectorFromViews() const;
    // takes local vecotr that has been populated from perRead and sets up other thingies with
    // pointers to actors.
    void setUpViewsFromLocalVector();
    friend std::ostream& operator<<(std::ostream& o, const MachLogConstructionPersistence& t);

    MachLogConstructionPersistence(const MachLogConstructionPersistence&);
    MachLogConstructionPersistence& operator=(const MachLogConstructionPersistence&);

    MachLogConstructionPersistence();
    using Constructions = ctl_pvector<MachLogConstruction>;
    Constructions constructions_;
};

PER_DECLARE_PERSISTENT(MachLogConstructionPersistence);

#endif

/* End PERCONS.HPP **************************************************/
