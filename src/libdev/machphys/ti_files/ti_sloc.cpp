/*
 * T I _ S L O C . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "machphys/Machines/SpyLocator.hpp"

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysSpyLocator);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysSpyLocator);

void MachDummyFunctionSpyLocator()
{
    static MachPhysObjectFactory<size_t, MachPhysSpyLocator> dummyFactory3(1);
}

/* End TI_GLOC.CPP *****************************************************/
