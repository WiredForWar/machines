
set(CRASHDUMP_SOURCES
    CrashDump.cpp
    CrashDump.hpp
    CrashTest.cpp
    CrashTest.hpp
    SafeFormat.cpp
    Watchdog.cpp
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

# The watchdog runs on a thread of its own, and asks the watched thread for its
# stack through a signal on POSIX.
find_package(Threads REQUIRED)
target_link_libraries(crashdump PUBLIC Threads::Threads)

# Deliberately no dependency on the rest of the engine: a handler that has to
# call into the code that is dying is a handler that can die with it.

if(PLATFORM_WINDOWS)
    # For MiniDumpWriteDump only. The stack walk uses the unwinder in ntdll, so
    # that it needs no symbol handler and takes no loader lock.
    target_link_libraries(crashdump PRIVATE dbghelp)

    # These three are Microsoft extensions to the C runtime rather than standard
    # functions, and the game ships built against the mingw-w64 runtime rather
    # than the Microsoft one, so each is checked instead of assumed. A missing
    # one costs the coverage it would have given and nothing else.
    include(CheckCXXSourceCompiles)

    check_cxx_source_compiles("
        #include <cstdlib>
        int main() { _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT); }
    " HAVE_SET_ABORT_BEHAVIOR)

    check_cxx_source_compiles("
        #include <cstdlib>
        static void handler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t) { }
        int main() { _set_invalid_parameter_handler(handler); }
    " HAVE_SET_INVALID_PARAMETER_HANDLER)

    check_cxx_source_compiles("
        #include <cstdlib>
        static void handler() { }
        int main() { _set_purecall_handler(handler); }
    " HAVE_SET_PURECALL_HANDLER)

    foreach(feature HAVE_SET_ABORT_BEHAVIOR HAVE_SET_INVALID_PARAMETER_HANDLER HAVE_SET_PURECALL_HANDLER)
        if(${feature})
            target_compile_definitions(crashdump PRIVATE ${feature})
        endif()
    endforeach()
endif()
