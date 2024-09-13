/*
 * P L A P A R S E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#ifndef _ENVIRNMT_PLAPARSE_HPP
#define _ENVIRNMT_PLAPARSE_HPP

#include "base/base.hpp"

#include "ctl/map.hpp"

#include "stdlib/string.hpp"

#include "mathex/mathex.hpp"
#include "render/stars.hpp"
#include "envirnmt/internal/skydecl.hpp"

class SysPathName;

class EnvOrbit;
class EnvSatellite;
class EnvElevationColourTable;
class EnvPlanetEnvironment;
class EnvUniformSky;
class EnvStaticSky;
class EnvDynamicSky;
class EnvIOrbitParams;
class EnvIClutParams;
class EnvISatelliteParams;

class W4dStars;

template <class T> class ctl_pvector;

class EnvIPlanetParser
{
public:
    //  Singleton class
    static EnvIPlanetParser& instance();
    ~EnvIPlanetParser();

    void parse(const SysPathName&, EnvPlanetEnvironment*);

    // Clear out any stored data following a parse. This leaves the maps etc in virgin state.
    void clear();

    void startOrbit(const std::string*);
    EnvIOrbitParams* orbitInConstruction();
    void orbitComplete();
    EnvOrbit* lookUpOrbit(const std::string*);

    void startSatellite(const std::string*);
    EnvISatelliteParams* satelliteInConstruction();
    void satelliteComplete();
    EnvSatellite* lookUpSatellite(const std::string*);
    void copySatellites(ctl_pvector<EnvSatellite>* vec); // PRE(vec);

    void addClut(EnvElevationColourTable*);
    EnvElevationColourTable* lookUpClut(const std::string*);

    EnvISkyDeclaration& skyDeclaration();
    EnvUniformSky* createUniformSky();
    EnvStaticSky* createStaticSky();
    EnvDynamicSky* createDynamicSky();

    void startSkyline();
    void completeSkyline();
    void skylineMeshFile(const std::string*);
    void skylineController(const std::string*);
    void skylineColourTable(const std::string*);

    W4dStars* pStars() const;
    void createStars(RenStars::Configuration, MATHEX_SCALAR radius, int nStars);

    EnvPlanetEnvironment* planet() { return planet_; }

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const EnvIPlanetParser& t);

private:
    EnvPlanetEnvironment* planet_;
    EnvIOrbitParams* orbParams_;
    EnvISatelliteParams* satParams_;
    EnvISkyDeclaration skyDeclaration_;
    W4dStars* pStars_;

    // Sky-line parameters.
    bool contructingSkyline_;
    std::string skylineMesh_;
    EnvElevationColourTable* skylineClut_;
    EnvSatellite* skylineController_;

    using ClutMap = ctl_map<std::string, EnvElevationColourTable*, std::less<std::string>>;
    using SatelliteMap = ctl_map<std::string, EnvSatellite*, std::less<std::string>>;
    using OrbitMap = ctl_map<std::string, EnvOrbit*, std::less<std::string>>;

    ClutMap cluts_;
    OrbitMap orbits_;
    SatelliteMap satellites_;

    EnvIPlanetParser(const EnvIPlanetParser&);
    EnvIPlanetParser& operator=(const EnvIPlanetParser&);
    bool operator==(const EnvIPlanetParser&);

    EnvIPlanetParser();
};

#endif

/* End PLAPARSE.HPP *************************************************/
