
set(RECORDER_SOURCES
    internal/recimpl.hpp
    private/recpriv.ctf
    private/recpriv.hpp
    private/recpriv.itf
    recimpl.cpp
    recorder.cpp
    recorder.hpp
    recpriv.cpp
)

add_library(recorder ${GAME_COMPONENT_LIB_TYPE} ${RECORDER_SOURCES})
add_library(LibDev::recorder ALIAS recorder)

target_link_libraries(recorder PUBLIC
    LibDev::device
)
