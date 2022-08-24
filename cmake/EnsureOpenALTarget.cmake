
if(OpenAL_FOUND AND NOT TARGET OpenAL::OpenAL)
    add_library(OpenAL::OpenAL INTERFACE IMPORTED)
    if(OPENAL_INCLUDE_DIR)
        set_target_properties(OpenAL::OpenAL PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                              "${OPENAL_INCLUDE_DIR}")
    endif()
    set_property(TARGET OpenAL::OpenAL PROPERTY INTERFACE_LINK_LIBRARIES
                 "${OPENAL_LIBRARY}")
endif()
