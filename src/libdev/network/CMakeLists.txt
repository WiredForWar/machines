
set(NETWORK_SOURCES
    buffer.cpp
    buffer.hpp
    buffer.ipp
    comport.cpp
    comport.hpp
    compound.cpp
    internal/compound.hpp
    internal/mapping.hpp
    internal/netinet.hpp
    internal/nodei.hpp
    internal/recorder.hpp
    internal/sessioni.hpp
    mapping.cpp
    message.cpp
    message.hpp
    messbody.cpp
    messbody.hpp
    messhead.cpp
    messhead.hpp
    netdefs.hpp
    netinet.cpp
    netnet.cpp
    netnet.hpp
    # node.cpp // somewhy unused
    node.hpp
    nodei.cpp
    nodeuid.cpp
    nodeuid.hpp
    pinghelp.cpp
    pinghelp.hpp
    priority.cpp
    priority.hpp
    recorder.cpp
    # session.cpp // somewhy unused
    session.hpp
    # sessioni.cpp // somewhy unused
    sessuid.cpp
    sessuid.hpp
    sysmess.cpp
    sysmess.hpp
)

add_library(network STATIC ${NETWORK_SOURCES})
add_library(LibDev::network ALIAS network)

target_link_libraries(network PRIVATE
    LibDev::base

    enet::enet
)
