
set(NETWORK_SOURCES
    AddressUtils.cpp
    AddressUtils.hpp
    NetSessionInfo.hpp
    buffer.cpp
    buffer.hpp
    buffer.ipp
    comport.cpp
    comport.hpp
    compound.cpp
    internal/compound.hpp
    internal/netinet.hpp
    internal/nodei.hpp
    internal/recorder.hpp
    internal/sessioni.hpp
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
    # node.cpp # unused
    # node.hpp # unused
    # nodei.cpp # unused
    nodeuid.cpp
    nodeuid.hpp
    pinghelp.cpp
    pinghelp.hpp
    priority.cpp
    priority.hpp
    recorder.cpp
    # session.cpp # unused
    session.hpp
    # sessioni.cpp # unused
    sessuid.cpp
    sessuid.hpp
    sysmess.cpp
    sysmess.hpp
)

add_library(network STATIC ${NETWORK_SOURCES})
add_library(LibDev::network ALIAS network)

target_link_libraries(network PRIVATE
    LibDev::base
    LibDev::system

    enet::enet
)
