
project(stb_vorbis
    LANGUAGES C
)

set(STB_VORBIS_SOURCES
    stb_vorbis.c
)

add_library(stb_vorbis STATIC ${STB_VORBIS_SOURCES})
add_library(External::stb_vorbis ALIAS stb_vorbis)

target_include_directories(stb_vorbis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
