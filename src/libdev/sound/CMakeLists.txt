
set(SOUND_SOURCES
    alsample.cpp
    alsound.cpp
    internal/alsample.hpp
    internal/alsample.ipp
    internal/alsound.hpp
    internal/dxs.hpp
    internal/sample.hpp
    internal/sample.ipp
    internal/wavefmt.hpp
    internal/wavefmt.ipp
    internal/waveform.hpp
    internal/waveman.hpp
    sample.cpp
    smphandl.hpp
    smpid.hpp
    smpparam.cpp
    smpparam.hpp
    snd.cpp
    snd.hpp
    sndparam.cpp
    sndparam.hpp
    sndwavid.cpp
    sndwavid.hpp
    soundmix.cpp
    soundmix.hpp
    wavefmt.cpp
    waveform.cpp
    waveman.cpp
)

add_library(sound ${GAME_COMPONENT_LIB_TYPE} ${SOUND_SOURCES})
add_library(LibDev::sound ALIAS sound)

target_link_libraries(sound PUBLIC
    LibDev::base

    External::libaudio
    OpenAL::OpenAL
)
