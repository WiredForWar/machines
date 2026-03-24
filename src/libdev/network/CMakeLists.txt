
set(NETWORK_SOURCES
    AddressUtils.cpp
    AddressUtils.hpp
    Buffer.cpp
    Buffer.hpp
    Buffer.ipp
    ComPortAddress.cpp
    ComPortAddress.hpp
    CompoundMessage.cpp
    internal/CompoundMessage.hpp
    internal/NetInet.hpp
    internal/NodeImpl.hpp
    internal/Recorder.hpp
    internal/SessionImpl.hpp
    Message.cpp
    Message.hpp
    MessageBody.cpp
    MessageBody.hpp
    MessageHeader.cpp
    MessageHeader.hpp
    NetDefs.hpp
    NetInet.cpp
    Network.cpp
    Network.hpp
    NetSessionInfo.hpp
    # node.cpp # unused
    # node.hpp # unused
    # nodei.cpp # unused
    NodeUid.cpp
    NodeUid.hpp
    PingHelper.cpp
    PingHelper.hpp
    Priority.cpp
    Priority.hpp
    Recorder.cpp
    RendezvousWorker.cpp
    RendezvousWorker.hpp
    # session.cpp # unused
    Session.hpp
    # sessioni.cpp # unused
    SessionUid.cpp
    SessionUid.hpp
    StunClient.cpp
    StunClient.hpp
    SystemMessageHandler.cpp
    SystemMessageHandler.hpp
)

add_library(network STATIC ${NETWORK_SOURCES})
add_library(LibDev::network ALIAS network)

target_link_libraries(network PRIVATE
    LibDev::base
    LibDev::system
    LibDev::rendezvous
    LibDev::version

    enet::enet
)
