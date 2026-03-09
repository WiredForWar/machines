#pragma once

#include "machlog/Actors/Construction.hpp"
#include "machlog/Tech/ResearchTree.hpp"

class MachPhysSoftwareLab;
class MachLogRace;
class MexPoint3d;
class MexRadians;
class MachLogResearchItem;
class MachPhysSoftwareLabData;

class MachLogSoftwareLab : public MachLogConstruction
{
public:
    MachLogSoftwareLab(
        MachLogRace* pRace,
        uint level,
        const MexPoint3d& location,
        const MexRadians& angle);

    MachLogSoftwareLab(
        MachLogRace* pRace,
        uint level,
        const MexPoint3d& location,
        const MexRadians& angle,
        UtlId withId);

    ~MachLogSoftwareLab() override;

    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearanceFromDisplayedVolume) override;

    const MachPhysConstructionData& constructionData() const override;
    const MachPhysSoftwareLabData& data() const;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogSoftwareLab& t);

    MachLogResearchTree::ResearchItems& availableResearchItems();
    MachLogResearchTree::ResearchItems& currentResearchQueue();
    void initialiseResearchItems();

    bool addResearchItem(const MachLogResearchItem&);
    bool removeResearchItem(const MachLogResearchItem&);
    bool currentlyResearching(MachLogResearchItem**) const;
    MachPhys::BuildingMaterialUnits amountBuilt() const;
    MachPhys::ResearchUnits amountResearched() const;

    void cancelCurrentResearch();

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogSoftwareLab);
    PER_FRIEND_READ_WRITE(MachLogSoftwareLab);

protected:
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogSoftwareLab(const MachLogSoftwareLab&);
    MachLogSoftwareLab& operator=(const MachLogSoftwareLab&);
    bool operator==(const MachLogSoftwareLab&);

    static MachPhysConstruction* pNewPhysSoftwareLab(
        MachLogRace* pRace,
        uint level,
        const MexPoint3d& location,
        const MexRadians& angle);

    MachPhysSoftwareLab* pPhysSoftwareLab();
    const MachPhysSoftwareLab* pPhysSoftwareLab() const;

    MachLogResearchTree::ResearchItems availableResearchItems_{};
    MachLogResearchTree::ResearchItems currentResearchQueue_{};
    PhysAbsoluteTime lastUpdateTime_{};
};

PER_DECLARE_PERSISTENT(MachLogSoftwareLab);
