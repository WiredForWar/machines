
set(DEVICE_SOURCES
    DevCDImpl.hpp
    SdlDelegate.hpp
    butevent.cpp
    butevent.hpp
    cd.cpp
    cd.hpp
    cd_helper.cpp
    cd_helper.hpp
    cddefs.hpp
    cdlist.cpp
    cdlist.hpp
    eventq.cpp
    eventq.hpp
    internal/sdltimei.hpp
    key.hpp
    keyboard.hpp
    keycomnd.cpp
    keycomnd.hpp
    keytrans.cpp
    keytrans.hpp
    mouse.hpp
    private/sdlkeybd.hpp
    private/sdlmouse.hpp
    private/sdltime.hpp
    private/shrkeybd.hpp
    private/shrmouse.hpp
    private/w95keybd.hpp
    private/w95mouse.hpp
    private/w95time.hpp
    rate.cpp
    rate.hpp
    sdlkeybd.cpp
    sdlmouse.cpp
    sdltime.cpp
    sdltimei.cpp
    shrkeybd.cpp
    shrmouse.cpp
    time.cpp
    time.hpp
    timer.cpp
    timer.hpp
)

add_library(device ${GAME_COMPONENT_LIB_TYPE} ${DEVICE_SOURCES})
add_library(LibDev::device ALIAS device)

target_link_libraries(device PUBLIC
    SDL2::SDL2
)

target_link_libraries(device PRIVATE
    OpenAL::OpenAL

    spdlog::spdlog
)

if(TARGET Alure::Alure)
    target_link_libraries(device PRIVATE Alure::Alure)
    target_compile_definitions(device PRIVATE USE_ALURE=1)
else()
    target_compile_definitions(device PRIVATE USE_ALURE=0)
endif()
