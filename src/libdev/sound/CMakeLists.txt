
set(SOUND_SOURCES
    ALSample.cpp
    ALSound.cpp
    internal/ALSample.hpp
    internal/ALSample.ipp
    internal/ALSound.hpp
    internal/dxs.hpp
    internal/Sample.hpp
    internal/Sample.ipp
    internal/WaveFormat.hpp
    internal/WaveFormat.ipp
    internal/Waveform.hpp
    internal/WaveManager.hpp
    Sample.cpp
    SampleHandle.hpp
    SampleId.hpp
    SampleParameters.cpp
    SampleParameters.hpp
    snd.cpp
    snd.hpp
    MixerParameters.cpp
    MixerParameters.hpp
    WaveformId.cpp
    WaveformId.hpp
    Mixer.cpp
    Mixer.hpp
    WaveFormat.cpp
    Waveform.cpp
    WaveManager.cpp
)

add_library(sound ${GAME_COMPONENT_LIB_TYPE} ${SOUND_SOURCES})
add_library(LibDev::sound ALIAS sound)

target_link_libraries(sound PUBLIC
    LibDev::base
    LibDev::system

    External::libaudio
    OpenAL::OpenAL
)
