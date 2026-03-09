/*
 * T I _ P O D . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

// #include "utility/basfact.hpp"
// #include "utility/basfact.ctp"

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "ctl/fixedvec.hpp"
// #include "ctl/fixedvec.ctp"

#include "machphys/Constructions/Pod.hpp"

using MachPhysPodId = MachPhysPod::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysPodId, MachPhysPod);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysPodId, MachPhysPod);

void MachDummyFunctionPod()
{
    static MachPhysObjectFactory<MachPhysPod::Id, MachPhysPod> dummyFactory(1);
}

/* End TI8.CPP *****************************************************/
