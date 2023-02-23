/*
 * C O V A P T R A . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysDynamicLightsComplexity

    A brief description of the class should go in here
*/

#ifndef _COVAPTRA_HPP
#define _COVAPTRA_HPP

#include "base/base.hpp"
#include "machphys/compitem.hpp"

class MachPhysVapourTrailsComplexity : public MachPhysComplexityBooleanItem
{
public:
    MachPhysVapourTrailsComplexity(const ItemId& id, bool enabled);
    ~MachPhysVapourTrailsComplexity() override;

    void update() override;
};

#endif

/* End COVAPTRA.HPP **************************************************/
