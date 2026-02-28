
set(AGT_LOADER_SOURCES
    AgtMeshLoader.cpp
    AgtMeshLoader.hpp
)

add_library(agt_loader ${GAME_COMPONENT_LIB_TYPE}
    ${AGT_LOADER_SOURCES}
    $<TARGET_OBJECTS:gxin>
)
add_library(FormatSupport::agt ALIAS agt_loader)

target_include_directories(agt_loader PUBLIC
    ${CMAKE_SOURCE_DIR}/src/libdev
)

target_link_libraries(agt_loader PUBLIC
    format_support_iface
)

target_link_libraries(agt_loader PRIVATE
    LibDev::mathex
    LibDev::system
)

target_compile_definitions(agt_loader INTERFACE
    HAS_AGT_LOADER
)
