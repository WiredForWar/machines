
set(XIN_SOURCES
    XFileHelper.hpp
    XFileParser.cpp
    XFileParser.hpp
    XFileTypes.hpp
    fastatof.hpp
    formatter.hpp
    strutils.hpp
)

add_library(xin OBJECT ${XIN_SOURCES})
