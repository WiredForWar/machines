
set(WINDOW_SUPPORT_SDL_SOURCES
    SDLWindowAdapter.cpp
    SDLWindowAdapter.hpp
)

add_library(window_support_sdl ${GAME_COMPONENT_LIB_TYPE} ${WINDOW_SUPPORT_SDL_SOURCES})
add_library(LibDev::window_support_sdl ALIAS window_support_sdl)

target_include_directories(window_support_sdl PUBLIC
    ${CMAKE_SOURCE_DIR}/src/libdev
)

target_link_libraries(window_support_sdl PUBLIC
    LibDev::render
)

target_link_libraries(window_support_sdl PRIVATE
    SDL3::SDL3
    spdlog::spdlog
)
