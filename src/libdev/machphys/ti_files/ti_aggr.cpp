/*
 * T I _ A G G R . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"
#include "machphys/SubId.hpp"
// #include "machphys/SubId.ctp"

#include "machphys/Machines/Aggressor.hpp"

using MachPhysAggressorId = MachPhysAggressor::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysAggressorId, MachPhysAggressor);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysAggressorId, MachPhysAggressor);

void MachDummyFunctionAggressor()
{
    static MachPhysObjectFactory<MachPhysAggressor::Id, MachPhysAggressor> dummyFactory4(1);
}

/* End TI_AGGR.CPP *****************************************************/
