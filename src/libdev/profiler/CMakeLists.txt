
set(PROFILER_SOURCES
    Profiler.cpp
    Profiler.hpp
    StackTracer.cpp
    StackTracer.hpp
)

add_library(profiler OBJECT ${PROFILER_SOURCES})
