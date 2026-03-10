
set(FTL_SOURCES
    Collection.hpp
    Container.hpp
    ftl.hpp
    Map.hpp
    ReusingIdGenerator.hpp
    SerialId.hpp
    SerialMap.ctp
    SerialMap.hpp
    SerialMap.itp
    SerialPMap.ctp
    SerialPMap.hpp
    SerialPMap.itp
)

add_library(ftl INTERFACE)
target_sources(ftl INTERFACE ${FTL_SOURCES})
