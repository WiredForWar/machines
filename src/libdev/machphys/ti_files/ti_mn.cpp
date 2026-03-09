/*
 * T I _ M N . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

// #include "utility/basfact.hpp"
// #include "utility/basfact.ctp"

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "machphys/Constructions/Mine.hpp"

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysMine);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysMine);

void MachDummyFunctionMine()
{
    static MachPhysObjectFactory<MachPhysMine::Id, MachPhysMine> dummyFactory9(1);
}

/* End TI8.CPP *****************************************************/
