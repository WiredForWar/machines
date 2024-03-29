/*
 * G E O L O C A I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/internal/geolocai.hpp"
#include "machphys/locator.hpp"

PER_DEFINE_PERSISTENT(MachPhysGeoLocatorImpl);

MachPhysGeoLocatorImpl::MachPhysGeoLocatorImpl()
    : isLocating_(false)
    , locatingPlanEndTime_(0)
    , pLocator_(nullptr)
    , locatorSize_(0)
{

    TEST_INVARIANT;
}

MachPhysGeoLocatorImpl::~MachPhysGeoLocatorImpl()
{
    delete pLocator_;
    TEST_INVARIANT;
}

void MachPhysGeoLocatorImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysGeoLocatorImpl& t)
{

    o << "MachPhysGeoLocatorImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysGeoLocatorImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

MachPhysGeoLocatorImpl::MachPhysGeoLocatorImpl(PerConstructor)
    : isLocating_(false)
    , locatingPlanEndTime_(0)
    , pLocator_(nullptr)
    , locatorSize_(0)
{
}

void perWrite(PerOstream& str, const MachPhysGeoLocatorImpl& geoLocatorImpl)
{
    // str << geoLocatorImpl.pLocator_;
    // str << geoLocatorImpl.locatorSize_;
}

void perRead(PerIstream& str, MachPhysGeoLocatorImpl& geoLocatorImpl)
{
    // str >> geoLocatorImpl.pLocator_;
    // str >> geoLocatorImpl.locatorSize_;
}

/* End GEOLOCAI.CPP *************************************************/
