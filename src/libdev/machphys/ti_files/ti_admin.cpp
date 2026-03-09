/*
 * T I _ A D M I N . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"
#include "machphys/SubId.hpp"
// #include "machphys/SubId.ctp"

#include "machphys/Machines/Administrator.hpp"

using MachPhysAdministratorId = MachPhysAdministrator::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysAdministratorId, MachPhysAdministrator);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysAdministratorId, MachPhysAdministrator);

void MachDummyFunctionAdministrator()
{
    static MachPhysObjectFactory<MachPhysAdministrator::Id, MachPhysAdministrator> dummyFactory5(1);
}

/* End TI_ADMIN.CPP *****************************************************/
