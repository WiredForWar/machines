
set(SIM_SOURCES
    Action.cpp
    Action.hpp
    Actor.cpp
    Actor.hpp
    Actor.ipp
    ConditionsManager.cpp
    ConditionsManager.hpp
    Condition.cpp
    Condition.hpp
    DestructEntityEvent.cpp
    DestructEntityEvent.hpp
    DiscreteEvent.cpp
    DiscreteEvent.hpp
    DiscreteEvent.ipp
    EventDiary.cpp
    EventDiary.hpp
    EventDiary.ipp
    internal/ManagerImpl.hpp
    Manager.cpp
    Manager.hpp
    ManagerImpl.cpp
    Process.cpp
    Process.hpp
    Projectile.cpp
    Projectile.hpp
    sim.hpp
    Stats.cpp
    Stats.hpp
)

add_library(sim STATIC ${SIM_SOURCES})
add_library(LibDev::sim ALIAS sim)

target_link_libraries(sim PUBLIC
    LibDev::network
    LibDev::world4d
)
