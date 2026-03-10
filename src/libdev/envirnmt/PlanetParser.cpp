/*
 * P L A P A R S E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "envirnmt/internal/PlanetParser.hpp"
#include "envirnmt/PlanetEnvironment.hpp"
#include "envirnmt/Satellite.hpp" // TBD: remove, only needed for debug.
#include "envirnmt/ElevationColourTable.hpp"
#include "envirnmt/Sky.hpp"
#include "envirnmt/Skyline.hpp"
#include "envirnmt/internal/OrbitParams.hpp"
#include "envirnmt/internal/SatelliteParams.hpp"

#include "base/Diag.hpp"
#include "ctl/PtrVector.hpp"
#include "system/PathName.hpp"
#include <stdio.h>

#include "world4d/Scene/Stars.hpp"

#include <fstream>

EnvIPlanetParser& EnvIPlanetParser::instance()
{
    static EnvIPlanetParser instance_;
    return instance_;
}

EnvIPlanetParser::EnvIPlanetParser()
    : planet_(nullptr)
    , orbParams_(nullptr)
    , pStars_(nullptr)
    , skylineClut_(nullptr)
    , skylineController_(nullptr)
    , contructingSkyline_(false)
{
    TEST_INVARIANT;
}

EnvIPlanetParser::~EnvIPlanetParser()
{
    TEST_INVARIANT;
    delete orbParams_;
}

void EnvIPlanetParser::parse(const SysPathName& envFile, EnvPlanetEnvironment* p)
{
    PRE_INFO(envFile);
    PRE(p);

    // Clear out data from any previous pasring session
    clear();

    extern int yylineno; /* line # of current line, defined in lex.l */
    extern int error_count; /* count of errors, defined in err_skel.c */
    extern int yyparse(); /* defined in err_skel.c */
    extern std::istream* pGlobalLexIstream;

    ASSERT(pGlobalLexIstream == nullptr, "This should be nulled.");

    void yydoinit(); // Defined in lexer.l

    // Bits of the parser require that this is set.
    planet_ = p;

    // Do the global variable YACC stuff.
    yydoinit();

    ASSERT_FILE_EXISTS(envFile.c_str());
    pGlobalLexIstream = new std::ifstream(envFile.c_str(), std::ios::in);

    yylineno = 1;
    yyparse();

    delete pGlobalLexIstream;
    pGlobalLexIstream = nullptr;
    planet_ = nullptr;
}

void EnvIPlanetParser::startOrbit(const std::string* name)
{
    orbParams_ = new EnvIOrbitParams(name);
    POST(orbitInConstruction());
}

EnvIOrbitParams* EnvIPlanetParser::orbitInConstruction()
{
    return orbParams_;
}

void EnvIPlanetParser::orbitComplete()
{
    PRE(planet_);
    PRE(orbitInConstruction());

    EnvOrbit* orbit = planet_->createOrbit(orbParams_);
    std::pair<const std::string, EnvOrbit*> value(orbParams_->name(), orbit);
    orbits_.insert(value);

    delete orbParams_;
    orbParams_ = nullptr;

    POST(!orbitInConstruction());
}

EnvOrbit* EnvIPlanetParser::lookUpOrbit(const std::string* name)
{
    PRE(name);
    OrbitMap::const_iterator it = orbits_.find(*name);

    if (it == orbits_.end())
        return nullptr;
    else
        return (*it).second;
}

void EnvIPlanetParser::startSatellite(const std::string* name)
{
    satParams_ = new EnvISatelliteParams(name);
    POST(satelliteInConstruction());
}

EnvISatelliteParams* EnvIPlanetParser::satelliteInConstruction()
{
    return satParams_;
}

void EnvIPlanetParser::satelliteComplete()
{
    PRE(planet_);
    PRE(satelliteInConstruction());

    std::unique_ptr<EnvSatellite> satellite = planet_->createSatellite(satParams_);
    IAIN_STREAM("Created new " << *satellite << std::endl);

    satellites_.emplace(satParams_->name(), std::move(satellite));

    delete satParams_;
    satParams_ = nullptr;

    POST(!satelliteInConstruction());
}

