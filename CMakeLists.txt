##
# Main CMake project file
# Contains global options and definitions
##

cmake_minimum_required(VERSION 3.14)

project(machines
    VERSION 1.6.0
    LANGUAGES C CXX
)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CMakeDependentOption)

message(STATUS "Building Machines ${machines_VERSION}")

set(BUILD_NUMBER 0)
if(NOT "$ENV{BUILD_NUMBER}" STREQUAL "")
    set(BUILD_NUMBER "$ENV{BUILD_NUMBER}")
    message(STATUS "CI build #${BUILD_NUMBER}")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

##
# Platform detection and some related checks
##
if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    message(STATUS "Build for Windows system")
    set(PLATFORM_WINDOWS 1)
    set(PLATFORM_GNU     0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_OTHER   0)

    # Platform-dependent implementation of system.h
    #set(SYSTEM_CPP_MODULE "system_windows.cpp")
    #set(SYSTEM_H_MODULE "system_windows.h")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    message(STATUS "Build for Linux system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   1)
    set(PLATFORM_GNU     1)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_OTHER   0)

    set(OpenGL_GL_PREFERENCE "GLVND")
    # Platform-dependent implementation of system.h
    #set(SYSTEM_CPP_MODULE "system_linux.cpp")
    #set(SYSTEM_H_MODULE "system_linux.h")
else()
    message(STATUS "Build for other system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_GNU     0)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_OTHER   1)

    # Platform-dependent implementation of system.h
    set(SYSTEM_CPP_MODULE "system_other.cpp")
    set(SYSTEM_H_MODULE "system_other.h")
endif()


##
# Build options
##
# Include cmake directory with some additional scripts
set(CMAKE_MODULE_PATH "${machines_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Build targets should be placed in the root build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# 32 bit build configuration
option(BUILD_32 "Builds 32 bit executable" OFF)
# Include cmake directory with some additions

include(CompilerSettings)

# Asserts can be enabled/disabled regardless of build type
option(DEV_BUILD "Enable assert()s and other debug checks" ON)

# Warn about development build
if(DEV_BUILD)
    message(STATUS "Building with development extensions")
endif()

# Testing
#option(BUILD_TESTS "Build the test suite" off)
cmake_dependent_option(BUILD_TESTS "Build the test suites" ON "DEV_BUILD" OFF)

# Warn about xbawks huge test binaries
if(BUILD_TESTS)
    message(WARNING
        "The testing binaries are HUGE! Each compiled test suite will be around 25MB.\n"
        "If this a problem for you, run \"cmake -DBUILD_TESTS=OFF ..\" to disable.")
endif()

# Export compiler commands when building
#option(BUILD_EXPORT_CC_CMDS "Export the compiler commands when building to use with Sourcetrail" ON)
cmake_dependent_option(BUILD_EXPORT_CC_CMDS "Export the compiler commands when building to use with Sourcetrail" ON "DEV_BUILD" OFF)

# Export the compiler commands when building to use with Sourcetrail
if(BUILD_EXPORT_CC_CMDS)
    if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
        set(CMAKE_EXPORT_COMPILE_COMMANDS
            ON
            CACHE BOOL "Enable output of compile commands during generation." FORCE)
    endif()
    message(STATUS "Export of compile commands is set to: ${CMAKE_EXPORT_COMPILE_COMMANDS}")
endif(BUILD_EXPORT_CC_CMDS)

##
# Searching for packages
##
#set_target_properties(machines PROPERTIES LINK_SEARCH_START_STATIC 1)
#set_target_properties(machines PROPERTIES LINK_SEARCH_END_STATIC 1)
if(BUILD_32)
    # Prefer static libraries
    #set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB32_PATHS ON)
else()
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB32_PATHS OFF)
endif()

find_package(OpenGL 2.1 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Alure REQUIRED)
find_package(ENet REQUIRED)
find_package(Freetype REQUIRED)
find_package(SWScale REQUIRED)


if(NOT DEV_BUILD)
    add_definitions(-DNDEBUG)
    add_definitions(-DPRODUCTION)
endif()

if(PLATFORM_WINDOWS)
  if(NOT BUILD_32)
    find_library(MSVCRT_LIBRARY msvcrt HINTS ${WINDOWS_DEPENDENCIES_DIR})
    find_library(FREETYPE_LIBRARY freetype HINTS ${WINDOWS_DEPENDENCIES_DIR})
    find_library(HARFBUZZ_LIBRARY harfbuzz HINTS ${WINDOWS_DEPENDENCIES_DIR})
    find_library(BZ2_LIBRARY bz2 HINTS ${WINDOWS_DEPENDENCIES_DIR})
    find_library(PNG16_LIBRARY png16 HINTS ${WINDOWS_DEPENDENCIES_DIR})
    find_library(GRAPHITE2_LIBRARY graphite2 HINTS ${WINDOWS_DEPENDENCIES_DIR})
    find_library(Z_LIBRARY z HINTS ${WINDOWS_DEPENDENCIES_DIR})
    find_library(RPCRT4_LIBRARY rpcrt4 HINTS ${WINDOWS_DEPENDENCIES_DIR})
    find_library(USP10_LIBRARY usp10 HINTS ${WINDOWS_DEPENDENCIES_DIR})
    # Windows 64-bit libraries
    set(LIBS
        ${MSVCRT_LIBRARY}
        ${FREETYPE_LIBRARY}
        ${HARFBUZZ_LIBRARY}
        ${BZ2_LIBRARY}
        ${PNG16_LIBRARY}
        ${GRAPHITE2_LIBRARY}
        ${Z_LIBRARY}
        ${RPCRT4_LIBRARY}
        ${USP10_LIBRARY}
    )
  endif()
endif()

##
# Targets
##

# Subdirectory with sources
add_subdirectory(external)
add_subdirectory(src)

# TESTS
if(BUILD_TESTS)
    message(STATUS "Unit tests are enabled.")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/refs/tags/release-1.10.0.zip
    )

    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
      FetchContent_Populate(googletest)

      add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    if(PLATFORM_WINDOWS)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif(PLATFORM_WINDOWS)
    enable_testing()
    add_subdirectory(test)
endif(BUILD_TESTS)

# Distribution Target
# A directory that can be copied into your installation folder
set(DISTRIBUTION_DIR "${CMAKE_BINARY_DIR}/distribution")
add_custom_target(distribution
                    DEPENDS machines planeted)
add_custom_command(TARGET distribution
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${DISTRIBUTION_DIR}/data
                    COMMENT "Copying GAME-data files to distribution directory" VERBATIM)
add_custom_command(TARGET distribution
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/sounds ${DISTRIBUTION_DIR}/sounds
                    COMMENT "Copying SOUND-data files to distribution directory" VERBATIM)
add_custom_command(TARGET distribution
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/gui ${DISTRIBUTION_DIR}/gui
                    COMMENT "Copying GUI graphics files to distribution directory" VERBATIM)
add_custom_command(TARGET distribution
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/models ${DISTRIBUTION_DIR}/models
                    COMMENT "Copying WORLD4D-data and graphics files to distribution directory" VERBATIM)
add_custom_command(TARGET distribution
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:machines> ${DISTRIBUTION_DIR}/$<TARGET_FILE_NAME:machines>
                    COMMENT "Copying machines binary to distribution directory" VERBATIM)
add_custom_command(TARGET distribution
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:planeted> ${DISTRIBUTION_DIR}/$<TARGET_FILE_NAME:planeted>
                    COMMENT "Copying planeted binary to distribution directory" VERBATIM)
