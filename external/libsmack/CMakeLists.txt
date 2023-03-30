
project(smacker
    LANGUAGES C
    VERSION 1.2.0.43 #r43
)

set(LIBSMACKER_SOURCES
    smacker.c
    smacker.h
    smk_malloc.h
)

add_library(libsmacker STATIC ${LIBSMACKER_SOURCES})
add_library(External::LibSmacker ALIAS libsmacker)

target_include_directories(libsmacker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
