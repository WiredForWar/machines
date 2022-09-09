
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

add_library(stdlib OBJECT ${STDLIB_SOURCES})
