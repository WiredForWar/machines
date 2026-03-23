
add_subdirectory(audio)
add_subdirectory(libsmack)

set(SPDLOG_DISABLE_DEFAULT_LOGGER ON FORCE)
add_subdirectory(spdlog)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

set(HTTPLIB_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(cpp-httplib)
