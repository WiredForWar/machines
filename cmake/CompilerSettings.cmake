
# Compiler detection
#set(CMAKE_CXX_COMPILER "${OMPP_CXX}" CACHE PATH "Set compiler used")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
        message(FATAL_ERROR "${PROJECT_NAME} requires GCC 4.7 or greater.")
    endif()

    message(STATUS "Detected GCC version ${CMAKE_CXX_COMPILER_VERSION}")

    #set(NORMAL_CXX_FLAGS "-Wall -Werror -Wold-style-cast -pedantic-errors -Wmissing-declarations")
    if(BUILD_32)
        set(MACHINES_COMPILER_BIT_MODE -m32)
    else()
        set(MACHINES_COMPILER_BIT_MODE -m64)
    endif()
    check_cxx_compiler_flag("${MACHINES_COMPILER_BIT_MODE}" CXX_COMPILER_SUPPORTS_BIT_MODE)
    check_c_compiler_flag("${MACHINES_COMPILER_BIT_MODE}" C_COMPILER_SUPPORTS_BIT_MODE)
    if(NOT C_COMPILER_SUPPORTS_BIT_MODE OR NOT CXX_COMPILER_SUPPORTS_BIT_MODE)
        set(MACHINES_COMPILER_BIT_MODE "")
        message(WARNING "${PROJECT_NAME} will be built for compiler default target architecture.")
    endif()

    set(NORMAL_CXX_FLAGS "${MACHINES_COMPILER_BIT_MODE} -Wall -Wold-style-cast -Wmissing-declarations")
    set(NORMAL_C_FLAGS " ${MACHINES_COMPILER_BIT_MODE} -Wall")
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wsuggest-override")
    endif()

    set(RELEASE_CXX_FLAGS "-O2")
    set(DEBUG_CXX_FLAGS "-gstabs -O0")
    set(TEST_CXX_FLAGS "-pthread")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
        message(FATAL_ERROR "${PROJECT_NAME} requires Clang 3.1 or greater.")
    endif()

    message(STATUS "Detected Clang version 3.1+")

    set(NORMAL_CXX_FLAGS "${MACHINES_COMPILER_BIT_MODE} -Wall -Werror -Wold-style-cast -pedantic-errors -Wmissing-prototypes")
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958
    set(RELEASE_CXX_FLAGS "-O2")
    set(DEBUG_CXX_FLAGS "-g -O0")
    set(TEST_CXX_FLAGS "-pthread")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Detected MSVC compiler")

    set(NORMAL_CXX_FLAGS "/wd\"4244\" /wd\"4309\" /wd\"4800\" /wd\"4996\" /wd\"4351\" /EHsc") # disable some useless warnings
    set(RELEASE_CXX_FLAGS "/MD")
    set(DEBUG_CXX_FLAGS "/MDd /ZI")
    set(TEST_CXX_FLAGS "")

    # Needed for Debug information (it's set to "No" by default for some reason)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG")
else()
    message(FATAL_ERROR "Your C++ compiler doesn't seem to be supported.")
endif()

# Compiler flags
set(MACHINES_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NORMAL_CXX_FLAGS}")
set(MACHINES_C_FLAGS "${CMAKE_C_FLAGS} ${NORMAL_C_FLAGS}")
set(MACHINES_CXX_FLAGS_RELEASE "${RELEASE_CXX_FLAGS}")
set(MACHINES_CXX_FLAGS_DEBUG "${DEBUG_CXX_FLAGS}")
