/*
 * T I _ P C A R R . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "ctl/Map.hpp"
// #include "ctl/Map.ctp"

#include "machphys/Machines/APC.hpp"

using MachPhysAPCId = MachPhysAPC::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysAPCId, MachPhysAPC);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysAPCId, MachPhysAPC);

void MachDummyFunctionPersonnelCarrier()
{
    static MachPhysObjectFactory<size_t, MachPhysAPC> dummyFactory(1);
}

/* End TI2.CPP *****************************************************/
