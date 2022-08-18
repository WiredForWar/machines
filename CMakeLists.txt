##
# Main CMake project file
# Contains global options and definitions
##

cmake_minimum_required(VERSION 3.19)

project(Machines
    VERSION 1.7.2
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

set(CMAKE_CXX_STANDARD 20)

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
include(DefaultOptions)

# Options for optional libs
option_on_off_auto(BUILD_WITH_SWSCALE "Build with SWScale support" "Auto")
option_on_off_auto(BUILD_WITH_ALURE "Build with Alure support" "Auto")

option(USE_CONAN "Use Conan for dependencies" OFF)

if(USE_CONAN)
    set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}" ${CMAKE_MODULE_PATH})
    set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}" ${CMAKE_PREFIX_PATH})

    set(CMAKE_CXX_STANDARD 14)

    include(cmake/conan.cmake)

    if(PLATFORM_LINUX)
        set(CONAN_PLATFORM_DEPS
            libalsa/1.2.7.2
            wayland/1.21.0 # (override for 'vulkan-loader/1.3.221' / 'sdl/2.0.20')
        )
    endif()

    conan_cmake_configure(
        REQUIRES
            sdl/2.0.20
            sdl_image/2.0.5
            glew/2.2.0
            glm/0.9.9.8
            openal/1.22.2
            enet/1.3.17
            freetype/2.11.1
            # swscale:
            ffmpeg/5.0
            ${CONAN_PLATFORM_DEPS}
            # Overrides:
            # libfdk_aac/2.0.1
            openssl/1.1.1q
            libwebp/1.2.3
            zlib/1.2.12
        OPTIONS
            # *:shared=False
            # openssl:shared=True
        GENERATORS
            cmake_find_package
    )

    conan_cmake_autodetect(settings)

    conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings})
endif()

# 32 bit build configuration
option(BUILD_32 "Builds 32 bit executable" OFF)
# Include cmake directory with some additions

# Asserts can be enabled/disabled regardless of build type
option(DEV_BUILD "Enable assert()s and other debug checks" OFF)
option(USE_STANDALONE_LAYOUT "Install the executable and data into the same dir" ${DEFAULT_USE_STANDALONE_LAYOUT})

set(WARNINGS_AS_ERRORS FALSE)
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
option(BUILD_PLANETED "Build the PlanetEditor" OFF)

# Warn about development build
if(DEV_BUILD)
    message(STATUS "Building with development extensions")
endif()

include(CompilerSettings)
include(StaticAnalyzers)

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

add_library(machines_options INTERFACE)
if(ENABLE_CLANG_TIDY)
    enable_clang_tidy(machines_options ${WARNINGS_AS_ERRORS})
endif()

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

find_package(enet REQUIRED)
find_package(Freetype REQUIRED)

if(USE_CONAN)
    set(BUILD_WITH_FFMPEG ${BUILD_WITH_SWSCALE})
    find_optional_dependency(ffmpeg COMPONENTS swscale)
    set(USE_SWSCALE ${USE_FFMPEG})

    set(GLEW_LIBRARY GLEW::GLEW)
    set(FREETYPE_LIBRARY Freetype::Freetype)
else()
    find_optional_dependency(SWScale)

    # CMake Freetype find package sets only the capital-case variable
    set(Freetype_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
endif()

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

if(PLATFORM_WINDOWS AND NOT USE_CONAN)
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
if(USE_STANDALONE_LAYOUT)
    set(BIN_PATH ".")
    set(DATA_PATH ".")
else()
    include(GNUInstallDirs)
    set(BIN_PATH "${CMAKE_INSTALL_BINDIR}")
    set(DATA_PATH "${CMAKE_INSTALL_DATAROOTDIR}/machines")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT USE_STANDALONE_LAYOUT)
    install(FILES "packaging/machines.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications/")
    install(FILES "packaging/machines.appdata.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo/")
    install(FILES "packaging/machines-64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps/" RENAME "machines.png")
endif()

install(DIRECTORY "data" DESTINATION ${DATA_PATH})
install(DIRECTORY "sounds" DESTINATION ${DATA_PATH})
install(DIRECTORY "gui" DESTINATION ${DATA_PATH})
install(DIRECTORY "models" DESTINATION ${DATA_PATH})
install(FILES "machstrg.xml" DESTINATION ${DATA_PATH})

install(TARGETS machines DESTINATION ${BIN_PATH})
if(TARGET planeted)
    install(TARGETS planeted DESTINATION ${BIN_PATH})
endif()

set(CPACK_GENERATOR ZIP)
include(CPack)
