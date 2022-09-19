
# Usage:
# option_with_list(BUILD_WITH_SWSCALE "Build with SWScale support" "Auto"
#     STRINGS
#         "ON" "OFF" "Auto"
# )
function(option_with_list OPTION_NAME OPTION_TEXT OPTION_VALUE OPTION_LIST_PLACEHOLDER)
    set(${OPTION_NAME} "${OPTION_VALUE}" CACHE STRING "${OPTION_TEXT}")
    set_property(CACHE "${OPTION_NAME}" PROPERTY STRINGS ${ARGN})

    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        set(${OPTION_PREFIX}_${OPTION_NAME} ${${OPTION_NAME}} PARENT_SCOPE)
    endif()
endfunction()

# Usage:
# option_on_off_auto(BUILD_WITH_ALURE "Build with Alure support" "Auto")
function(option_on_off_auto OPTION_NAME OPTION_TEXT OPTION_VALUE)
    option_with_list(${OPTION_NAME} "${OPTION_TEXT}" "${OPTION_VALUE}"
        STRINGS
            "ON" "OFF" "Auto"
    )
endfunction()

# Usage:
#   find_optional_dependency(<PACKAGE_NAME> [extra find_package arguments ...])
# Input variables:
#   - BUILD_WITH_<PACKAGE_NAME_CAPITALIZED>=<"Auto" or any boolean value>
# Output variables:
#   - USE_<PACKAGE_NAME_CAPITALIZED>
# E.g.:
#   find_optional_dependency(Alure)
#   find_optional_dependency(Alure 1.2)
#
# Note: this has to be a macro for compatibility with CMake < 3.24 (function requires find_package(.. GLOBAL))
macro(find_optional_dependency)
    if(DEFINED PACKAGE_NAME OR DEFINED OPTION_NAME OR DEFINED OUTPUT_VAR)
        message(AUTHOR_WARNING "Some variables conflicts with macro find_optional_dependency()")
    endif()
    set(PACKAGE_NAME ${ARGV0})
    string(TOUPPER "${PACKAGE_NAME}" NAME_CAPITALIZED)
    set(OPTION_NAME "BUILD_WITH_${NAME_CAPITALIZED}")
    set(OUTPUT_VAR "USE_${NAME_CAPITALIZED}")

    string(TOUPPER "${${OPTION_NAME}}" OPTION_VALUE_CAPITALIZED)

    if("${OPTION_VALUE_CAPITALIZED}" STREQUAL "AUTO" OR "${OPTION_VALUE_CAPITALIZED}")
        if(NOT "${OPTION_VALUE_CAPITALIZED}" STREQUAL "AUTO")
            set(Package_REQUIRED REQUIRED)
        endif()

        set(FIND_PACKAGE_EXTRA_ARGS "${ARGN}")
        list(REMOVE_AT FIND_PACKAGE_EXTRA_ARGS 0)

        find_package(${PACKAGE_NAME} ${Package_REQUIRED} ${FIND_PACKAGE_EXTRA_ARGS})

        unset(FIND_PACKAGE_EXTRA_ARGS)

        set(${OUTPUT_VAR} ${${PACKAGE_NAME}_FOUND})
    else()
        set(${OUTPUT_VAR} OFF)
    endif()

    unset(PACKAGE_NAME)
    unset(OPTION_NAME)
    unset(OUTPUT_VAR)
endmacro()
