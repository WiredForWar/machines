
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Compiler detection
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
        message(FATAL_ERROR "${PROJECT_NAME} requires GCC 4.7 or greater.")
    endif()

    message(STATUS "Detected GCC version ${CMAKE_CXX_COMPILER_VERSION}")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
        message(FATAL_ERROR "${PROJECT_NAME} requires Clang 3.1 or greater.")
    endif()

    message(STATUS "Detected ${CMAKE_CXX_COMPILER_ID} version ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Detected ${CMAKE_CXX_COMPILER_ID} version ${CMAKE_CXX_COMPILER_VERSION}")
else()
    message(FATAL_ERROR "Your C++ compiler doesn't seem to be supported.")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")
elseif(MSVC)
  # similar to -Werror=return-type
  # https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4715?view=msvc-170
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4715")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=mismatched-tags")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=unused-value")
endif()

# Clang exposes either the GCC-compatible driver (clang++) or the
# MSVC-compatible one (clang-cl); the option syntax differs between them.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(MACHINES_CLANG_CL TRUE)
else()
    set(MACHINES_CLANG_CL FALSE)
endif()

# Emit unwind tables in every configuration. The optimiser is free to drop the
# frame pointer, and where it does, the tables are the only thing a crash
# handler can walk the stack from. They cost image size and no run time.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MACHINES_CLANG_CL))
    check_cxx_compiler_flag("-funwind-tables" CXX_COMPILER_SUPPORTS_UNWIND_TABLES)
    if(CXX_COMPILER_SUPPORTS_UNWIND_TABLES)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funwind-tables")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funwind-tables")
    endif()

    # The C sources -- the audio and video decoders under external/ -- are built
    # with CMake's own release flags rather than the ones set below, so this is
    # an append and not a set: replacing them would quietly drop -DNDEBUG and
    # turn every assertion in them back on.
    string(APPEND CMAKE_C_FLAGS_RELEASE " -g")
endif()

# -m32/-m64 are understood by the GCC-compatible drivers only
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MACHINES_CLANG_CL))
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
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(NORMAL_CXX_FLAGS "${MACHINES_COMPILER_BIT_MODE} -Wall -Wmissing-declarations")
    set(NORMAL_C_FLAGS " ${MACHINES_COMPILER_BIT_MODE} -Wall")
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-suggest-override")
    endif()

    # These replace CMake's own release flags, so -g has to be asked for here or
    # the build has no debug information at all -- and then splitting it out
    # produces a .debug file with nothing in it. A stack from such a build
    # resolves to a plausible-looking file name and no function name, because
    # all that is left to read is the object files' names in the symbol table.
    # It costs no run time and stays out of the shipped binary, which is
    # stripped after the split.
    set(RELEASE_CXX_FLAGS "-O2 -g")
    set(DEBUG_CXX_FLAGS "-g -O0")
    set(TEST_CXX_FLAGS "-pthread")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(MACHINES_CLANG_CL)
        # clang-cl targets the MSVC ABI and takes MSVC-style options, so it
        # needs the same set as the MSVC branch below. GCC-style -W flags are
        # still accepted and are used for the warning selection.
        set(NORMAL_CXX_FLAGS "/EHsc")
        # Including the optimiser, for the reason the MSVC branch gives below.
        set(RELEASE_CXX_FLAGS "/MD /O2 /Ob2")
        set(DEBUG_CXX_FLAGS "/MDd /Zi")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
        set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG")
    else()
        #set(NORMAL_CXX_FLAGS "-Wall -Werror -Wold-style-cast -pedantic-errors -Wmissing-prototypes")
        set(NORMAL_CXX_FLAGS "${MACHINES_COMPILER_BIT_MODE} -Wall")
        # -g for the reason the GCC branch above gives.
        set(RELEASE_CXX_FLAGS "-O2 -g")
        set(DEBUG_CXX_FLAGS "-g -O0")
    endif()

    # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations")

    # The code base predates `override` and declares overriding members with
    # plain `virtual`. Clang reports every one of them (~11k warnings), which
    # buries everything else, so silence that single diagnostic.
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-inconsistent-missing-override")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(NORMAL_CXX_FLAGS "/wd4244 /wd4309 /wd4800 /wd4996 /wd4351 /EHsc") # disable some useless warnings
    # These replace CMake's own release flags rather than adding to them, so the
    # optimiser has to be asked for explicitly. Without it this build came out
    # unoptimised, which cost it about a third of its frame rate next to the
    # MinGW one.
    # /Zi puts the debug information in a .pdb beside the binary rather than in
    # it, which is the MSVC equivalent of the split this build does with objcopy
    # elsewhere. Without it a release build has no symbols anywhere, and a crash
    # report from it can never be resolved.
    set(RELEASE_CXX_FLAGS "/MD /O2 /Ob2 /Zi")
    set(DEBUG_CXX_FLAGS "/MDd /ZI")

    if (CMAKE_CXX_STANDARD GREATER_EQUAL 11)
        # Ensure __cplusplus is correct, otherwise it defaults to 199711L which isn't true
        # https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-160
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
    endif()

    # Needed for Debug information (it's set to "No" by default for some reason)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG")

    # /DEBUG is what actually emits the .pdb, and on its own it also turns off
    # /OPT:REF and /OPT:ICF -- so asking for symbols would quietly hand back a
    # bigger and slower binary than the one measured. Asking for both again
    # restores exactly the release the linker would otherwise have produced.
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG /OPT:REF /OPT:ICF")
endif()

# Compiler flags
set(MACHINES_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NORMAL_CXX_FLAGS}")
set(MACHINES_C_FLAGS "${CMAKE_C_FLAGS} ${NORMAL_C_FLAGS}")
set(MACHINES_CXX_FLAGS_RELEASE "${RELEASE_CXX_FLAGS}")
set(MACHINES_CXX_FLAGS_DEBUG "${DEBUG_CXX_FLAGS}")
