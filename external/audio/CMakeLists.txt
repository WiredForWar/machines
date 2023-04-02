
set(LIBAUDIO_SOURCES
    Amd.h
    Aos.h
    Aosdefs.h
    audio.h
    fileutil.c
    fileutil.h
    wave.c
    wave.h
)

add_library(libaudio STATIC ${LIBAUDIO_SOURCES})
add_library(External::libaudio ALIAS libaudio)

target_include_directories(libaudio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
