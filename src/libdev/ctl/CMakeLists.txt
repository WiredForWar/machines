
file(GLOB CTL_SOURCES
    TFlag.hpp
    Algorithm.hpp
    BitVector.cpp
    BitVector.hpp
    CountedPtr.cpp
    CountedPtr.hpp
    Deque.hpp
    Erase.hpp
    FixedVector.hpp
    Function.hpp
    List.hpp
    Map.hpp
    MinMemoryVector.hpp
    NoBoundsVector.hpp
    PtrVector.hpp
    Utility.hpp
    Vector.hpp
)

add_library(ctl ${GAME_COMPONENT_LIB_TYPE} ${CTL_SOURCES})
add_library(LibDev::ctl ALIAS ctl)
