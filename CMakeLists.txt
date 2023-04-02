##
# Main CMake project file
# Contains global options and definitions
##

cmake_minimum_required(VERSION 3.14)

project(Machines
    VERSION 1.6.0
    LANGUAGES C CXX
)
include(CMakeDependentOption)

message(STATUS "Building Machines ${Machines_VERSION}")
message(STATUS "CMake version: ${CMAKE_VERSION}")

if (NOT BUILD_VERSION)
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE BUILD_VERSION
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        set(BUILD_VERSION "unknown")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
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
set(CMAKE_MODULE_PATH "${Machines_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Build targets should be placed in the root build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

include(Utils)

# Options for optional libs
option_on_off_auto(BUILD_WITH_SWSCALE "Build with SWScale support" "Auto")
option_on_off_auto(BUILD_WITH_ALURE "Build with Alure support" "Auto")

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
find_package(glm REQUIRED)
include(EnsureOpenALTarget)

find_optional_dependency(Alure)
find_optional_dependency(SWScale)

find_package(enet REQUIRED)
find_package(Freetype REQUIRED)

if(NOT DEV_BUILD)
    add_definitions(-DNDEBUG)
    add_definitions(-DPRODUCTION)
endif()

# CMake Freetype find package sets only the capital-case variable
set(Freetype_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})

# Some glm versions export only `glm` target
if(TARGET glm AND NOT TARGET glm::glm)
    add_library(glm::glm INTERFACE IMPORTED)
    target_link_libraries(glm::glm INTERFACE glm)
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
        ${PNG16_LIBRARY}
        ${Z_LIBRARY}
        ${RPCRT4_LIBRARY}
        ${USP10_LIBRARY}
    )
  endif()

  target_link_libraries(Freetype::Freetype INTERFACE
      ${PNG16_LIBRARY}
      ${HARFBUZZ_LIBRARY}
      ${GRAPHITE2_LIBRARY}
      ${FREETYPE_LIBRARY}
      ${BZ2_LIBRARY}
      ${Z_LIBRARY}
      ${RPCRT4_LIBRARY}
      ${USP10_LIBRARY}
  )
endif()

# Compile flags as defined in global CMakeLists
set(CMAKE_CXX_FLAGS "${MACHINES_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${MACHINES_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE ${MACHINES_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_DEBUG ${MACHINES_CXX_FLAGS_DEBUG})

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
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
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

# Install
install(DIRECTORY "data" DESTINATION ".")
install(DIRECTORY "sounds" DESTINATION ".")
install(DIRECTORY "gui" DESTINATION ".")
install(DIRECTORY "models" DESTINATION ".")
install(FILES "machstrg.xml" DESTINATION ".")
install(TARGETS machines planeted DESTINATION ".")

set(CPACK_GENERATOR ZIP)
include(CPack)
