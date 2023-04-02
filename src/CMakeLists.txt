# Subdirectories

#add_subdirectory(libdev)
#add_subdirectory(projects)



# Source files
# GLOB is sometimes considered anti pattern but watever

set(MACHINES_SOURCES
    projects/machines/errorhnd.cpp
    projects/machines/errorhnd.hpp
    projects/machines/guistuff.cpp
    projects/machines/leaktrak.cpp
    projects/machines/leaktrak.hpp
    projects/machines/loadplan.cpp
    projects/machines/motion.cpp
    projects/machines/scrndump.cpp
    projects/machines/scrndump.hpp
    projects/machines/sdlapp.cpp
    projects/machines/sdlapp.hpp
    projects/machines/simstuff.cpp
    projects/machines/sndstuff.cpp
)

set(PLANETED_SOURCES
    projects/planeted/availods.cpp
    projects/planeted/availods.hpp
    projects/planeted/cameras.cpp
    projects/planeted/cameras.hpp
    projects/planeted/camrmrkr.cpp
    projects/planeted/camrmrkr.hpp
    projects/planeted/datas.hpp
    projects/planeted/domain.cpp
    projects/planeted/domain.hpp
    projects/planeted/editactor.cpp
    projects/planeted/editactor.hpp
    projects/planeted/editart.cpp
    projects/planeted/editart.hpp
    projects/planeted/editcamr.cpp
    projects/planeted/editcamr.hpp
    projects/planeted/editconstr.cpp
    projects/planeted/editconstr.hpp
    projects/planeted/editdoma.cpp
    projects/planeted/editdoma.hpp
    projects/planeted/editmach.cpp
    projects/planeted/editmach.hpp
    projects/planeted/editmode.cpp
    projects/planeted/editmode.hpp
    projects/planeted/editobst.cpp
    projects/planeted/editobst.hpp
    projects/planeted/editpoly.cpp
    projects/planeted/editpoly.hpp
    projects/planeted/editport.cpp
    projects/planeted/editport.hpp
    projects/planeted/edittile.cpp
    projects/planeted/edittile.hpp
    projects/planeted/errorhnd.cpp
    projects/planeted/errorhnd.hpp
    projects/planeted/keyboard.cpp
    projects/planeted/leaktrak.cpp
    projects/planeted/leaktrak.hpp
    projects/planeted/mapbmp.cpp
    projects/planeted/mapbmp.hpp
    projects/planeted/motion.cpp
    projects/planeted/mpreload.cpp
    projects/planeted/mpreload.hpp
    projects/planeted/obstacle.cpp
    projects/planeted/obstacle.hpp
    projects/planeted/planeted.cpp
    projects/planeted/planeted.hpp
    projects/planeted/polygon.cpp
    projects/planeted/polygon.hpp
    projects/planeted/portal.cpp
    projects/planeted/portal.hpp
    projects/planeted/race.cpp
    projects/planeted/race.hpp
    projects/planeted/scenfile.cpp
    projects/planeted/scenfile.hpp
    projects/planeted/sdlapp.cpp
    projects/planeted/sdlapp.hpp
    projects/planeted/strings.hpp
    projects/planeted/testtibo.cpp
    projects/planeted/testtibo.hpp
    projects/planeted/tilemrkr.cpp
    projects/planeted/tilemrkr.hpp
    projects/planeted/vetxmrkr.cpp
    projects/planeted/vetxmrkr.hpp
    projects/planeted/vpolymark.cpp
    projects/planeted/vpolymark.hpp
    projects/planeted/wdebug.cpp
    projects/planeted/wdebug.hpp
)

if(PLATFORM_WINDOWS)
  set(MACHINES_SOURCES
      ${MACHINES_SOURCES}
      # Exe icon and meta info
      ${PROJECT_SOURCE_DIR}/windows/resource.rc
  )
  set(PLANETED_SOURCES
      ${PLANETED_SOURCES}
      ${PROJECT_SOURCE_DIR}/windows/planeted.rc
  )
endif()

