# - Try to find enet
# Once done this will define
#
#  ENET_FOUND - system has enet
#  ENET_INCLUDE_DIRS - the enet include directory
#  ENET_LIBRARIES - the libraries needed to use enet
#
# $ENETDIR is an environment variable used for finding enet.
#
#  Borrowed from The Mana World
#  http://themanaworld.org/
#
# Several changes and additions by Fabian 'x3n' Landau
# Lots of simplifications by Adrian Friedli
#                 > www.orxonox.net <

find_path(ENET_INCLUDE_DIRS enet/enet.h
    PATHS
    $ENV{ENETDIR}
    /usr/local
    /usr
    PATH_SUFFIXES include
)

find_library(ENET_LIBRARY
    NAMES
        enet
    PATHS
        $ENV{ENETDIR}
        /usr/local
        /usr
    PATH_SUFFIXES
        lib
        lib/static
)

# handle the QUIETLY and REQUIRED arguments and set ENET_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(enet DEFAULT_MSG ENET_LIBRARY ENET_INCLUDE_DIRS)

if(ENET_FOUND)
    if(WIN32)
        set(WINDOWS_ENET_DEPENDENCIES "ws2_32;winmm")
        set(ENET_LIBRARIES ${ENET_LIBRARY} ${WINDOWS_ENET_DEPENDENCIES})
    else()
        set(ENET_LIBRARIES ${ENET_LIBRARY})
    endif()

    if(NOT TARGET enet::enet)
        add_library(enet::enet INTERFACE IMPORTED)
        set_target_properties(enet::enet PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
            "${ENET_INCLUDE_DIRS}")
        set_property(TARGET enet::enet PROPERTY INTERFACE_LINK_LIBRARIES
            "${ENET_LIBRARIES}")
    endif()
endif()

mark_as_advanced(ENET_LIBRARY ENET_LIBRARIES ENET_INCLUDE_DIRS)
