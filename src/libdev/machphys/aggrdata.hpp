/*
 * A G G R D A T A . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPhysAggressorData
    Data specific to aggressors lives here.
*/

#ifndef _AGGRDATA_HPP
#define _AGGRDATA_HPP

#include "machphys/machdata.hpp"

class MachPhysAggressorData : public MachPhysMachineData
{
public:
    ~MachPhysAggressorData() override;

    MATHEX_SCALAR accuracy() const;
    MATHEX_SCALAR nWeapons() const;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPhysAggressorData& t);

private:
    friend class MachPhysDataParser;

    // Operations deliberately revoked
    MachPhysAggressorData();
    MachPhysAggressorData(const MachPhysAggressorData&);
    MachPhysAggressorData& operator=(const MachPhysAggressorData&);
    bool operator==(const MachPhysAggressorData&);

    void accuracy(const MATHEX_SCALAR&);
    void nWeapons(const MATHEX_SCALAR&);

    MATHEX_SCALAR accuracy_;
    MATHEX_SCALAR nWeapons_;
};

#endif

/* End AGGRDATA.HPP *************************************************/
