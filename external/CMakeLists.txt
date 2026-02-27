
add_subdirectory(audio)
add_subdirectory(libsmack)

set(SPDLOG_DISABLE_DEFAULT_LOGGER ON FORCE)
add_subdirectory(spdlog)

add_subdirectory(nlohmann_json)

add_subdirectory(tinygltf)
