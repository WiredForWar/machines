
set(UTILITY_SOURCES
    Array.hpp
    CallbackHandle.hpp
    DependencyProvider.hpp
    AsciiPicture.cpp
    AsciiPicture.hpp
    AsciiPictureImpl.cpp
    BoundedIdPMap.hpp
    BoundedIdPMap.itp
    BoundedIdPMapVoid.cpp
    BoundedIdGenerator.cpp
    BoundedIdGenerator.hpp
    Constraint.hpp
    Factory.hpp
    Id.hpp
    Indent.hpp
    internal/AsciiPictureImpl.hpp
    Interval.hpp
    LineTokeniser.cpp
    LineTokeniser.hpp
    Percentage.hpp
    private/BoundedIdPMapVoid.hpp
    private/UInt64.hpp
    Property.hpp
    rapidxml.hpp
    rapidxml_ext.hpp
    rapidxml_print.hpp
    rapidxml_utils.hpp
    String.cpp
    String.hpp
    Subrange.hpp
    DebugTimer.hpp
    ToString.cpp
    ToString.hpp
)

add_library(utility OBJECT ${UTILITY_SOURCES})
add_library(LibDev::utility ALIAS utility)

target_link_libraries(utility PUBLIC
    LibDev::base
)
