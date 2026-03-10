
set(STDLIB_SOURCES
    Exceptions.cpp
    Exceptions.hpp
)

add_library(stdlib ${GAME_COMPONENT_LIB_TYPE} ${STDLIB_SOURCES})
add_library(LibDev::stdlib ALIAS stdlib)
