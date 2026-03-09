/*
 * T I _ T E C H . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"
#include "machphys/SubId.hpp"
// #include "machphys/SubId.ctp"

#include "machphys/Machines/Technician.hpp"

using MachPhysTechnicianId = MachPhysTechnician::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysTechnicianId, MachPhysTechnician);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysTechnicianId, MachPhysTechnician);

void MachDummyFunctionTechnician()
{
    static MachPhysObjectFactory<MachPhysTechnician::Id, MachPhysTechnician> dummyFactory6(1);
}

/* End TI_TECH.CPP *****************************************************/
