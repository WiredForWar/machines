
set(CRASHDUMP_SOURCES
    CrashDump.cpp
    CrashDump.hpp
    SafeFormat.cpp
    internal/CrashDumpInternal.hpp
    internal/ReportWriter.hpp
    internal/SafeFormat.hpp
    internal/StackWalk.hpp
)

if(PLATFORM_WINDOWS)
    list(APPEND CRASHDUMP_SOURCES
        CrashDumpWindows.cpp
        ReportWriterWindows.cpp
        StackWalkWindows.cpp
    )
else()
    list(APPEND CRASHDUMP_SOURCES
        CrashDumpPosix.cpp
        ReportWriterPosix.cpp
        StackWalkPosix.cpp
    )
endif()

add_library(crashdump ${GAME_COMPONENT_LIB_TYPE} ${CRASHDUMP_SOURCES})
add_library(LibDev::crashdump ALIAS crashdump)

# Deliberately no dependency on the rest of the engine: a handler that has to
# call into the code that is dying is a handler that can die with it.

if(PLATFORM_WINDOWS)
    # For MiniDumpWriteDump only. The stack walk uses the unwinder in ntdll, so
    # that it needs no symbol handler and takes no loader lock.
    target_link_libraries(crashdump PRIVATE dbghelp)
endif()
