/*
 * T I _ B K . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

// #include "utility/basfact.hpp"
// #include "utility/basfact.ctp"

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "machphys/Constructions/Beacon.hpp"

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysBeacon);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysBeacon);

void MachDummyFunctionBeacon()
{
    static MachPhysObjectFactory<size_t, MachPhysBeacon> dummyFactory(1);
}

/* End TI_BK.CPP *****************************************************/
