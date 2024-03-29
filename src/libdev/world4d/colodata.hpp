/*
 * C O L O D A T A . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    W4dColourData

*/

#ifndef _WORLD4D_COLODATA_HPP
#define _WORLD4D_COLODATA_HPP

#include "base/base.hpp"
#include "base/persist.hpp"

#include "ctl/vector.hpp"

#include "phys/phys.hpp"

#include "world4d/world4d.hpp"
#include "render/colour.hpp"

class W4dColourPlanData
// Canonical form revoked
{
public:
    using ScalarVec = ctl_vector<PhysRelativeTime>;
    using ColourVec = ctl_vector<RenColour>;
    W4dColourPlanData(const ScalarVec& times, const ColourVec& colours);
    W4dColourPlanData(const W4dColourPlanData&);

    ~W4dColourPlanData();

    void CLASS_INVARIANT;

    const PhysRelativeTime& duration() const;

    const ScalarVec& times() const;
    const ColourVec& colours() const;

    void times(const ScalarVec&);
    void colours(const ColourVec&);

    //    returns the colour value at timeOffset
    RenColour colour(const PhysRelativeTime& timeOffset) const;

    PER_MEMBER_PERSISTENT(W4dColourPlanData);
    PER_FRIEND_READ_WRITE(W4dColourPlanData);

private:
    friend std::ostream& operator<<(std::ostream& o, const W4dColourPlanData& t);

    // W4dColourPlanData( const W4dColourPlanData& );
    W4dColourPlanData& operator=(const W4dColourPlanData&);

    ScalarVec times_;
    ColourVec colours_;
};

PER_DECLARE_PERSISTENT(W4dColourPlanData);

using W4dColourPlanDataPtr = CtlCountedPtr<W4dColourPlanData>;

#endif

/* End COLODATA.HPP **************************************************/
