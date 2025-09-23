
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

add_library(machines_version_iface INTERFACE)
target_link_libraries(machines_version_iface INTERFACE machines_version $<TARGET_OBJECTS:machines_version>)
add_library(LibDev::version ALIAS machines_version_iface)
