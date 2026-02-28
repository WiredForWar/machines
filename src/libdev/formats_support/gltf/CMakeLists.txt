
set(GLTF_LOADER_SOURCES
    GltfMeshLoader.cpp
    GltfMeshLoader.hpp
)

add_library(gltf_loader ${GAME_COMPONENT_LIB_TYPE} ${GLTF_LOADER_SOURCES})
add_library(FormatSupport::gltf ALIAS gltf_loader)

target_include_directories(gltf_loader PUBLIC
    ${CMAKE_SOURCE_DIR}/src/libdev
)

target_link_libraries(gltf_loader PUBLIC
    format_support_iface
)

target_link_libraries(gltf_loader PRIVATE
    LibDev::mathex
    LibDev::system
    LibDev::utility
    tinygltf::tinygltf
)

target_compile_definitions(gltf_loader INTERFACE
    HAS_GLTF_LOADER
)
