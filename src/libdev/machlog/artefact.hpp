/*
 * A R T E F A C T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogArtefact

    Models a (terrain) artefact which interacts logically with the agme, and is therefore
    an actor.
*/

#ifndef _MACHLOG_ARTEFACT_HPP
#define _MACHLOG_ARTEFACT_HPP

#include "base/base.hpp"
#include "utility/id.hpp"
#include "machphys/artfdata.hpp"
#include "machlog/actor.hpp"

// forward refs
template <class T> class ctl_list;
class MexPoint3d;
class MexRadians;
class MachPhysArtefact;
class FtlSerialId;
class MachLogArtefact;

// Stores damage link to another artefact
class MachLogArtefactLinkData
{
public:
    enum DamageType
    {
        DAMAGE_ABSOLUTE,
        DAMAGE_RELATIVE
    };
    MachLogArtefactLinkData(
        MachLogArtefact* pLinkedArtefact,
        DamageType hpDamageType,
        int hpDamage,
        DamageType armourDamageType,
        int armourDamage)
        : pLinkedArtefact_(pLinkedArtefact)
        , hpDamageType_(hpDamageType)
        , hpDamage_(hpDamage)
        , armourDamageType_(armourDamageType)
        , armourDamage_(armourDamage) {};

    //////////////////////////////////////////
    // Needed by ctl_list

    PER_MEMBER_PERSISTENT_DEFAULT(MachLogArtefactLinkData);

    MachLogArtefactLinkData() {};

    friend bool operator<(const MachLogArtefactLinkData& lhs, const MachLogArtefactLinkData& rhs)
    {
        return lhs.hpDamage_ < rhs.hpDamage_;
    };

    friend bool operator==(const MachLogArtefactLinkData& lhs, const MachLogArtefactLinkData& rhs)
    {
        return lhs.hpDamage_ == rhs.hpDamage_;
    };

    //////////////////////////////////////////

    // data members
    MachLogArtefact* pLinkedArtefact_; // Artefact to be damaged
    DamageType hpDamageType_; // Indicates whether hp to be altered in absolute or relative manner
    int hpDamage_; // hp damage value
    DamageType armourDamageType_; // Indicates whether armour to be altered in absolute or relative manner
    int armourDamage_; // armour damage value
};

PER_ENUM_PERSISTENT(MachLogArtefactLinkData::DamageType);

class MachLogArtefact : public MachActor
// Canonical form revoked
{
public:
    // Constructs an artefact of registered subType at location rotated about the z axis
    // through angle.
    // Overload uses supplied logical actor id.
    MachLogArtefact(int subType, const MexPoint3d& location, const MexRadians& angle);
    MachLogArtefact(int subType, const MexPoint3d& location, const MexRadians& angle, UtlId withId);

    ~MachLogArtefact() override;

    // Scenario specific sub-type
    int subType() const;

    // The id of the obstacle polygon. NB exported as shown to avoid dependency on cspace2.hpp.
    // Type should be PhysConfigSpace2d::PolygonId.
    FtlSerialId obstaclePolygonId() const;

    // Set up damage to occur to pLinkedArtefact when this artefact is destroyed.
    // That artefact's hitpoints are modified to have absolute value hpDamage if hpDamageType is ABSOLUTE,
    // or change by hpDamage if RELATIVE.
    // Similar rules apply to its armour value as defined by armourDamageType and armourDamage.
    void damageOnDestroy(
        MachLogArtefact* pLinkedArtefact,
        MachLogArtefactLinkData::DamageType hpDamageType,
        int hpDamage,
        MachLogArtefactLinkData::DamageType armourDamageType,
        int armourDamage);

    // Ensure the physical model doesnt get deleted for the specified time.
    // This is done by adding a dying entity event.
    void preservePhysicalModel(const PhysRelativeTime& forTime);

    // Drop debris model corresponding to this object, starting at specified time.
    //(The time functionality isn't implemented yet - always drops at current time).
    void dropDebris(const PhysAbsoluteTime& atTime);

    /////////////////////////////////////////////////
    // Inherited from SimActor
    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearanceFromDisplayedVolume) override;

    /////////////////////////////////////////////////
    // Inherited from MachLogCanBeDestroyed

    // Destroy oneself, returning
    // the time any animation will take
    PhysRelativeTime beDestroyed() override;

    /////////////////////////////////////////////////
    // Inherited from MachActor

    void
    beHit(const int&, MachPhys::WeaponType byType, MachActor* pByActor, MexLine3d* pByDirection, EchoBeHit) override;

    void beHitWithoutAnimation(
        int damage,
        PhysRelativeTime physicalTimeDelay = 0,
        MachActor* pByActor = nullptr,
        EchoBeHit = ECHO) override;

    // return object data view of actor
    const MachPhysObjectData& objectData() const override;

    /////////////////////////////////////////////////
    // Inherited from W4dObserver

    // true iff this observer is to exist in this subject's list of observers
    // following this call. This will typically be implemented using double dispatch.
    // The clientData is of interest only if event == CLIENT_SPECIFIC.Interpretation
    // is client defined.
    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    // Informs observer that an observed domain is being deleted.
    // This observer need not call the W4dDomain::detach() method - this
    // will be done automatically.
    void domainDeleted(W4dDomain* pDomain) override;

    /////////////////////////////////////////////////

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogArtefact);
    PER_FRIEND_READ_WRITE(MachLogArtefact);

protected:
    /////////////////////////////////////////////////
    // Inherited from MachActor

    void doStartExplodingAnimation() override;
    void doEndExplodingAnimation() override;

    /////////////////////////////////////////////////

private:
    friend std::ostream& operator<<(std::ostream& o, const MachLogArtefact& t);

    MachLogArtefact(const MachLogArtefact&);
    MachLogArtefact& operator=(const MachLogArtefact&);

    /////////////////////////////////////////////////
    // Inherited from MachActor

    // Subclass must override to modify the display.
    // Called on change of state.
    void doVisualiseSelectionState() override;

    /////////////////////////////////////////////////

    // Constructs and returns a new artefact model of given subType.
    // NB Also initialises pPhysArtefact_ as a side effect.
    W4dEntity* newPhysArtefact(int subType, const MexPoint3d& location, const MexRadians& angle);

    using LinkDatas = ctl_list<MachLogArtefactLinkData>;

    // Inflict specified damage on linked artefact
    void damageLinkedArtefact(const MachLogArtefactLinkData& linkData);

    // Register an obstacle polygon with the config space
    void addObstaclePolygon();

    // Do common ctor initialisation
    void initialise();

    // data members
    MachPhysArtefact* pPhysArtefact_; // The physical rep NB IS_NOT_A W4dEntity, but HAS-A
    LinkDatas* pLinkDatas_; // Collection of artefact damage link information
    uint obstaclePolygonId_; // Id of the obstacle polygon registered in the config space.
                             // NB saved in this form to avoid dependency on cspace2.hpp
};

PER_DECLARE_PERSISTENT(MachLogArtefact);

PER_READ_WRITE(MachLogArtefactLinkData);
PER_DECLARE_PERSISTENT(MachLogArtefactLinkData);

#ifdef _INLINE
#include "machlog/artefact.ipp"
#endif

#endif

/* End ARTEFACT.HPP *************************************************/
