
set(STDLIB_SOURCES
    exceptio.cpp
    exceptio.hpp
    function.hpp
    memory.hpp
    strfwd.hpp
    string.cpp
    string.hpp
    utility.hpp
)

add_library(stdlib ${GAME_COMPONENT_LIB_TYPE} ${STDLIB_SOURCES})
add_library(LibDev::stdlib ALIAS stdlib)
