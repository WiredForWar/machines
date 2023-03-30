
set(ANI_SOURCES
    AniSmacker.hpp
    AniSmackerRegular.cpp
    AniSmackerRegular.hpp
)

if(USE_SWSCALE)
    list(APPEND ANI_SOURCES
        AniSmackerCutscene.cpp
        AniSmackerCutscene.hpp
        FrameScaler.cpp
        FrameScaler.hpp
    )
endif()

add_library(ani STATIC ${ANI_SOURCES})
add_library(LibDev::ani ALIAS ani)

target_link_libraries(ani PUBLIC
    LibDev::render
)

target_link_libraries(ani PRIVATE
    External::LibSmacker
)

if(USE_SWSCALE)
    target_link_libraries(ani PUBLIC
        ffmpeg::swscale
    )
endif()

target_compile_definitions(ani PUBLIC USE_SWSCALE=$<BOOL:${USE_SWSCALE}>)
