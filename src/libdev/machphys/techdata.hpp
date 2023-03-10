/*
 * T E C H D A T A . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPhysTechnicianData

    Technician specific default data
*/

#ifndef _TECHDATA_HPP
#define _TECHDATA_HPP

#include "machphys/machdata.hpp"

class MachPhysTechnicianData : public MachPhysMachineData
{
public:
    ~MachPhysTechnicianData() override;

    // get methods public
    MachPhys::ResearchUnits researchRate() const;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachPhysTechnicianData& t);

private:
    friend class MachPhysDataParser;
    // Operation deliberately revoked
    MachPhysTechnicianData(const MachPhysTechnicianData&);

    // Operation deliberately revoked
    MachPhysTechnicianData& operator=(const MachPhysTechnicianData&);

    // Operation deliberately revoked
    bool operator==(const MachPhysTechnicianData&);

    MachPhysTechnicianData();

    // set methods private
    void researchRate(const MachPhys::ResearchUnits&);

    MachPhys::ResearchUnits researchRate_;
};

#endif

/* End TECHDATA.HPP *************************************************/
