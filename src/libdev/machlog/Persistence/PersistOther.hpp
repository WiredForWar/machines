/*
 * P E R O T H E R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogOtherPersistence

    Persists other types (i.e. not machines or constructions) of actor.
*/

#ifndef _MACHLOG_PEROTHER_HPP
#define _MACHLOG_PEROTHER_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "ctl/pvector.hpp"

class MachActor;
class MachLogMineralSite;
class MachLogResearchItem;
class MachLogConstructionItem;
class SimAction;
class SimCondition;

class MachLogOtherPersistence
// Canonical form revoked
{
public:
    //  Singleton class
    static MachLogOtherPersistence& instance();
    ~MachLogOtherPersistence();

    void CLASS_INVARIANT;
    PER_MEMBER_PERSISTENT_DEFAULT(MachLogOtherPersistence);
    PER_FRIEND_READ_WRITE(MachLogOtherPersistence);

private:
    void registerDerivedClasses();
    // This const method is really non-const as it will set up the others data member
    // takes MLRaces views and converts to local vector for persistence
    void setUpLocalVectorFromViews() const;
    // takes local vecotr that has been populated from perRead and sets up other thingies with
    // pointers to actors.
    void setUpViewsFromLocalVector();

    friend std::ostream& operator<<(std::ostream& o, const MachLogOtherPersistence& t);

    MachLogOtherPersistence(const MachLogOtherPersistence&);
    MachLogOtherPersistence& operator=(const MachLogOtherPersistence&);

    MachLogOtherPersistence();
    using Others = ctl_pvector<MachActor>;
    using Sites = ctl_pvector<MachLogMineralSite>;
    using ResearchItems = ctl_pvector<MachLogResearchItem>;
    using ConstructionItems = ctl_pvector<MachLogConstructionItem>;
    using Actions = ctl_pvector<SimAction>;
    using Conditions = ctl_pvector<SimCondition>;

    Others others_; // actor types not machine or constructions
    Sites sites_; // mineral sites - copied to/from MLPlamet collection
    ResearchItems researchItems_; // research items - copied to/from MLRaces::researchTree collection
    Actions actions_; // given to SimConditionsManager
    Conditions conditions_; // as are these.
    ConstructionItems constructionItems_; // copied to from MLRaces::constructionTree
};

PER_DECLARE_PERSISTENT(MachLogOtherPersistence);

#endif

/* End PERMACH.HPP **************************************************/
