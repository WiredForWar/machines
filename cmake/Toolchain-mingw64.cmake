# Sample toolchain file for building for Windows from an Ubuntu Linux system.
#
# Typical usage:
#    *) install cross compiler: `sudo apt-get install mingw-w64`
#    *) mkdir buildMingw64 && cd buildMingw64
#    *) cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw64.cmake ..
#

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
include(${CMAKE_CURRENT_LIST_DIR}/Toolchain-mingw.cmake)
