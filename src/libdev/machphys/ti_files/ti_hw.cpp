/*
 * T I _ H W . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

// #include "utility/basfact.hpp"
// #include "utility/basfact.ctp"

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"
#include "machphys/SubId.hpp"
// #include "machphys/SubId.ctp"

#include "machphys/Constructions/HardwareLab.hpp"

using MachPhysHardwareLabId = MachPhysHardwareLab::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysHardwareLabId, MachPhysHardwareLab);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysHardwareLabId, MachPhysHardwareLab);

void MachDummyFunctionHardwareLab()
{
    static MachPhysObjectFactory<MachPhysHardwareLab::Id, MachPhysHardwareLab> dummyFactory(1);
}

/* End TI8.CPP *****************************************************/
