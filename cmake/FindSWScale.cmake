# Filename: FindSWScale.cmake
# Author: CFSworks (10 Apr, 2014)
#
# Usage:
#   find_package(SWScale [REQUIRED] [QUIET])
#
# Once done this will define:
#   SWSCALE_FOUND       - system has ffmpeg's libswscale
#   SWSCALE_INCLUDE_DIRS - the libswscale include directory
#   SWSCALE_LIBRARIES     - the path to the library binary
#

# Find the libswscale include files
find_path(SWSCALE_INCLUDE_DIRS
  NAMES "libswscale/swscale.h"
  PATHS "/usr/include"
        "/usr/local/include"
        "/sw/include"
        "/opt/include"
        "/opt/local/include"
        "/opt/csw/include"
  PATH_SUFFIXES "libav" "ffmpeg"
)

# Find the libswscale library
find_library(SWSCALE_LIBRARIES
  NAMES "swscale"
  PATHS "/usr"
        "/usr/local"
        "/usr/freeware"
        "/sw"
        "/opt"
        "/opt/csw"
  PATH_SUFFIXES "lib" "lib32" "lib64"
)

mark_as_advanced(SWSCALE_INCLUDE_DIRS)
mark_as_advanced(SWSCALE_LIBRARIES)

# Translate library into library directory
if(SWSCALE_LIBRARIES)
  unset(SWSCALE_LIBRARY_DIRS CACHE)
  get_filename_component(SWSCALE_LIBRARY_DIRS "${SWSCALE_LIBRARIES}" PATH)
  set(SWSCALE_LIBRARY_DIRS "${SWSCALE_LIBRARY_DIRS}" CACHE PATH "The path to libffmpeg's library directory.") # Library path
endif()

mark_as_advanced(SWSCALE_LIBRARY_DIRS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SWScale DEFAULT_MSG SWSCALE_LIBRARIES SWSCALE_INCLUDE_DIRS SWSCALE_LIBRARY_DIRS)

if(SWScale_FOUND)
    if(NOT TARGET ffmpeg::swscale)
        add_library(ffmpeg::swscale INTERFACE IMPORTED)
        set_target_properties(ffmpeg::swscale PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                              "${SWSCALE_INCLUDE_DIRS}")
        set_target_properties(ffmpeg::swscale PROPERTIES INTERFACE_LINK_DIRECTORIES
                              "${SWSCALE_LIBRARY_DIRS}")
        set_target_properties(ffmpeg::swscale PROPERTIES INTERFACE_LINK_LIBRARIES
                              "${SWSCALE_LIBRARIES}")
    endif()
endif()
