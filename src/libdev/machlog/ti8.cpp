/*
 * T I 8 . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved.
 */
#define _INSTANTIATE_TEMPLATE_CLASSES

#include "ctl/vector.hpp"
#include "phys/phys.hpp"
#include "machlog/Operations/MachineOperation.hpp"
#include "machlog/ProductionUnit.hpp"
#include "machlog/Internal/GroupMoveUtility.hpp"

void ti8()
{
    static ctl_vector<PhysAbsoluteTime> dummyX;

    static MachLogMachineOperations dummy1;

    static ctl_vector<MachLogGroupMoverUtility::Clump> dummy2;

    typedef ctl_vector<MachLogProductionUnit> MachLogProductionUnits;
    static MachLogProductionUnits dummy3;
    static ctl_vector<MachLogProductionUnits> dummy4;
}

/* End TI8.CPP *******************************************************/