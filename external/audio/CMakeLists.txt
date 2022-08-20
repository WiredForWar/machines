
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
