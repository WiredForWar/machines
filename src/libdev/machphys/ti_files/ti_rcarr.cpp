/*
 * T I _ R C A R R . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "machphys/Machines/ResourceCarrier.hpp"

using MachPhysResourceCarrierId = MachPhysResourceCarrier::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysResourceCarrierId, MachPhysResourceCarrier);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysResourceCarrierId, MachPhysResourceCarrier);

void MachDummyFunctionResourceCarrier()
{
    static MachPhysObjectFactory<size_t, MachPhysResourceCarrier> dummyFactory(1);
}

/* End TI2.CPP *****************************************************/
