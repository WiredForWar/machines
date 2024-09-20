
file(GLOB CTL_SOURCES
    TFlag.hpp
    algorith.hpp
    bitvec.cpp
    bitvec.hpp
    countptr.cpp
    countptr.hpp
    deque.hpp
    erase.hpp
    fixedvec.hpp
    function.hpp
    list.hpp
    map.hpp
    mmvector.hpp
    nbvector.hpp
    pvector.hpp
    set.hpp
    utility.hpp
    vector.hpp
)

add_library(ctl ${GAME_COMPONENT_LIB_TYPE} ${CTL_SOURCES})
add_library(LibDev::ctl ALIAS ctl)
