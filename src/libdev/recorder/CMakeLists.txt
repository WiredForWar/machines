
set(RECORDER_SOURCES
    internal/RecorderImpl.hpp
    private/RecorderPrivate.ctf
    private/RecorderPrivate.hpp
    private/RecorderPrivate.itf
    RecorderImpl.cpp
    Recorder.cpp
    Recorder.hpp
    RecorderPrivate.cpp
)

add_library(recorder ${GAME_COMPONENT_LIB_TYPE} ${RECORDER_SOURCES})
add_library(LibDev::recorder ALIAS recorder)

target_link_libraries(recorder PUBLIC
    LibDev::device
)
