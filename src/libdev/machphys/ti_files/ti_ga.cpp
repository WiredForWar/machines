/*
 * T I _ G A . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

// #include "utility/basfact.hpp"
// #include "utility/basfact.ctp"

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "machphys/Constructions/Garrison.hpp"

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysGarrison);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysGarrison);

void MachDummyFunctionGarrison()
{
    static MachPhysObjectFactory<MachPhysGarrison::Id, MachPhysGarrison> dummyFactory(1);
}

/* End TI8.CPP *****************************************************/
