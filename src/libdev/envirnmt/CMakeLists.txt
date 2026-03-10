set(ENVIRNMT_SOURCES
    ElevationColourTable.cpp
    ElevationColourTable.hpp
    ElevationMaterialPlan.cpp
    ElevationMaterialPlan.hpp
    internal/OrbitParams.hpp
    internal/PlanetParser.hpp
    internal/SatelliteParams.hpp
    internal/SkyDeclarations.hpp
    Lexer.cpp
    Orbit.cpp
    Orbit.hpp
    OrbitParams.cpp
    Parser.cpp
    PlanetEnvironment.cpp
    PlanetEnvironment.hpp
    PlanetParser.cpp
    Satellite.cpp
    Satellite.hpp
    SatelliteParams.cpp
    Sky.cpp
    SkyDeclarations.cpp
    Sky.hpp
    Sky.ipp
    Skyline.cpp
    Skyline.hpp
)

add_library(envirnmt STATIC ${ENVIRNMT_SOURCES})
add_library(LibDev::environment ALIAS envirnmt)

set_source_files_properties(
    Lexer.cpp
    Parser.cpp
    PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION
            ON
)

target_link_libraries(envirnmt PUBLIC
    LibDev::render
    LibDev::world4d
)
