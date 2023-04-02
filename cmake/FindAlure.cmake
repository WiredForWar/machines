# - Try to find Alure
# Once done this will define
#
#  ALURE_FOUND - system has Alure
#  ALURE_INCLUDE_DIRS - the Alure include directory
#  ALURE_LIBRARIES - Link these to use Alure
#
#  Copyright © 2011 Laszlo Papp <djszapi@archlinux.us>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if(NOT TARGET OpenAL::OpenAL)
    message(FATAL_ERROR "Alure: OpenAL is missing")
    set(Alure_FOUND FALSE)
    return()
endif()

find_path(ALURE_INCLUDE_DIR
    NAMES
      alure.h
      PATH_SUFFIXES AL OpenAL
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

IF (NOT ALURE_MIN_VERSION)
  SET(ALURE_MIN_VERSION "1.1")
ENDIF()

if(ALURE_INCLUDE_DIR)
    file(STRINGS ${ALURE_INCLUDE_DIR}/alure.h VERSION_LINE
        LIMIT_COUNT 1
        REGEX "^#define ALURE_VERSION_STRING "
    )
    string(REGEX MATCH "([0-9]+)(\\.([0-9]+))+" ALURE_VERSION "${VERSION_LINE}")
endif()

  find_library(ALURE_LIBRARY
    NAMES alure alure-static ALURE32-static
    PATHS
      /usr/lib/i386-linux-gnu
      /usr
      /usr/local
      /opt/local
      /sw
    PATH_SUFFIXES lib32 lib
  )

  set(ALURE_INCLUDE_DIRS
      ${ALURE_INCLUDE_DIR}
  )
  set(ALURE_LIBRARIES
      ${ALURE_LIBRARY}
  )

  INCLUDE(FindPackageHandleStandardArgs)
  # handle the QUIETLY and REQUIRED arguments and set ALURE_FOUND to TRUE if
  # all listed variables are TRUE
  find_package_handle_standard_args(Alure
    REQUIRED_VARS
      ALURE_LIBRARY ALURE_INCLUDE_DIR
    VERSION_VAR
      ALURE_VERSION
  )

  # show the ALURE_INCLUDE_DIRS and ALURE_LIBRARIES variables only in the advanced view
  mark_as_advanced(ALURE_INCLUDE_DIRS ALURE_LIBRARIES)

if(Alure_FOUND)
    set(ALURE_LIBRARIES ${ALURE_LIBRARIES} OpenAL::OpenAL)
    if(NOT TARGET Alure::Alure)
        add_library(Alure::Alure INTERFACE IMPORTED)
        set_target_properties(Alure::Alure PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES
                "${ALURE_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES
                "${ALURE_LIBRARIES}"
        )
    endif()
endif()
