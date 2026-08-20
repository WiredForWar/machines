
set(DEVICE_SOURCES
    ButtonEvent.cpp
    ButtonEvent.hpp
    CD.cpp
    CD.hpp
    CDDefs.hpp
    CDHelper.cpp
    CDHelper.hpp
    CDPlayList.cpp
    CDPlayList.hpp
    DevCDImpl.hpp
    EventQueue.cpp
    EventQueue.hpp
    Input.cpp
    Input.hpp
    internal/SdlTimeImpl.hpp
    Key.hpp
    KeyBind.hpp
    Keyboard.cpp
    Keyboard.hpp
    KeyNames.cpp
    KeyNames.hpp
    KeyToCommand.cpp
    KeyToCommand.hpp
    KeyToCommandTranslator.cpp
    KeyToCommandTranslator.hpp
    KeyWithModifiers.hpp
    Mouse.hpp
    MusicResumeStore.cpp
    MusicResumeStore.hpp
    OggStream.cpp
    OggStream.hpp
    private/SdlMouse.hpp
    private/SdlTime.hpp
    private/SharedMouse.hpp
    Rate.cpp
    Rate.hpp
    SdlDelegate.hpp
    SdlKeyCodes.cpp
    SdlKeyCodes.hpp
    SdlMouse.cpp
    SdlTime.cpp
    SdlTimeImpl.cpp
    Time.cpp
    Time.hpp
    Timer.cpp
    Timer.hpp
)

add_library(device ${GAME_COMPONENT_LIB_TYPE} ${DEVICE_SOURCES})
add_library(LibDev::device ALIAS device)

target_link_libraries(device PUBLIC
    SDL3::SDL3
)

target_link_libraries(device PRIVATE
    OpenAL::OpenAL

    External::stb_vorbis

    spdlog::spdlog
)
