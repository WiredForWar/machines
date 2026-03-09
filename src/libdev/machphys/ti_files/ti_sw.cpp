/*
 * T I _ S W . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

// #include "utility/basfact.hpp"
// #include "utility/basfact.ctp"

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "machphys/Constructions/SoftwareLab.hpp"

typedef MachPhysSoftwareLab::Id MachPhysSoftwareLabId;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysSoftwareLabId, MachPhysSoftwareLab);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysSoftwareLabId, MachPhysSoftwareLab);

void MachDummyFunctionSwLab()
{
    static MachPhysObjectFactory<MachPhysSoftwareLab::Id, MachPhysSoftwareLab> dummyFactory(1);
}

/* End TI8.CPP *****************************************************/
