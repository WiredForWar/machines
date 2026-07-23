
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(Machines_BUILD_COMPILER "GCC ${CMAKE_CXX_COMPILER_VERSION}")
else()
    set(Machines_BUILD_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
endif()
if(MINGW)
    string(APPEND Machines_BUILD_COMPILER " (MinGW)")
endif()

set(Machines_BUILD_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
if(Machines_BUILD_ARCH STREQUAL "AMD64")
    set(Machines_BUILD_ARCH "x86_64")
endif()

configure_file("machines_version.h.in" "machines_version.h" @ONLY)

set(VERSION_SOURCES
    MachinesVersion.cpp
    MachinesVersion.hpp
    machines_version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/machines_version.h
)

add_library(machines_version OBJECT ${VERSION_SOURCES})
target_include_directories(machines_version PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(machines_version PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# The build type is a generator expression to support multi-config generators,
# for which configure_file() would be too early.
target_compile_definitions(machines_version PRIVATE MACHINES_BUILD_CONFIG="$<CONFIG>")

add_library(machines_version_iface INTERFACE)
target_link_libraries(machines_version_iface INTERFACE machines_version $<TARGET_OBJECTS:machines_version>)
add_library(LibDev::version ALIAS machines_version_iface)
