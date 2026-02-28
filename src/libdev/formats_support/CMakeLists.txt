
# Interface-only target for the shared headers (IMeshLoader.hpp, MeshData.hpp).
add_library(format_support_iface INTERFACE)
target_sources(format_support_iface INTERFACE
    IMeshLoader.hpp
    MeshData.hpp
)
target_include_directories(format_support_iface INTERFACE
    ${CMAKE_SOURCE_DIR}/src/libdev
)

# Loader submodules
add_subdirectory(agt)
add_subdirectory(xfile)

add_subdirectory(factory)
