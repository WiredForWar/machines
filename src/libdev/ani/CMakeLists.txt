
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

add_library(ani OBJECT ${ANI_SOURCES})

target_compile_definitions(ani PUBLIC USE_SWSCALE=$<BOOL:${USE_SWSCALE}>)
