
add_subdirectory(audio)
add_subdirectory(libsmack)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

set(SPDLOG_DISABLE_DEFAULT_LOGGER ON FORCE)
add_subdirectory(spdlog)

add_subdirectory(stb_vorbis)
