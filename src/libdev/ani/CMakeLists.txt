
set(ANI_SOURCES
    AniSmacker.hpp
    AniSmackerRegular.cpp
    AniSmackerRegular.hpp
)

add_library(ani OBJECT ${ANI_SOURCES})

if(USE_SWSCALE)
    target_compile_definitions(ani PUBLIC USE_SWSCALE=1)
    target_sources(ani PUBLIC
        AniSmackerCutscene.cpp
        AniSmackerCutscene.hpp
        FrameScaler.cpp
        FrameScaler.hpp
    )
else()
    target_compile_definitions(ani PUBLIC USE_SWSCALE=0)
endif()
