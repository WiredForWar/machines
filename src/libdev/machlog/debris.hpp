/*
 * O R E H O L O . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogDebris

    Represents logical debris dropped from building etc when they explode.
*/

#ifndef _MACHLOG_DEBRIS_HPP
#define _MACHLOG_DEBRIS_HPP

#include "base/base.hpp"

#include "machlog/actor.hpp"
#include "machphys/machphys.hpp"

// forward refs
class MachLogRace;
class MexPoint3d;
class MexAlignedBox2d;
class MachPhysDebris;
class MachPhysConstructionData;

// orthodox canonical( revoked )
class MachLogDebris : public MachActor
{
public:
    // ctor
    MachLogDebris(
        MachLogRace* pRace,
        MachPhys::BuildingMaterialUnits quantity,
        const MexPoint3d& location,
        const MexAlignedBox2d& boundary);

    MachLogDebris(
        MachLogRace* pRace,
        MachPhys::BuildingMaterialUnits quantity,
        const MexPoint3d& location,
        const MexAlignedBox2d& boundary,
        UtlId);

    // dtor.
    ~MachLogDebris() override;

    // view of MachPhysObject data
    const MachPhysObjectData& objectData() const override;
    const MachPhysConstructionData& constructionData() const;
    const MachPhysConstructionData& data() const;

    void CLASS_INVARIANT;

    ///////////////////////////////

    // inherited from MachLogCanBeDestroyed
    PhysRelativeTime beDestroyed() override;

    virtual bool exists() const;

    ///////////////////////////////
    const MachPhys::BuildingMaterialUnits quantity() const;

    void hasBeenPickedUp(); // orders the debris to arrange its own instant death

    // inherited from MachActor
    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogDebris);
    PER_FRIEND_READ_WRITE(MachLogDebris);

protected:
    void doStartExplodingAnimation() override;
    void doEndExplodingAnimation() override;

private:
    // The physical factory
    MachPhysDebris* pPhysDebris();
    const MachPhysDebris* pPhysDebris() const;
    // Subclass must override to modify the display.
    // Called on change of state.
    void doVisualiseSelectionState() override;
    // Operations deliberately revoked
    MachLogDebris(const MachLogDebris&);
    MachLogDebris& operator=(const MachLogDebris&);
    bool operator==(const MachLogDebris&);

    // assigns a value to the lifetime of the debris which takes into account factors such as the existence of
    // scavengers in the game (if none, tweaks it so it disappears earlier).
    void setLifeTime(MachPhys::BuildingMaterialUnits quantity);

    // assign all children debrisunit objects of the associated machphysdebris object to have the same ID as
    // the debris object itself
    void setupChildrenDebrisIDs();

    // Construct a physical holograph at required location and in correct domain
    static MachPhysDebris* pNewDebris(
        MachLogRace* pRace,
        MachPhys::BuildingMaterialUnits quantity,
        const MexPoint3d& location,
        const MexAlignedBox2d& boundary);
    // data members
    MachPhys::BuildingMaterialUnits quantity_;
    PhysRelativeTime lifetimeRemaining_;
    PhysAbsoluteTime lastUpdateTime_;
    MexAlignedBox2d* pBoundary_;
};

PER_DECLARE_PERSISTENT(MachLogDebris);

#endif

/* End OREHOLO.HPP **************************************************/
