/*
 * T I _ M I S . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

// #include "utility/basfact.hpp"
// #include "utility/basfact.ctp"

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"
#include "machphys/SubId.hpp"
// #include "machphys/SubId.ctp"

#include "machphys/Constructions/MissileEmplacement.hpp"

using MachPhysMissileEmplacementId = MachPhysMissileEmplacement::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysMissileEmplacementId, MachPhysMissileEmplacement);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysMissileEmplacementId, MachPhysMissileEmplacement);

void MachDummyFunctionMissileEmplacement()
{
    static MachPhysObjectFactory<MachPhysMissileEmplacement::Id, MachPhysMissileEmplacement> dummyFactory(1);
}

/* End TI8.CPP *****************************************************/
