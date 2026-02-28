
set(XFILE_LOADER_SOURCES
    XFileMeshLoader.cpp
    XFileMeshLoader.hpp
)

add_library(xfile_loader ${GAME_COMPONENT_LIB_TYPE}
    ${XFILE_LOADER_SOURCES}
    $<TARGET_OBJECTS:xin>
)
add_library(FormatSupport::xfile ALIAS xfile_loader)

target_include_directories(xfile_loader PUBLIC
    ${CMAKE_SOURCE_DIR}/src/libdev
)

target_link_libraries(xfile_loader PUBLIC
    format_support_iface
)

target_link_libraries(xfile_loader PRIVATE
    LibDev::mathex
    LibDev::system
    LibDev::utility
)

target_compile_definitions(xfile_loader INTERFACE
    HAS_XFILE_LOADER
)
