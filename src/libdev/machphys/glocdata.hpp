/*
 * G L O C D A T A . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPhysGeoLocatorData

    Geo locator specific data lives here
*/

#ifndef _MACHPHYS_GLOCDATA_HPP
#define _MACHPHYS_GLOCDATA_HPP

#include "machphys/machdata.hpp"

class MachPhysGeoLocatorData : public MachPhysMachineData
{
public:
    ~MachPhysGeoLocatorData() override;

    void CLASS_INVARIANT;

    MachPhys::MineralGrade mineralGrade() const;

    friend std::ostream& operator<<(std::ostream& o, const MachPhysGeoLocatorData& t);

private:
    friend class MachPhysDataParser;
    // Operation deliberately revoked
    MachPhysGeoLocatorData(const MachPhysGeoLocatorData&);

    // Operation deliberately revoked
    MachPhysGeoLocatorData& operator=(const MachPhysGeoLocatorData&);

    // Operation deliberately revoked
    bool operator==(const MachPhysGeoLocatorData&);

    void mineralGrade(const MachPhys::MineralGrade&);

    MachPhysGeoLocatorData();
    MachPhys::MineralGrade mineralGrade_;
};

#endif

/* End GLOCDATA.HPP *************************************************/
