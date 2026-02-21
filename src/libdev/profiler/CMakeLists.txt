
set(PROFILER_SOURCES
    profiler.cpp
    profiler.hpp
    stktrace.cpp
    stktrace.hpp
)

add_library(profiler OBJECT ${PROFILER_SOURCES})
