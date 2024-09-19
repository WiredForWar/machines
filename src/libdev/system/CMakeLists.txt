
set(SYSTEM_SOURCES
    Endian.hpp
    SysInfo.cpp
    SysInfo.hpp
    filedata.cpp
    filedata.hpp
    filedate.cpp
    filedate.hpp
    fileenum.cpp
    fileenum.hpp
    filefind.cpp
    filefind.hpp
    filetime.cpp
    filetime.hpp
    internal/metafili.hpp
    internal/pathnami.hpp
    memcaps.cpp
    memcaps.hpp
    metafile.cpp
    metafile.hpp
    metaistr.cpp
    metaistr.hpp
    pathname.cpp
    pathname.hpp
    registry.cpp
    registry.hpp
    winapi.cpp
    winapi.hpp
)

add_library(system ${GAME_COMPONENT_LIB_TYPE} ${SYSTEM_SOURCES})
add_library(LibDev::system ALIAS system)

target_link_libraries(system PUBLIC
    LibDev::recorder
    SDL2::SDL2
)

if(PLATFORM_WINDOWS)
    target_link_libraries(system PUBLIC
        version
    )
endif()
