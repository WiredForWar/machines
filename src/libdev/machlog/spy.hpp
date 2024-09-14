/*
 * S P Y . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _MACHLOG_SPY_LOCATOR_HPP
#define _MACHLOG_SPY_LOCATOR_HPP

#include "machlog/machine.hpp"

#include "machphys/machphys.hpp"

/* //////////////////////////////////////////////////////////////// */

// class ostream;
class MachPhysSpyLocator;
class MachPhysSpyLocatorData;
class MachLogResearchItem;

class MachLogSpyLocator : public MachLogMachine
{
public:
    MachLogSpyLocator(
        MachLogMachine::Level hwLevel,
        MachLogMachine::Level swLevel,
        MachLogRace* pRace,
        const MexPoint3d& location,
        std::optional<UtlId> withId = std::nullopt);

    ~MachLogSpyLocator() override;

    ///////////////////////////////

    // view of MachPhys data objects
    const MachPhysMachineData& machineData() const override;
    const MachPhysSpyLocatorData& data() const;

    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearence) override;

    void placeMine();
    void restockMines();
    int nResItemsICouldSteal(
        MachPhys::Race otherRace,
        MachPhys::HardwareLabSubType
            hardwareLabSubType); // returns total number of stealable technologies at the present time

    // Get methods public
    int nMines() const;
    bool isDownloading() const { return isDownloading_; }
    bool fullyStockedMines() const;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogSpyLocator);
    PER_FRIEND_READ_WRITE(MachLogSpyLocator);

protected:
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogSpyLocator(const MachLogSpyLocator&);
    MachLogSpyLocator& operator=(const MachLogSpyLocator&);
    bool operator==(const MachLogSpyLocator&) const;

    // The physical SpyLocator
    MachPhysSpyLocator& physSpyLocator();
    const MachPhysSpyLocator& physSpyLocator() const;

    // Functions used for stealing other races' technologies
    MachLogResearchItem*
    stealNewResearchItem(MachPhys::Race otherRace, MachPhys::HardwareLabSubType hardwareLabSubType);
    bool stealable(
        const MachLogResearchItem& researchItem,
        MachPhys::Race otherRace,
        MachPhys::HardwareLabSubType hardwareLabSubType);

    void executeTheft();
    // PRE ( insideBuilding() );
    // PRE (  insideWhichBuilding().objectType() == MachLog::HARDWARE_LAB );

    static MachPhysSpyLocator*
    pNewPhysSpyLocator(Level hwLevel, Level swLevel, MachLogRace* pRace, const MexPoint3d& location);

    // Set methods private

    void isDownloading(bool newStatus);

    int nMines_;
    PhysAbsoluteTime lastUpdateTime_;
    MachLogResearchItem* pResItemCurrentlyStealing_{};
    bool isDownloading_{}; // Whether or not the spy is actively downloading an enemy's research item
};

PER_DECLARE_PERSISTENT(MachLogSpyLocator);
/* //////////////////////////////////////////////////////////////// */

#endif /*  #ifndef _LOG_Spy_HPP    */

/* End Spy.HPP **************************************************/