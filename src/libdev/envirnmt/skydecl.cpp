/***********************************************************

  S K Y D E C L . H P P
  Copyright (c) 1998 Charybdis Limited, All rights reserved.

***********************************************************/

#include "envirnmt/internal/skydecl.hpp"

#include "stdlib/string.hpp"

#include "mathex/angle.hpp"

#include "ctl/vector.hpp"

#include "system/pathname.hpp"

#include "render/colour.hpp"

#include "envirnmt/internal/plaparse.hpp"
#include "envirnmt/sky.hpp"

class EnvSatellite;

void EnvISkyDeclaration::UniformSky::controller(const string& name)
{
    PRE(! controllerSet_);

    EnvSatellite* pController = EnvIPlanetParser::instance().lookUpSatellite(&name);
    ASSERT(pController, "An invalid satellite name was parsed.");

    pSky_->controller(pController);
    controllerSet_ = true;
}

void EnvISkyDeclaration::UniformSky::colourTable(const string& name)
{
    PRE(! colourTableSet_);

    EnvElevationColourTable* pElevationClut = EnvIPlanetParser::instance().lookUpClut(&name);
    ASSERT(pElevationClut, "An invalid elevation clut name was parsed.");

    pSky_->colourTable(pElevationClut);
    colourTableSet_ = true;
}

void EnvISkyDeclaration::UniformSky::completeSky()
{
    PRE(controllerSet_);
    PRE(colourTableSet_);

    EnvIPlanetParser::instance().skyDeclaration().setCompleteSky(pSky_);
}

EnvISkyDeclaration::UniformSky::UniformSky(EnvUniformSky* const pSky)

    : controllerSet_(false)
    , colourTableSet_(false)
    , pSky_(pSky)
{
    PRE(pSky);
    PRE(! pSky_->controller());
}

////////////////////////////////////////////////////////////

void EnvISkyDeclaration::StaticSky::meshFile(const string& pathname)
{
    PRE(! meshFileSet_);

    pSky_->mesh(pathname);
    meshFileSet_ = true;
}

void EnvISkyDeclaration::StaticSky::backgroundColour(const RenColour& backgrdColour)
{
    PRE(! backgroundColourSet_);

    pSky_->backgroundColour(backgrdColour);
    backgroundColourSet_ = true;
}

void EnvISkyDeclaration::StaticSky::completeSky()
{
    PRE(meshFileSet_);
    PRE(backgroundColourSet_);

    EnvIPlanetParser::instance().skyDeclaration().setCompleteSky(pSky_);
}

EnvISkyDeclaration::StaticSky::StaticSky(EnvStaticSky* const pSky)

    : meshFileSet_(false)
    , backgroundColourSet_(false)
    , pSky_(pSky)
{
    PRE(pSky);
}

////////////////////////////////////////////////////////////

void EnvISkyDeclaration::DynamicSky::meshFile(const string& pathname)
{
    PRE(! meshFileSet_);

    pSky_->mesh(pathname);
    meshFileSet_ = true;
}

void EnvISkyDeclaration::DynamicSky::controller(const string& name)
{
    PRE(! controllerSet_);

    EnvSatellite* pController = EnvIPlanetParser::instance().lookUpSatellite(&name);
    ASSERT(pController, "An invalid satellite name was parsed.");

    pSky_->controller(pController);
    controllerSet_ = true;
}

void EnvISkyDeclaration::DynamicSky::colourTable(MexDegrees at, const string& name)
{
    EnvElevationColourTable* pElevationClut = EnvIPlanetParser::instance().lookUpClut(&name);
    ASSERT(pElevationClut, "An invalid elevation clut name was parsed.");

    pSky_->colourTable(at, pElevationClut);
    atLeastOneColourTableSet_ = true;
}

void EnvISkyDeclaration::DynamicSky::completeSky()
{
    PRE(meshFileSet_);
    PRE(controllerSet_);
    PRE(atLeastOneColourTableSet_);

    EnvIPlanetParser::instance().skyDeclaration().setCompleteSky(pSky_);
}

