/*
 * T I _ C O N S T . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"
#include "machphys/SubId.hpp"
// #include "machphys/SubId.ctp"

#include "machphys/Machines/Constructor.hpp"

using MachPhysConstructorId = MachPhysConstructor::Id;

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysConstructorId, MachPhysConstructor);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, MachPhysConstructorId, MachPhysConstructor);

void MachDummyFunctionConstructor()
{
    static MachPhysObjectFactory<MachPhysConstructor::Id, MachPhysConstructor> dummyFactory7(1);
}

/* End TI_CONST.CPP *****************************************************/
