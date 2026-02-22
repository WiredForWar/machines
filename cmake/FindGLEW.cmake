# CMake module to find GLEW
# Borrowed from http://code.google.com/p/nvidia-texture-tools/
# MIT license Copyright (c) 2007 NVIDIA Corporation

# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_PATH
# GLEW_LIBRARY
#

if(WIN32)
    find_path(GLEW_INCLUDE_PATH GL/glew.h
        $ENV{PROGRAMFILES}/GLEW/include
        ${PROJECT_SOURCE_DIR}/src/nvgl/glew/include
        DOC "The directory where GL/glew.h resides"
    )
    find_library(GLEW_LIBRARY
        NAMES glew GLEW glew32 glew32s
        PATHS
        /mingw/bin # for MinGW's MSYS
        /mingw/lib
        ${PROJECT_SOURCE_DIR}/glew/bin # or in local directory
        ${PROJECT_SOURCE_DIR}/glew/lib
        DOC "The GLEW library"
    )
else()
    find_path(GLEW_INCLUDE_PATH GL/glew.h
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
        DOC "The directory where GL/glew.h resides"
    )
    find_library(GLEW_LIBRARY
        NAMES GLEW glew
        PATHS
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        DOC "The GLEW library"
    )
endif()

mark_as_advanced(GLEW_INCLUDE_PATH)
mark_as_advanced(GLEW_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_LIBRARY GLEW_INCLUDE_PATH)

if(GLEW_FOUND)
    if(NOT TARGET GLEW::GLEW)
        add_library(GLEW::GLEW INTERFACE IMPORTED)
        set_target_properties(GLEW::GLEW PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES
                "${GLEW_INCLUDE_PATH}"
            INTERFACE_LINK_LIBRARIES
                "${GLEW_LIBRARY}"
        )
    endif()
endif()
