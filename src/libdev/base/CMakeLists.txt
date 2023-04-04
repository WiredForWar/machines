
set(BASE_SOURCES
    IProgressReporter.hpp
    appbuff.cpp
    append.cpp
    base.cpp
    base.hpp
    cballoc.cpp
    cballoc.hpp
    compiler.hpp
    crashi.cpp
    cstrbuff.cpp
    diag.cpp
    diag.hpp
    diagint.cpp
    diagstr.cpp
    diagstrs.cpp
    DiagStreamType.hpp
    error.cpp
    error.hpp
    filtbuff.cpp
    filter.cpp
    indent.cpp
    infsbuff.cpp
    istr.cpp
    istr.hpp
    istrbuff.cpp
    istrimpl.cpp
    lessptr.hpp
    logbuff.cpp
    logbuff.hpp
    lstrbuff.cpp
    memcdata.cpp
    memcdata.hpp
    memchk.cpp
    memchk.hpp
    memwatch.cpp
    memwatch.hpp
    ostr.cpp
    ostr.hpp
    periread.cpp
    periwrit.cpp
    perolog.cpp
    persist.cpp
    persist.hpp
    poolallo.cpp
    poolallo.hpp
    prepbuff.cpp
    prepend.cpp
    prepost.cpp
    prepost.hpp
    trace.cpp
    tracker.cpp
    tracker.hpp
    trackeri.cpp
)

add_library(base ${GAME_COMPONENT_LIB_TYPE} ${BASE_SOURCES})
add_library(LibDev::base ALIAS base)

target_link_libraries(base PUBLIC
    LibDev::stdlib

    spdlog::spdlog
)