EnvISkyDeclaration::DynamicSky::DynamicSky(EnvDynamicSky* const pSky)

    : meshFileSet_(false)
    , controllerSet_(false)
    , atLeastOneColourTableSet_(false)
    , pSky_(pSky)
{
    PRE(pSky);
    PRE(! pSky_->controller());
}

////////////////////////////////////////////////////////////

EnvISkyDeclaration::EnvISkyDeclaration()

    : pCompletedSky_(nullptr)
    , pUniformSky_(nullptr)
    , pStaticSky_(nullptr)
    , pDynamicSky_(nullptr)
{
}

EnvISkyDeclaration::~EnvISkyDeclaration()
{
    TEST_INVARIANT;

    clear();
}

void EnvISkyDeclaration::createUniformSkyDeclaration(EnvUniformSky* const pSky)
{
    PRE(isClear());

    pUniformSky_ = new UniformSky(pSky);

    POST(isUniformSkyDeclaration());
}

void EnvISkyDeclaration::createStaticSkyDeclaration(EnvStaticSky* const pSky)
{
    PRE(isClear());

    pStaticSky_ = new StaticSky(pSky);

    POST(isStaticSkyDeclaration());
}

void EnvISkyDeclaration::createDynamicSkyDeclaration(EnvDynamicSky* const pSky)
{
    PRE(isClear());

    pDynamicSky_ = new DynamicSky(pSky);

    POST(isDynamicSkyDeclaration());
}

void EnvISkyDeclaration::clear()
{
    PRE(isClear() || isComplete());

    pCompletedSky_ = nullptr;

    if (pUniformSky_)
    {
        delete pUniformSky_;
        pUniformSky_ = nullptr;
    }

    if (pStaticSky_)
    {
        delete pStaticSky_;
        pStaticSky_ = nullptr;
    }

    if (pDynamicSky_)
    {
        delete pDynamicSky_;
        pDynamicSky_ = nullptr;
    }
}

const EnvISkyDeclaration::UniformSky& EnvISkyDeclaration::uniformSky() const
{
    PRE(isUniformSkyDeclaration());

    return *pUniformSky_;
}

EnvISkyDeclaration::UniformSky& EnvISkyDeclaration::uniformSky()
{
    PRE(isUniformSkyDeclaration());

    return *pUniformSky_;
}

const EnvISkyDeclaration::StaticSky& EnvISkyDeclaration::staticSky() const
{
    PRE(isStaticSkyDeclaration());

    return *pStaticSky_;
}

EnvISkyDeclaration::StaticSky& EnvISkyDeclaration::staticSky()
{
    PRE(isStaticSkyDeclaration());

    return *pStaticSky_;
}

const EnvISkyDeclaration::DynamicSky& EnvISkyDeclaration::dynamicSky() const
{
    PRE(isDynamicSkyDeclaration());

    return *pDynamicSky_;
}

EnvISkyDeclaration::DynamicSky& EnvISkyDeclaration::dynamicSky()
{
    PRE(isDynamicSkyDeclaration());

    return *pDynamicSky_;
}

bool EnvISkyDeclaration::isUniformSkyDeclaration() const
{
    return (pUniformSky_) ? true : false;
}

bool EnvISkyDeclaration::isStaticSkyDeclaration() const
{
    return (pStaticSky_) ? true : false;
}

bool EnvISkyDeclaration::isDynamicSkyDeclaration() const
{
    return (pDynamicSky_) ? true : false;
}

bool EnvISkyDeclaration::isClear() const
{
    return (pUniformSky_ || pStaticSky_ || pDynamicSky_ || pCompletedSky_) ? false : true;
}

bool EnvISkyDeclaration::isBuilding() const
{
    return ! isComplete() && ! isClear();
}

bool EnvISkyDeclaration::isComplete() const
{
    return (pCompletedSky_) ? true : false;
}

EnvSky* EnvISkyDeclaration::completedSky() const
{
    PRE(isComplete());

    return pCompletedSky_;
}

void EnvISkyDeclaration::CLASS_INVARIANT
{
    INVARIANT(this);
}

void EnvISkyDeclaration::setCompleteSky(EnvSky* const pSky)
{
    PRE(! pCompletedSky_);

    pCompletedSky_ = pSky;
}

/* End SkyDecl.hpp ****************************************/