# Libraries
set(LIBS
    ${LIBS}
    #pthread
    SDL2::SDL2
    #GLEW
    #openal
    ${OPENAL_LIBRARY}
    #audio
    #dl
    #static-libstdc++
)

# Includes
set(LOCAL_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/libdev
    ${CMAKE_CURRENT_SOURCE_DIR}/projects
)

set(SYSTEM_INCLUDES
    ${LIBSNDFILE_INCLUDE_DIR}
    ${OPENAL_INCLUDE_DIR}
)

if(USE_ALURE)
    list(APPEND SYSTEM_INCLUDES
        ${ALURE_INCLUDE_DIRS}
    )
    list(APPEND LIBS
        #alure-static
        ${ALURE_LIBRARIES}
    )
endif()

if(TARGET ffmpeg::swscale)
    list(APPEND LIBS ffmpeg::swscale)
endif()
set(MACHINES_LIBS ${LIBS} PARENT_SCOPE)

include_directories(${LOCAL_INCLUDES})
include_directories(SYSTEM ${SYSTEM_INCLUDES})


# Link directories
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/.
)


# Targets

add_subdirectory(libdev/afx)
add_subdirectory(libdev/base)
add_subdirectory(libdev/ctl)
add_subdirectory(libdev/ftl)
add_subdirectory(libdev/mathex)
add_subdirectory(libdev/gxin)
add_subdirectory(libdev/xin)
add_subdirectory(libdev/phys)
add_subdirectory(libdev/stdlib)
add_subdirectory(libdev/profiler)
add_subdirectory(libdev/system)
add_subdirectory(libdev/device)
add_subdirectory(libdev/utility)
add_subdirectory(libdev/sound)
add_subdirectory(libdev/recorder)
add_subdirectory(libdev/world4d)
add_subdirectory(libdev/network)
add_subdirectory(libdev/render)
add_subdirectory(libdev/gui)
add_subdirectory(libdev/sim)
add_subdirectory(libdev/machphys)
add_subdirectory(libdev/envirnmt)
add_subdirectory(libdev/ani)
add_subdirectory(libdev/machlog)
add_subdirectory(libdev/machgui)

add_library(libs INTERFACE)
target_sources(libs

    INTERFACE 
    $<TARGET_OBJECTS:base>
    $<TARGET_OBJECTS:mathex>
    $<TARGET_OBJECTS:afx>
    $<TARGET_OBJECTS:utility>
    $<TARGET_OBJECTS:profiler>
    $<TARGET_OBJECTS:system>
    $<TARGET_OBJECTS:device>
    $<TARGET_OBJECTS:world4d>
    $<TARGET_OBJECTS:gxin>
    $<TARGET_OBJECTS:xin>
    $<TARGET_OBJECTS:stdlib>
    $<TARGET_OBJECTS:sound>
    $<TARGET_OBJECTS:ctl>
    $<TARGET_OBJECTS:phys>
    $<TARGET_OBJECTS:sim>
    $<TARGET_OBJECTS:gui>
    $<TARGET_OBJECTS:recorder>
    $<TARGET_OBJECTS:envirnmt>
    $<TARGET_OBJECTS:machphys>
    $<TARGET_OBJECTS:machlog>

)

target_link_libraries(libs INTERFACE
    LibDev::network
    LibDev::render
    LibDev::machgui
)

add_executable(machines ${MACHINES_SOURCES})
add_executable(planeted ${PLANETED_SOURCES})
set_target_properties(machines planeted PROPERTIES LINK_FLAGS_RELEASE -s)
set_target_properties(machines planeted PROPERTIES 
    COMPILE_FLAGS "${MACHINES_COMPILER_BIT_MODE}" LINK_FLAGS "${MACHINES_COMPILER_BIT_MODE}"
)

target_link_libraries(machines libs libaudio ${LIBS} )
target_link_libraries(planeted libs libaudio ${LIBS} )

if(TOOLCHAIN_EXTRA_FLAGS)
    target_link_libraries (machines ${TOOLCHAIN_EXTRA_FLAGS})
    target_link_libraries (planeted ${TOOLCHAIN_EXTRA_FLAGS})
endif()
