
set(SYSTEM_SOURCES
    ConfigVariables.cpp
    ConfigVariables.hpp
    Console.cpp
    Console.hpp
    ConsoleBuiltins.cpp
    ConsoleBuiltins.hpp
    Endian.hpp
    IConsole.cpp
    IConsole.hpp
    SysInfo.cpp
    SysInfo.hpp
    Variable.cpp
    Variable.hpp
    Variable_p.hpp
    FileData.cpp
    FileData.hpp
    FileDate.cpp
    FileDate.hpp
    FileEnumerator.cpp
    FileEnumerator.hpp
    FileFinder.cpp
    FileFinder.hpp
    FileTime.cpp
    FileTime.hpp
    internal/MetaFileImpl.hpp
    internal/PathNameImpl.hpp
    MemoryCaps.cpp
    MemoryCaps.hpp
    PathName.cpp
    PathName.hpp
    Registry.cpp
    Registry.hpp
    VFS.cpp
    VFS.hpp
    WindowsAPI.cpp
    WindowsAPI.hpp
)

add_library(system ${GAME_COMPONENT_LIB_TYPE} ${SYSTEM_SOURCES})
add_library(LibDev::system ALIAS system)

target_link_libraries(system PUBLIC
    LibDev::recorder
    SDL3::SDL3
)

if(PLATFORM_WINDOWS)
    target_link_libraries(system PUBLIC
        version
    )
endif()
