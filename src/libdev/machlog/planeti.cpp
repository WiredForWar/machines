/*
 * P L A N E T I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <string>
#include "ctl/vector.hpp"
#include "machlog/internal/planeti.hpp"
#include "world4d/root.hpp"

MachLogPlanetImpl::MachLogPlanetImpl()
    : pSurface_(nullptr)
    , pWorld_(nullptr)
    , pConfigSpace_(nullptr)
    , pPressurePads_(nullptr)
    , pHiddenRootDomain_(nullptr)
    , pHiddenConfigSpace_(nullptr)
    , pHiddenRoot_(new W4dRoot(100243))
    , idsGenerated_(0)
{

    TEST_INVARIANT;
}

MachLogPlanetImpl::~MachLogPlanetImpl()
{
    TEST_INVARIANT;
}

void MachLogPlanetImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogPlanetImpl& t)
{

    o << "MachLogPlanetImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogPlanetImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End PLANETI.CPP **************************************************/
