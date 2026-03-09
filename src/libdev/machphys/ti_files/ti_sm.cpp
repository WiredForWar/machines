/*
 * T I _ S M . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

// #include "utility/basfact.hpp"
// #include "utility/basfact.ctp"

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "machphys/Constructions/Smelter.hpp"

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysSmelter);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysSmelter);

void MachDummyFunctionSmelter()
{
    static MachPhysObjectFactory<MachPhysSmelter::Id, MachPhysSmelter> dummyFactory(1);
}

/* End TI_SM.CPP *****************************************************/
