
set(MESH_LOADER_FACTORY_SOURCES
    MeshLoaderFactory.cpp
    MeshLoaderFactory.hpp
)

add_library(mesh_loader_factory ${GAME_COMPONENT_LIB_TYPE} ${MESH_LOADER_FACTORY_SOURCES})
add_library(FormatSupport::factory ALIAS mesh_loader_factory)

target_include_directories(mesh_loader_factory PUBLIC
    ${CMAKE_SOURCE_DIR}/src/libdev
)

target_link_libraries(mesh_loader_factory PUBLIC
    format_support_iface
)
