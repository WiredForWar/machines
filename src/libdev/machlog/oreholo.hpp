/*
 * O R E H O L O . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogOreHolograph

    Represents logical mineral deposit holograph
*/

#ifndef _MACHLOG_OREHOLO_HPP
#define _MACHLOG_OREHOLO_HPP

#include "base/base.hpp"

#include "machlog/actor.hpp"

// forward refs
class MachLogRace;
class MexPoint3d;
class MachPhysOreHolograph;
class MachPhysConstructionData;
class MachLogMineralSite;

// orthodox canonical( revoked )
class MachLogOreHolograph : public MachActor
{
public:
    // ctor
    MachLogOreHolograph(MachLogRace* pRace, uint concentration, uint quantity, const MexPoint3d& location);

    MachLogOreHolograph(MachLogRace* pRace, uint concentration, uint quantity, const MexPoint3d& location, UtlId);

    // dtor.
    ~MachLogOreHolograph() override;

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

    const MachLogMineralSite& mineralSite() const;

    friend class MachLogMineralSite;

    void assignToDifferentRace(MachLogRace& newRace) override;

protected:
    void doStartExplodingAnimation() override;
    void doEndExplodingAnimation() override;

private:
    // Subclass must override to modify the display.
    // Called on change of state.
    void doVisualiseSelectionState() override;
    // Operations deliberately revoked
    MachLogOreHolograph(const MachLogOreHolograph&);
    MachLogOreHolograph& operator=(const MachLogOreHolograph&);
    bool operator==(const MachLogOreHolograph&);

    // designed for when the associated mineral site has been exhausted
    void removeMe();

    // Construct a physical holograph at required location and in correct domain
    static MachPhysOreHolograph*
    pNewOreHolograph(MachLogRace* pRace, uint concentration, uint quantity, const MexPoint3d& location);
    // data members

    MachPhysOreHolograph* pPhysHolo_;
};

#endif

/* End OREHOLO.HPP **************************************************/
