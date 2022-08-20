# Sample toolchain file for building for Windows from an Ubuntu Linux system.
#
# Typical usage:
#    *) install cross compiler: `sudo apt-get install mingw-w64`
#    *) mkdir buildMingw32 && cd buildMingw32
#    *) cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake ..
#

set(TOOLCHAIN_PREFIX i686-w64-mingw32)
include(${CMAKE_CURRENT_LIST_DIR}/Toolchain-mingw.cmake)
