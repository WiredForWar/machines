
set(SIM_SOURCES
    action.cpp
    action.hpp
    actor.cpp
    actor.hpp
    actor.ipp
    conditim.cpp
    conditim.hpp
    conditio.cpp
    conditio.hpp
    destenty.cpp
    destenty.hpp
    disevent.cpp
    disevent.hpp
    disevent.ipp
    evdiary.cpp
    evdiary.hpp
    evdiary.ipp
    internal/manageri.hpp
    manager.cpp
    manager.hpp
    manageri.cpp
    process.cpp
    process.hpp
    projecti.cpp
    projecti.hpp
    sim.hpp
    simstats.cpp
    simstats.hpp
)

add_library(sim STATIC ${SIM_SOURCES})
add_library(LibDev::sim ALIAS sim)

target_link_libraries(sim PUBLIC
    LibDev::network
    LibDev::world4d
)
