
add_library(tinygltf)
target_sources(tinygltf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tiny_gltf.cc)
target_include_directories(tinygltf
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(NOT TARGET nlohmann_json::nlohmann_json)
    message(SEND_ERROR "nlohmann_json not found")
endif()

target_link_libraries(tinygltf PUBLIC
    nlohmann_json::nlohmann_json
)

add_library(tinygltf::tinygltf ALIAS tinygltf)
