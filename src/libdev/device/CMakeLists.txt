
set(DEVICE_SOURCES
    DevCDImpl.hpp
    KeyBind.hpp
    KeyWithModifiers.hpp
    SdlDelegate.hpp
    ButtonEvent.cpp
    ButtonEvent.hpp
    CD.cpp
    CD.hpp
    OggStream.cpp
    OggStream.hpp
    CDHelper.cpp
    CDHelper.hpp
    CDDefs.hpp
    CDPlayList.cpp
    CDPlayList.hpp
    EventQueue.cpp
    EventQueue.hpp
    internal/SdlTimeImpl.hpp
    Key.hpp
    Keyboard.hpp
    KeyToCommand.cpp
    KeyToCommand.hpp
    KeyToCommandTranslator.cpp
    KeyToCommandTranslator.hpp
    Mouse.hpp
    private/SdlKeyboard.hpp
    private/SdlMouse.hpp
    private/SdlTime.hpp
    private/SharedKeyboard.hpp
    private/SharedMouse.hpp
    Rate.cpp
    Rate.hpp
    SdlKeyboard.cpp
    SdlMouse.cpp
    SdlTime.cpp
    SdlTimeImpl.cpp
    SharedKeyboard.cpp
    SharedMouse.cpp
    Time.cpp
    Time.hpp
    Timer.cpp
    Timer.hpp
)

add_library(device ${GAME_COMPONENT_LIB_TYPE} ${DEVICE_SOURCES})
add_library(LibDev::device ALIAS device)

target_link_libraries(device PUBLIC
    SDL2::SDL2
)

target_link_libraries(device PRIVATE
    OpenAL::OpenAL

    External::stb_vorbis

    spdlog::spdlog
)