EnvSatellite* EnvIPlanetParser::lookUpSatellite(const std::string* name)
{
    PRE(name);
    SatelliteMap::const_iterator it = satellites_.find(*name);

    if (it == satellites_.end())
        return nullptr;
    else
        return (*it).second.get();
}

std::vector<std::unique_ptr<EnvSatellite>> EnvIPlanetParser::takeSatellites()
{
    std::vector<std::unique_ptr<EnvSatellite>> result;
    result.reserve(satellites_.size());

    for(auto &[key, value] : satellites_)
    {
        result.emplace_back(std::move(value));
    }
    satellites_.clear();

    return result;
}

void EnvIPlanetParser::addClut(EnvElevationColourTable* clut)
{
    PRE(clut);
    PRE(clut->name().length() > 0);

    std::pair<const std::string, EnvElevationColourTable*> value(clut->name(), clut);
    cluts_.insert(value);
}

EnvElevationColourTable* EnvIPlanetParser::lookUpClut(const std::string* name)
{
    PRE(name);
    ClutMap::const_iterator it = cluts_.find(*name);

    if (it == cluts_.end())
        return nullptr;
    else
        return (*it).second;
}

EnvISkyDeclaration& EnvIPlanetParser::skyDeclaration()
{
    return skyDeclaration_;
}

EnvUniformSky* EnvIPlanetParser::createUniformSky()
{
    PRE(planet_);

    return planet_->createUniformSky();
}

EnvStaticSky* EnvIPlanetParser::createStaticSky()
{
    PRE(planet_);

    return planet_->createStaticSky();
}

EnvDynamicSky* EnvIPlanetParser::createDynamicSky()
{
    PRE(planet_);

    return planet_->createDynamicSky();
}

W4dStars* EnvIPlanetParser::pStars() const
{
    return pStars_;
}

void EnvIPlanetParser::createStars(RenStars::Configuration config, MATHEX_SCALAR radius, int nStars)
{
    PRE(planet_);
    pStars_ = planet_->createStars(config, radius, nStars);
    POST(pStars());
}

void EnvIPlanetParser::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const EnvIPlanetParser& t)
{
    o << "EnvIPlanetParser " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "EnvIPlanetParser " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void EnvIPlanetParser::clear()
{
    orbParams_ = nullptr;
    satParams_ = nullptr;
    pStars_ = nullptr;

    skyDeclaration().clear();

    // Clear out the maps
    if (cluts_.size() != 0)
        cluts_.erase(cluts_.begin(), cluts_.end());

    if (orbits_.size() != 0)
        orbits_.erase(orbits_.begin(), orbits_.end());

    if (satellites_.size() != 0)
        satellites_.erase(satellites_.begin(), satellites_.end());
}

void EnvIPlanetParser::startSkyline()
{
    contructingSkyline_ = true;
    POST(contructingSkyline_);
}

void EnvIPlanetParser::completeSkyline()
{
    PRE(skylineMesh_.length() > 0);
    EnvSkyline* skyline = planet_->createSkyline(skylineMesh_);

    if (skylineClut_ && skylineController_)
        skyline->colourTable(skylineClut_, skylineController_);

    skylineMesh_ = "";
    skylineClut_ = nullptr;
    skylineController_ = nullptr;

    contructingSkyline_ = false;
    POST(!contructingSkyline_);
}

void EnvIPlanetParser::skylineMeshFile(const std::string* m)
{
    PRE(m);
    PRE(contructingSkyline_);
    skylineMesh_ = *m;
}

void EnvIPlanetParser::skylineController(const std::string* s)
{
    PRE(s);
    PRE(contructingSkyline_);
    skylineController_ = lookUpSatellite(s);
    POST(skylineController_);
}

void EnvIPlanetParser::skylineColourTable(const std::string* c)
{
    PRE(c);
    PRE(contructingSkyline_);
    skylineClut_ = lookUpClut(c);
    POST(skylineClut_);
}

/* End PLAPARSE.CPP *************************************************/
