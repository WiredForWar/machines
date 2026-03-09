/*
 * T I _ G L O C . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "machphys/ObjectFactory.hpp"
// #include "machphys/ObjectFactory.ctp"

#include "machphys/Machines/GeoLocator.hpp"

PER_DEFINE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysGeoLocator);
PER_PRIVATE_READ_WRITE_PERSISTENT_T2(MachPhysObjectFactory, size_t, MachPhysGeoLocator);

void MachDummyFunctionGeoLocator()
{
    static MachPhysObjectFactory<size_t, MachPhysGeoLocator> dummyFactory3(1);
}

/* End TI_GLOC.CPP *****************************************************/
