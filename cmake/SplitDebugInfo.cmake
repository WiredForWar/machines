# Keep a release binary's debug information in a file of its own.
#
# A stripped binary turns a crash report into a list of addresses that nothing
# can resolve. Stripping at link time with `-s` throws that information away for
# good; splitting it out keeps it, at the cost of one artifact per executable
# that CI uploads and nobody has to ship to players.
#
# split_debug_info(<target>) writes <target>.debug beside the executable, strips
# the executable, and records the pair with a .gnu_debuglink section so that gdb
# and addr2line find the second from the first without being told.
#
# MACHINES_SPLIT_DEBUG_INFO says whether the tools to do it are present. It is
# what decides whether `-s` is still needed at link time: the two must not both
# happen, or there is nothing left to split.

include_guard(GLOBAL)

set(MACHINES_SPLIT_DEBUG_INFO FALSE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MACHINES_CLANG_CL))
    # CMAKE_OBJCOPY and CMAKE_STRIP are already resolved for the toolchain in
    # use, which is what makes this work for a mingw cross build without naming
    # the prefix here.
    if(CMAKE_OBJCOPY AND CMAKE_STRIP)
        set(MACHINES_SPLIT_DEBUG_INFO TRUE)
    else()
        message(STATUS "objcopy or strip not found: release binaries will be stripped without keeping their symbols")
    endif()
endif()

function(split_debug_info target)
    if(NOT MACHINES_SPLIT_DEBUG_INFO)
        return()
    endif()

    # A debug build keeps its symbols in the binary, which is the point of it.
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        return()
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_OBJCOPY}" --only-keep-debug "$<TARGET_FILE:${target}>" "$<TARGET_FILE:${target}>.debug"
        COMMAND "${CMAKE_STRIP}" --strip-debug --strip-unneeded "$<TARGET_FILE:${target}>"
        COMMAND "${CMAKE_OBJCOPY}" "--add-gnu-debuglink=$<TARGET_FILE_NAME:${target}>.debug" "$<TARGET_FILE:${target}>"
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Splitting the debug information out of ${target}"
        VERBATIM
    )
endfunction()
