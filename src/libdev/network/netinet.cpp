
#include "network/internal/netinet.hpp"

#include "base/diag.hpp"
#include "device/time.hpp"
#include "stdlib/string.hpp"
#include "profiler/profiler.hpp"
#include "ctl/algorith.hpp"
#include "utility/linetok.hpp"

#include "network/AddressUtils.hpp"
#include "network/nodeuid.hpp"
#include "network/sessuid.hpp"
#include "network/session.hpp"
#include "network/messbody.hpp"
#include "network/message.hpp"
#include "network/sysmess.hpp"

#include "network/internal/sessioni.hpp"
#include "network/internal/recorder.hpp"
#include "network/internal/compound.hpp"

#include "recorder/recorder.hpp"

#include "system/Endian.hpp"
#include "system/metafile.hpp"
#include "system/metaistr.hpp"
#include "system/registry.hpp"

#include "spdlog/spdlog.h"

#include <memory>

#define MAXNAMELEN 200

#define INITGUID

static constexpr char DiscoveryMagic[4] = { 'W', 'F', 'W', '0' };
static constexpr uint32_t DiscoveryVersion = 1;
static constexpr uint16_t GamePort = 1234;
// Port 30583 means Wired for War
static constexpr uint16_t LANServerDiscoveryPort = 'w' << 8 | 'w';

struct ServerReply
{
    char magic[sizeof(DiscoveryMagic)]{};
    uint32_t version{};
    char serverName[64]{};
    uint16_t port{};
};

struct ClientRequest
{
    char magic[sizeof(DiscoveryMagic)]{};
    uint32_t version{};
    char message[16]{};
};

NetINetwork::~NetINetwork()
{
    NETWORK_STREAM("NetINetwork::~NetINetwork " << static_cast<const void*>(this) << std::endl);
    NETWORK_INDENT(2);
    NETWORK_STREAM("ask record what state it is in...\n");
    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        RecRecorder::instance().recordingAllowed(false);

        NETWORK_STREAM("call to NetINetwork::clearProtocols" << std::endl);
        NetINetwork::clearProtocols();

        NETWORK_STREAM("delete sessions" << std::endl);
        sessions_.clear();

        NETWORK_STREAM(" delete message handler" << std::endl);
        if (pSystemMessageHandler_)
            delete pSystemMessageHandler_;

        enet_deinitialize();

        RecRecorder::instance().recordingAllowed(true);
    }
    NETWORK_INDENT(-2);
    NETWORK_STREAM("NetINetwork::~NetINetwork DONE " << static_cast<const void*>(this) << std::endl);
}

NetProcessUid& NetINetwork::processUidMaster() const
{
    PRE(isValidNoRecord());
    static bool initialised = false;

    static NetProcessUid thisProcessUid("temp", 0);

    return thisProcessUid;
}

bool NetINetwork::hasAppSession(const NetAppSessionUid& aUid) const
{
    PRE(isValidNoRecord());

    bool found = false;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        found = NetIRecorder::instance().playbackHasAppSession();
    }
    else
    {
        found = hasAppSessionNoRecord(aUid);

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordHasAppSession(found);
        }
    }

    POST(isValidNoRecord());

    return found;
}

///////////////////////////////

bool NetINetwork::hasAppSessionNoRecord(const NetAppSessionUid& aUid) const
{
    PRE(isValidNoRecord());

    // const auto result
    //     = std::find_if(sessions_.begin(), sessions_.end(), [&aUid](const std::unique_ptr<NetAppSessionUid>& session) {
    //     return session && *session == aUid;
    // });
    bool found = false; // result != sessions_.end();

    POST(isValidNoRecord());

    return found;
}

///////////////////////////////

// call periodically to ensure that network information remains current.
void NetINetwork::update()
{
    if (hasActiveSession())
    {
        if (isLogicalHost())
        {
            replyToServerDiscoveryRequests();
        }
    }
    else
    {
        static constexpr double UpdateInterval = 5.0;
        if (DevTime::instance().time() - lastSessionsUpdate_ > UpdateInterval)
        {
            updateSessions();
        }

        acceptLocalServersReplies();
    }
}

const NetNetwork::Sessions& NetINetwork::sessions() const
{
    PRE(isValidNoRecord());

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        NetINetwork* nonConstThis = _CONST_CAST(NetINetwork*, this);

        NetIRecorder::instance().playbackSessions(&nonConstThis->sessions_);
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordSessions(sessions_);
        }
    }

    return sessions_;
    // POST( for_each( result.begin(), result.end(), negate( mem_fun( NetAppSession::isEmpty ) ) ) );
    // i.e. every app session contains at least one node.
}

///////////////////////////////

///////////////////////////////
NetAppUid NetINetwork::appUid() const
{
    PRE(isValidNoRecord());

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        appUid_ = NetIRecorder::instance().playbackAppUid();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordAppUid(appUid_);
        }
    }

    return appUid_;
}

NetAppUid NetINetwork::appUidNoRecord() const
{
    PRE(isValidNoRecord());

    return appUid_;
}

NetINetwork::NetINetwork()
    : descFileName_("")
    , descCommandLine_("")
    , descPath_("")
    , descCurrentDirectory_("")
    , descDescription_("")
    , localPlayerName_("")
    , maxBytesPerSecond_(9000)
    , pingAllAllowed_(true)
    , maxSentMessagesPerSecond_(40)
    , originalMaxSentMessagesPerSecond_(40)
{
    PRE(isValidNoRecord());

    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        RecRecorder::instance().recordingAllowed(false);

        RecRecorder::instance().recordingAllowed(true);
    }

    POST(isValidNoRecord());
}

void NetINetwork::initHost(bool asServer)
{
    resetHost();

    isLogicalHost_ = asServer;
    ENetAddress address;
    if (asServer)
    {
        spdlog::info("NetINetwork: Initializing a server");
        // b. Create a host using enet_host_create
        address.host = ENET_HOST_ANY;
        address.port = GamePort;

        pHost_ = enet_host_create(
            &address /* the address to bind the server host to */,
            32 /* allow up to 32 clients and/or outgoing connections */,
            2 /* allow up to 2 channels to be used, 0 and 1 */,
            0 /* assume any amount of incoming bandwidth */,
            0 /* assume any amount of outgoing bandwidth */);

        if (pHost_ == nullptr)
        {
            spdlog::error("NetINetwork: Unable to create ENet server");
            // exit(EXIT_FAILURE);
            currentStatus(NetNetwork::NETNET_NODEERROR);
            return;
        }

        spdlog::info("NetINetwork: Created ENet LAN server on port {}", pHost_->address.port);
    }
    else
    {
        spdlog::info("NetINetwork: Initializing a client");
        // b. Create a host using enet_host_create
        pHost_ = enet_host_create(nullptr, 16, 2, /*57600/8*/ 0, /*14400/8*/ 0);

        if (pHost_ == nullptr)
        {
            spdlog::error("NetINetwork: Unable to create ENet client");
            // exit(EXIT_FAILURE);
            currentStatus(NetNetwork::NETNET_NODEERROR);
            return;
        }
    }
}

void NetINetwork::resetHost()
{
    if (pHost_ == nullptr)
        return;

    for (ENetPeer* peer : peers_)
    {
        enet_peer_disconnect(peer, 0);
    }
    peers_.clear();
    enet_host_destroy(pHost_);
    pHost_ = nullptr;

    isLogicalHost_ = false;
}

bool NetINetwork::hasActiveSession() const
{
    return pHost_ != nullptr;
}

void NetINetwork::pollMessages()
{
    //  This line must be outside the if for the recorder to work properly

    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        RecRecorder::instance().recordingAllowed(false);

        NETWORK_STREAM("NetINetwork::pollMessages\n");
        imStuffed_ = false;

        // The poll message call will also ask the node how much data is insiode the node outgoing buffer.
        // certain values will then set the "I'm stuffed flag" so that external clients will stop trying to stuff more
        // in.

        ENetEvent event;
        while (enet_host_service(pHost_, &event, 0) > 0)
        {
            // eventStatus = enet_host_service(client, &event, 50000);
            //  If we had some event that interested us
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    spdlog::debug(
                        "NetINetwork: Got a new connection from {} {}",
                        event.peer->address.host,
                        event.peer->connectID);
                    peers_.push_back(event.peer);
                    if (isLogicalHost_)
                    {
                        sendInitPacket(event.peer);
                    }
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    NETWORK_STREAM("Got message from : " << event.peer->address.host << std::endl);
                    // First packet is an peer name
                    if (!event.peer->data)
                    {
                        char* data = _NEW_ARRAY(char, strlen((char*)event.packet->data) + 1);
                        strcpy(data, (char*)event.packet->data);
                        event.peer->data = data;
                        NETWORK_STREAM("This is intruduction from: " << data << std::endl);
                    }
                    else
                    {
                        // Lets broadcast this message to all except sender
                        Peers::const_iterator i = peers_.begin();
                        Peers::const_iterator j = peers_.end();
                        while (i != j)
                        {
                            if ((*i) != event.peer)
                            {
                                ENetPacket* packet
                                    = enet_packet_create(event.packet->data, event.packet->dataLength, 0);
                                enet_peer_send(*i, 0, packet);
                            }
                            ++i;
                        }

                        NetMessageBody messageBody((uint8*)event.packet->data, event.packet->dataLength);
                        NetPriority priority(1);
                        NetMessageHeader thisMessageHeader(event.peer, priority);
                        NetMessage* pThisMessage = new NetMessage(thisMessageHeader, messageBody);
                        messageBuffer_.push_back(pThisMessage);
                    }
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    spdlog::debug("NetINetwork: Peer disconnected: {}", event.peer->address.host);

                    // Reset client's information
                    _DELETE_ARRAY((char*)event.peer->data);
                    event.peer->data = nullptr;
                    peers_.erase(std::remove(peers_.begin(), peers_.end(), event.peer), peers_.end());
                    if (peers_.empty())
                    {
                        if (pSystemMessageHandler_)
                            bool shouldAbort = ! pSystemMessageHandler_->handleSessionLostMessage();
                    }
                    break;
                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
        NETWORK_STREAM("NetINetwork::pollMessages DONE\n");
        RecRecorder::instance().recordingAllowed(true);
    }
}

bool NetINetwork::haveMessages()
{
    return (messageBuffer_.size() > 0);
}

NetMessage* NetINetwork::getMessage()
{

    PRE(isValidNoRecord());

    NetMessage* pFrontMessage = messageBuffer_.front();
    messageBuffer_.erase(messageBuffer_.begin());

    POST(pFrontMessage);
    PRE(isValidNoRecord());
    return pFrontMessage;
}

void NetINetwork::sendMessage(
    const NetPriority& priority,
    const NetMessageRecipients& /*to*/,
    const NetMessageBody& body)
{
    enet_uint32 flag = 0;
    if (priority.urgency() < 2)
        flag = ENET_PACKET_FLAG_RELIABLE;
    /* Create a reliable packet */
    ENetPacket* packet = enet_packet_create(body.body(), body.lengthInBytes(), flag);

    // Lets broadcast this message to all
    enet_host_broadcast(pHost_, 0, packet);

    NetNetwork::instance().netINetwork().addSentMessage(body.lengthInBytes());
}

// Host a game
NetAppSession* NetINetwork::createAppSession(const std::string& gameName)
{
    PRE(isValidNoRecord());

    deinitServersDiscoverySocket();
    initLocalServerDiscovery();
    initHost(true);

    gameName_ = gameName;

    return nullptr;
}

NetAppSession* NetINetwork::joinAppSession(const std::string& addressStr)
{
    PRE(isValidNoRecord());

    spdlog::info("NetINetwork: Connecting to {}", addressStr);
    initHost();

    std::string ipAddress = std::string(getIp(addressStr));
    std::optional<uint16_t> port = getPort(addressStr);

    // c. Connect and user service
    ENetAddress address;
    enet_address_set_host(&address, ipAddress.c_str());
    address.port = port.value_or(GamePort);

    ENetPeer* pPeer;
    pPeer = enet_host_connect(pHost_, &address, 2, 0);

    if (pPeer == nullptr)
    {
        std::cerr << "No available peers for initializing an ENet connection" << std::endl;
        // exit(EXIT_FAILURE);
        currentStatus(NetNetwork::NETNET_CONNECTIONERROR);
        return nullptr;
    }

    // Introduce yourself - send a name packet
    ENetEvent event;
    if (enet_host_service(pHost_, &event, 3000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    {
        NETWORK_STREAM("Joining session succeeded.\n");
        // peers_.push_back(pPeer);

        sendInitPacket(pPeer);
    }
    else
    {
        enet_peer_reset(pPeer);
        NETWORK_STREAM("Could not join session.\n");
        currentStatus(NetNetwork::NETNET_SESSIONERROR);
        return nullptr;
    }
    return nullptr;
}

NetAppSession* NetINetwork::connectAppSession()
{
    NETWORK_STREAM("NetINetwork::connectAppSession()\n");
    PRE(isValidNoRecord());

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        isLogicalHost_ = NetIRecorder::instance().playbackIsLogicalHost();
        localPlayerName_ = NetIRecorder::instance().playbackLocalPlayerName();
    }
    else
    {
    }

    POST(isValidNoRecord());
    POST(pLocalSession_)

    NETWORK_STREAM("NetINetwork::connectAppSession() DONE\n");
    return pLocalSession_;
}

void NetINetwork::resetAppSession()
{
    if (isLogicalHost_)
    {
        deinitLocalServerDiscovery();
    }

    resetHost();
    initServersDiscoverySocket();
}

NetAppSession& NetINetwork::session()
{
    PRE(isValidNoRecord());
    PRE(pLocalSession_);
    return *pLocalSession_;
}

void NetINetwork::clearSessions()
{
    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        RecRecorder::instance().recordingAllowed(false);

        PRE(isValidNoRecord());
        sessions_.clear();
        RecRecorder::instance().recordingAllowed(true);
    }
}

// called periodically by update() to ensure that sessions_ remains current.
void NetINetwork::updateSessions()
{
    lastSessionsUpdate_ = DevTime::instance().time();
    sendLocalServersDiscoveryBroadcast();
}

const NetNetwork::ProtocolMap& NetINetwork::availableProtocols(Update update)
{
    NETWORK_STREAM("NetINetwork::availableProtocols\n");
    NETWORK_INDENT(2);
    PRE(currentStatusNoRecord() == NetNetwork::NETNET_OK);

    NetNetwork::ProtocolMap& availableProtocols = NetNetwork::instance().netINetwork().availableProtocols_;

    if (update == Update::Yes)
    {
        NETWORK_STREAM("UPDATE\n");
        NetNetwork::ProtocolMap::iterator i = availableProtocols.begin();
        NetNetwork::ProtocolMap::iterator j = availableProtocols.end();

        if (i != j)
            availableProtocols.erase(i, j);

        if (RecRecorder::instance().state() == RecRecorder::PLAYING)
        {
            NetIRecorder::instance().playbackAvailableProtocols(&availableProtocols);
        }
        else
        {
            NetNetwork::ProtocolMap* pAvailableProtocols = &availableProtocols;
            pAvailableProtocols->insert("UDP connection for IP v4", NetNetwork::NetworkProtocol::UDP);

            if (RecRecorder::instance().state() == RecRecorder::RECORDING)
            {
                NetIRecorder::instance().recordAvailableProtocols(availableProtocols);
            }
        }

        i = availableProtocols.begin();
        j = availableProtocols.end();
    }

    POST(currentStatusNoRecord() == NetNetwork::NETNET_OK);
    NETWORK_INDENT(-2);
    NETWORK_STREAM(
        "NetINetwork::availableProtocols return " << (void*)&availableProtocols << " elements "
                                                  << availableProtocols.size() << std::endl);
    return availableProtocols;
}

void NetINetwork::clearProtocols()
{
    NETWORK_STREAM("NetINetwork::clearProtocols\n");
    NETWORK_INDENT(2);
    PRE(currentStatusNoRecord() == NetNetwork::NETNET_OK);

    NETWORK_STREAM("there are " << availableProtocols_.size() << " entries in the map" << std::endl);

    NetNetwork::ProtocolMap::iterator i = availableProtocols_.begin();
    NetNetwork::ProtocolMap::iterator j = availableProtocols_.end();

    NETWORK_STREAM(" iterator to begin is " << (void*)&(*i) << std::endl);
    NETWORK_STREAM(" iterator to end   is " << (void*)&(*j) << std::endl);
    NETWORK_STREAM(" iterator i == iterator j " << bool(i == j) << std::endl);

    NETWORK_STREAM(" delete all the array data " << std::endl);
    for (; i != j; ++i)
    {
        NETWORK_STREAM("delete for " << (*i).first << std::endl);
    }

    i = availableProtocols_.begin();

    NETWORK_STREAM(" erase elements\n");
    if (i != j)
        availableProtocols_.erase(i, j);

    POST(currentStatusNoRecord() == NetNetwork::NETNET_OK);
    NETWORK_INDENT(-2);
    NETWORK_STREAM("NetINetwork::clearProtocols DONE\n");
}

void NetINetwork::chooseProtocol(const std::string& protocolName, NetNetwork::InitialiseConnection /*initConnection*/)
{
    const NetNetwork::ProtocolMap& availableProtocolsConst = NetINetwork::availableProtocols();
    NetNetwork::ProtocolMap& availableProtocols = _CONST_CAST(NetNetwork::ProtocolMap&, availableProtocolsConst);

    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        RecRecorder::instance().recordingAllowed(false);

        NETWORK_STREAM("NetINetwork::chooseProtocol\n");
        PRE(currentStatusNoRecord() == NetNetwork::NETNET_OK);

        setProtocolName(protocolName);

        if (strstr(protocolName.c_str(), "UDP"))
        {
            currentProtocol_ = NetNetwork::NetworkProtocol::UDP;
            NETWORK_STREAM(" NetNetwork::UDP\n");
        }
        else if (strstr(protocolName.c_str(), "IPX "))
        {
            currentProtocol_ = NetNetwork::NetworkProtocol::IPX;
            NETWORK_STREAM(" NetNetwork::IPX\n");
        }
        else if (strstr(protocolName.c_str(), " TCP/IP "))
        {
            currentProtocol_ = NetNetwork::NetworkProtocol::TCPIP;
            NETWORK_STREAM(" NetNetwork::TCPIP\n");
        }
        else if (strstr(protocolName.c_str(), "Serial "))
        {
            currentProtocol_ = NetNetwork::NetworkProtocol::SERIAL;
            NETWORK_STREAM(" NetNetwork::SERIAL\n");
        }
        else if (strstr(protocolName.c_str(), "Modem "))
        {
            currentProtocol_ = NetNetwork::NetworkProtocol::MODEM;
            NETWORK_STREAM(" NetNetwork::MODEM\n");
        }
        else if (strstr(protocolName.c_str(), " Zone "))
        {
            currentProtocol_ = NetNetwork::NetworkProtocol::ZONE;
            NETWORK_STREAM(" NetNetwork::ZONE\n");
        }
        else
        {
            currentProtocol_ = NetNetwork::NetworkProtocol::OTHER;
            NETWORK_STREAM(" NetNetwork::OTHER\n");
        }

        // if( initConnection == NetNetwork::INITIALISE_CONNECTION )
        {
            // a. Initialize enet
            NETWORK_STREAM("NetINetwork::chooseProtocol init ENet\n");
            if (enet_initialize() != 0)
            {
                std::cerr << "An error occured while initializing ENet." << std::endl;
                // return EXIT_FAILURE;
                return;
            }

            atexit(enet_deinitialize);
        }
        RecRecorder::instance().recordingAllowed(true);
    }
}

void NetINetwork::initialiseConnection()
{
    const NetNetwork::ProtocolMap& availableProtocolsConst = NetINetwork::availableProtocols();
    NetNetwork::ProtocolMap& availableProtocols = _CONST_CAST(NetNetwork::ProtocolMap&, availableProtocolsConst);

    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        RecRecorder::instance().recordingAllowed(false);

        NETWORK_STREAM("NetINetwork::initialiseConnection\n");

        NETWORK_STREAM("NetINetwork::initialiseConnection DONE\n");

        RecRecorder::instance().recordingAllowed(true);
    }
}

bool NetINetwork::hasAppSession(const NetAppSessionName& appSessionName) const
{
    PRE(isValidNoRecord());

    bool found = false;

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        found = NetIRecorder::instance().playbackHasAppSession();
    }
    else
    {
        found = hasAppSessionNoRecord(appSessionName);

        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordHasAppSession(found);
        }
    }

    POST(isValidNoRecord());

    return found;
}

bool NetINetwork::hasAppSessionNoRecord(const NetAppSessionName& appSessionName) const
{
    PRE(isValidNoRecord());

    bool found = false;

    NetNetwork::Sessions::const_iterator i = sessions_.begin();
    NetNetwork::Sessions::const_iterator j = sessions_.end();

    // while (i != j && ! found)
    // {
    //     if ((*i)->appSessionName() == appSessionName)
    //         found = true;
    //     ++i;
    // }

    POST(isValidNoRecord());

    return found;
}

bool NetINetwork::isValid() const
{
    return (currentStatus() == NetNetwork::NETNET_OK);
}

bool NetINetwork::isValidNoRecord() const
{
    return (currentStatusNoRecord() == NetNetwork::NETNET_OK);
}

// static
const std::string& NetINetwork::protocolName()
{
    std::string& currentProtocolName = protocolNameNoRecord();

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        currentProtocolName = NetIRecorder::instance().playbackCurrentProtocolName();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordCurrentProtocolName(currentProtocolName);
        }
    }

    return currentProtocolName;
}

// static
std::string& NetINetwork::protocolNameNoRecord()
{
    static std::string currentProtocolName;

    return currentProtocolName;
}

// static
void NetINetwork::setProtocolName(const std::string& name)
{
    protocolNameNoRecord() = name;
}

void NetINetwork::setAppUid()
{
    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        RecRecorder::instance().recordingAllowed(false);

        NETWORK_STREAM("NetINetwork::setAppUid\n");
        const SysPathName pathName("appguid.ini");

        SysMetaFile metaFile("mach1.met");

        std::unique_ptr<std::istream> pGuidStream;

        if (SysMetaFile::useMetaFile())
        {
            pGuidStream = std::unique_ptr<std::istream>(new SysMetaFileIstream(metaFile, pathName, std::ios::in));
        }
        else
        {
            ASSERT_FILE_EXISTS(pathName.c_str());
            pGuidStream = std::unique_ptr<std::istream>(new std::ifstream(pathName.c_str(), std::ios::in));
        }

        NETWORK_STREAM(" close appguid.ini\n");

        std::unique_ptr<std::istream> pGuidStream2;

        if (SysMetaFile::useMetaFile())
        {
            pGuidStream2 = std::unique_ptr<std::istream>(new SysMetaFileIstream(metaFile, pathName, std::ios::in));
        }
        else
        {
            ASSERT_FILE_EXISTS(pathName.c_str());
            pGuidStream2 = std::unique_ptr<std::istream>(new std::ifstream(pathName.c_str(), std::ios::in));
        }

        NETWORK_STREAM(" open line tokeniser with appguid.ini\n");
        UtlLineTokeniser parser(*pGuidStream2, pathName.c_str());

        static string appName = "";
        static string fileName = "";
        static string commandLine = "";
        static string description = "";
        while (! parser.finished())
        {
            const string& token = parser.tokens()[0];
            if (token == "application")
            {
                appName = "";
                for (std::size_t i = 1; i < parser.tokens().size(); ++i)
                {
                    appName += parser.tokens()[i];
                    if (i < parser.tokens().size() - 1)
                        appName += " ";
                }
            }
            else if (token == "filename")
            {
                fileName = "";
                for (std::size_t i = 1; i < parser.tokens().size(); ++i)
                {
                    fileName += parser.tokens()[i];
                    if (i < parser.tokens().size() - 1)
                        fileName += " ";
                }
            }
            else if (token == "commandline")
            {
                commandLine = "";
                for (std::size_t i = 1; i < parser.tokens().size(); ++i)
                {
                    commandLine += parser.tokens()[i];
                    if (i < parser.tokens().size() - 1)
                        commandLine += " ";
                }
            }
            else if (token == "description")
            {
                description = "";
                for (std::size_t i = 1; i < parser.tokens().size(); ++i)
                {
                    description += parser.tokens()[i];
                    if (i < parser.tokens().size() - 1)
                        description += " ";
                }
            }
            else if (token == "maxBytesPerSecond")
            {
                maxBytesPerSecond_ = atol(parser.tokens()[1].c_str());
            }
            parser.parseNextLine();
        }
        ASSERT(appName.length() > 0, "The application name must be supplied to register the app.\n");
        NETWORK_STREAM(" file parsed\n");

        RecRecorder::instance().recordingAllowed(true);
    }
}

// static
void NetINetwork::resetStatus()
{
    if (RecRecorder::instance().state() != RecRecorder::PLAYING)
    {
        RecRecorder::instance().recordingAllowed(false);

        currentStatus(NetNetwork::NETNET_OK);
        //      NetIAppSession::currentStatusNoRecord() = NetAppSession::NETSESS_OK;
        //      NetNode::currentStatusNoRecord() = NetNode::NETNODE_OK;

        RecRecorder::instance().recordingAllowed(true);
    }
}

// static
const NetNetwork::NetNetworkStatus& NetINetwork::currentStatus()
{
    NetNetwork::NetNetworkStatus& status = currentStatusNoRecord();

    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        status = NetIRecorder::instance().playbackCurrentStatus();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordCurrentStatus(status);
        }
    }

    return status;
}

NetNetwork::NetNetworkStatus& NetINetwork::currentStatusNoRecord()
{
    static NetNetwork::NetNetworkStatus currentStatus = NetNetwork::NETNET_OK;

    if (currentStatus == NetNetwork::NETNET_OK)
    {
        /*      if(NetIAppSession::currentStatusNoRecord() != NetAppSession::NETSESS_OK)
            currentStatus = NetNetwork::NETNET_SESSIONERROR;
        else if(NetNode::currentStatusNoRecord() != NetNode::NETNODE_OK)
            currentStatus = NetNetwork::NETNET_NODEERROR;*/
    }

    return currentStatus;
}

void NetINetwork::currentStatus(NetNetwork::NetNetworkStatus status)
{
    currentStatusNoRecord() = status;
}

// static
NetAppUid NetINetwork::appUid_ = 0;

void NetINetwork::systemMessageHandler(NetSystemMessageHandler* pMessageHandler)
{
    if (pSystemMessageHandler_)
        delete pSystemMessageHandler_;
    pSystemMessageHandler_ = pMessageHandler;
}

void NetINetwork::doAbort(const string& reason)
{
    NETWORK_STREAM("NetINetwork::doAbort " << reason << std::endl);
}

bool NetINetwork::isLobbiedGame() const
{
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        NetINetwork* nonConstThis = _CONST_CAST(NetINetwork*, this);

        nonConstThis->isLobbiedGame_ = NetIRecorder::instance().playbackIsLobbiedGame();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordIsLobbiedGame(isLobbiedGame_);
        }
    }

    return isLobbiedGame_;
}

bool NetINetwork::isLogicalHost() const
{
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        NetINetwork* nonConstThis = _CONST_CAST(NetINetwork*, this);

        nonConstThis->isLogicalHost_ = NetIRecorder::instance().playbackIsLogicalHost();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordIsLogicalHost(isLogicalHost_);
        }
    }

    return isLogicalHost_;
}

const std::string& NetINetwork::localPlayerName() const
{
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        NetINetwork* nonConstThis = _CONST_CAST(NetINetwork*, this);

        nonConstThis->localPlayerName_ = NetIRecorder::instance().playbackLocalPlayerName();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordLocalPlayerName(localPlayerName_);
        }
    }

    return localPlayerName_;
}

void NetINetwork::setLocalPlayerName(const std::string& newName)
{
    localPlayerName_ = newName;
}

NetNetwork::NetworkProtocol NetINetwork::currentProtocol() const
{
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        NetINetwork* nonConstThis = _CONST_CAST(NetINetwork*, this);

        nonConstThis->currentProtocol_ = NetIRecorder::instance().playbackCurrentProtocol();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordCurrentProtocol(currentProtocol_);
        }
    }

    return currentProtocol_;
}

const std::string& NetINetwork::IPAddress() const
{
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        NetINetwork* nonConstThis = _CONST_CAST(NetINetwork*, this);

        nonConstThis->IPAddress_ = NetIRecorder::instance().playbackIPAddress();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordIPAddress(IPAddress_);
        }
    }

    return IPAddress_;
}

void NetINetwork::setIPAddress(const std::string& newIPAddress)
{
    IPAddress_ = newIPAddress;
}

bool NetINetwork::imStuffed() const
{
    if (RecRecorder::instance().state() == RecRecorder::PLAYING)
    {
        NetINetwork* nonConstThis = _CONST_CAST(NetINetwork*, this);

        nonConstThis->imStuffed_ = NetIRecorder::instance().playbackImStuffed();
    }
    else
    {
        if (RecRecorder::instance().state() == RecRecorder::RECORDING)
        {
            NetIRecorder::instance().recordImStuffed(imStuffed_);
        }
    }

    return imStuffed_;
}

bool NetINetwork::imStuffedNoRecord() const
{
    return imStuffed_;
}

bool NetINetwork::hasSystemMessageHandler() const
{
    return pSystemMessageHandler_ != nullptr;
}

NetSystemMessageHandler& NetINetwork::systemMessageHandler()
{
    return *pSystemMessageHandler_;
}

bool NetINetwork::deterministicPingDropoutAllowed() const
{
    return deterministicPingDropoutAllowed_;
}

void NetINetwork::setDeterministicPingDropoutAllowed(bool newValue)
{
    deterministicPingDropoutAllowed_ = newValue;
}

bool operator<(const NetMessageShortInfo& a, const NetMessageShortInfo& b)
{
    if (a.time_ < b.time_)
        return true;
    if (a.time_ != b.time_)
        if (a.length_ < b.length_)
            return true;
    return false;
}
bool operator==(const NetMessageShortInfo& a, const NetMessageShortInfo& b)
{
    if (a.time_ == b.time_)
        return true;
    if (a.time_ != b.time_)
        if (a.length_ == b.length_)
            return true;
    return false;
}

void NetINetwork::addSentMessage(int length)
{
    double time = timer_.time();
    NetMessageShortInfo* pInfo = new NetMessageShortInfo;
    pInfo->time_ = time;
    pInfo->length_ = length;
    sentMessages_.push_back(pInfo);
    computeSentMessageStuffedNess();
}

void NetINetwork::computeSentMessageStuffedNess()
{
    if (messageThrottlingActive_)
    {
        double time = timer_.time();
        double targetTime = time - 1.0;
        SentMessages messagesForDeletion;
        // Now iterate through list and remove any elements older than one second from now.
        for (SentMessages::iterator i = sentMessages_.begin(); i != sentMessages_.end(); ++i)
            if ((*i)->time_ < targetTime)
                messagesForDeletion.push_back(*i);
        for (SentMessages::iterator i = messagesForDeletion.begin(); i != messagesForDeletion.end(); ++i)
        {
            sentMessages_.erase(find(sentMessages_.begin(), sentMessages_.end(), *i));
            delete *i;
        }
        // Now iterator through the current collection and add up all the lengths - thats the current bandwidth usage
        // if it goes above maxBytesPerSecond_ bytes sec -1 then set stuffed to true.
        int totalLength = 0;
        for (SentMessages::iterator i = sentMessages_.begin(); i != sentMessages_.end(); ++i)
            totalLength += (*i)->length_;
        if (totalLength > maxBytesPerSecond_ || sentMessages_.size() > maxSentMessagesPerSecond())
            imStuffed_ = true;
    }
    //  NETWORK_STREAM("NetINetwork::computeSentMessageStuffedNess No. messages " << sentMessages_.size() << "
    //  totalLength " << totalLength << " maxBytesPerSecond " << maxBytesPerSecond_ << " maxSentMessagesPerSecond() " <<
    //  maxSentMessagesPerSecond() << std::endl );
}

double NetINetwork::deterministicTimeoutPeriod() const
{
    // 5 minutes allowed before IPX will blow up.
    if (currentProtocol() == NetNetwork::NetworkProtocol::IPX)
        return 300;
    // 10 minutes of lag allowed for Zone type games
    return 600;
}

size_t NetINetwork::maxSentMessagesPerSecond() const
{
    static bool initialisedFromRegistry = false;
    if (! initialisedFromRegistry)
    {
        NetINetwork* pMe = _CONST_CAST(NetINetwork*, this);
        bool fromLan = (currentProtocol() == NetNetwork::NetworkProtocol::IPX);
        string keyValue = "Max packets per second";

        if (fromLan)
            keyValue += " (IPX)";

        int possibleValue = SysRegistry::instance().queryIntegerValue("Network", keyValue);

        if (possibleValue == 0)
        {
            if (fromLan)
                pMe->maxSentMessagesPerSecond_ = 100;
            else
                pMe->maxSentMessagesPerSecond_ = 40;
            SysRegistry::instance().setIntegerValue("Network", keyValue, maxSentMessagesPerSecond_);
            possibleValue = maxSentMessagesPerSecond_;
        }

        if (possibleValue > 400)
            possibleValue = 400;
        else if (possibleValue < 40)
            possibleValue = 40;
        pMe->maxSentMessagesPerSecond_ = possibleValue;

        keyValue = "Max bytes per second";
        if (fromLan)
            keyValue += " (IPX)";
        possibleValue = SysRegistry::instance().queryIntegerValue("Network", keyValue);

        if (possibleValue == 0)
        {
            if (fromLan)
                pMe->maxBytesPerSecond_ = 60000;
            else
                pMe->maxBytesPerSecond_ = 6000;
            SysRegistry::instance().setIntegerValue("Network", keyValue, maxBytesPerSecond_);
            possibleValue = maxBytesPerSecond_;
        }
        if (possibleValue > 60000)
            possibleValue = 60000;
        else if (possibleValue < 6000)
            possibleValue = 6000;
        pMe->maxBytesPerSecond_ = possibleValue;

        NETWORK_STREAM(
            "NetINetwork::maxSentMessagesPerSecond initialisation maxPackets "
            << maxSentMessagesPerSecond_ << " max Bytes " << maxBytesPerSecond_ << std::endl);

        initialisedFromRegistry = true;

        pMe->originalMaxSentMessagesPerSecond_ = maxSentMessagesPerSecond_;
    }

    return maxSentMessagesPerSecond_;
}

void NetINetwork::disableNewPlayers()
{
    NETWORK_STREAM("NetINetwork::disableNewPlayers()\n");
}

bool NetINetwork::pingAllAllowed() const
{
    return pingAllAllowed_;
}

void NetINetwork::sendInitPacket(ENetPeer* pPeer)
{
    ENetPacket* packet
        = enet_packet_create(localPlayerName_.c_str(), localPlayerName_.length() + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(pPeer, 0, packet);
}

void NetINetwork::initServersDiscoverySocket()
{
    lanDiscoveryClientSocket_ = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    if (lanDiscoveryClientSocket_ == ENET_SOCKET_NULL)
    {
        spdlog::error("NetINetwork: Unable to create broadcast socket");
        return;
    }
    if (enet_socket_set_option(lanDiscoveryClientSocket_, ENET_SOCKOPT_BROADCAST, 1) != 0)
    {
        spdlog::error("NetINetwork: Unable to enable broadcast socket");
        return;
    }
}

void NetINetwork::deinitServersDiscoverySocket()
{
    if(lanDiscoveryClientSocket_ != ENET_SOCKET_NULL)
    {
        enet_socket_shutdown(lanDiscoveryClientSocket_, ENET_SOCKET_SHUTDOWN_READ_WRITE);
        enet_socket_destroy(lanDiscoveryClientSocket_);
        lanDiscoveryClientSocket_ = ENET_SOCKET_NULL;
    }
}

void NetINetwork::sendLocalServersDiscoveryBroadcast()
{
    if (lanDiscoveryClientSocket_ == ENET_SOCKET_NULL)
        return;

    ClientRequest request;
    static_assert(sizeof(request.magic) == sizeof(DiscoveryMagic));
    SDL_memcpy(request.magic, DiscoveryMagic, sizeof(DiscoveryMagic));
    request.version = System::toBigEndian(DiscoveryVersion);
    const char message[] = "DiscoverServers";
    static_assert(sizeof(request.message) == sizeof(message));
    SDL_memcpy(request.message, message, sizeof(message));

    ENetBuffer sendbuf;
    sendbuf.data = &request;
    sendbuf.dataLength = sizeof(request);

    ENetAddress scanaddr;
    scanaddr.host = ENET_HOST_BROADCAST;
    scanaddr.port = LANServerDiscoveryPort;

    if (enet_socket_send(lanDiscoveryClientSocket_, &scanaddr, &sendbuf, 1) != static_cast<int>(sendbuf.dataLength))
    {
        spdlog::warn("NetINetwork: Unable to send discovery broadcast");
        return;
    }
}

void NetINetwork::acceptLocalServersReplies()
{
    ENetSocketSet set;
    ENET_SOCKETSET_EMPTY(set);
    ENET_SOCKETSET_ADD(set, lanDiscoveryClientSocket_);

    if (!enet_socketset_select(lanDiscoveryClientSocket_, &set, nullptr, 0))
        return;

    ENetAddress serverAddress;

    ENetBuffer recvbuf;

    ServerReply reply;
    recvbuf.data = &reply;
    recvbuf.dataLength = sizeof(reply);
    const int recvlen = enet_socket_receive(lanDiscoveryClientSocket_, &serverAddress, &recvbuf, 1);

    char buf[256];
    enet_address_get_host_ip(&serverAddress, buf, sizeof buf);

    if (recvlen != sizeof(ServerReply) || (SDL_memcmp(reply.magic, DiscoveryMagic, sizeof(DiscoveryMagic)) != 0))
    {
        spdlog::warn("NetINetwork: Unexpected reply from server {}", buf);
        return;
    }

    const uint32_t replyVersion = System::fromBigEndian(reply.version);
    if (replyVersion != DiscoveryVersion)
    {
        spdlog::warn("NetINetwork: Unexpected discovery reply version {}", replyVersion);
        return;
    }

    std::string address = makeAddress(buf, reply.port);
    const auto session = std::find_if(sessions_.begin(), sessions_.end(), [&address](const NetSessionInfo& info) {
        return info.address == address;
    });
    if (session != sessions_.end())
    {
        session->serverName = reply.serverName;
        return;
    }

    sessions_.emplace_back(NetSessionInfo {
        .address = address,
        .serverName = reply.serverName,
    });
}

bool NetINetwork::initLocalServerDiscovery()
{
    if(lanDiscoveryServerSocket_ != ENET_SOCKET_NULL)
    {
        spdlog::warn("NetINetwork: LAN discovery server socket already exists");
        return false;
    }

    ENetSocket serverSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    if (serverSocket == ENET_SOCKET_NULL)
    {
        spdlog::error("NetINetwork: Unable to create LAN discovery server socket");
        return false;
    }

    ENetAddress listenaddr;
    listenaddr.host = ENET_HOST_ANY;
    listenaddr.port = LANServerDiscoveryPort;
    if (enet_socket_bind(serverSocket, &listenaddr) != 0)
    {
        spdlog::error("NetINetwork: Unable to bind LAN discovery server socket");
        enet_socket_destroy(serverSocket);
        return false;
    }
    if (enet_socket_get_address(serverSocket, &listenaddr) != 0)
    {
        spdlog::error("NetINetwork: Unable to get LAN discovery server socket address");
        enet_socket_shutdown(serverSocket, ENET_SOCKET_SHUTDOWN_READ_WRITE);
        enet_socket_destroy(serverSocket);
        return false;
    }
    spdlog::info("NetINetwork: Listening for scans on port {}", listenaddr.port);

    lanDiscoveryServerSocket_ = serverSocket;
    return true;
}

void NetINetwork::deinitLocalServerDiscovery()
{
    if(lanDiscoveryServerSocket_ != ENET_SOCKET_NULL)
    {
        enet_socket_shutdown(lanDiscoveryServerSocket_, ENET_SOCKET_SHUTDOWN_READ_WRITE);
        enet_socket_destroy(lanDiscoveryServerSocket_);
        lanDiscoveryServerSocket_ = ENET_SOCKET_NULL;
    }
}

void NetINetwork::replyToServerDiscoveryRequests()
{
    ENetSocketSet set;
    ENET_SOCKETSET_EMPTY(set);
    ENET_SOCKETSET_ADD(set, lanDiscoveryServerSocket_);

    if (!enet_socketset_select(lanDiscoveryServerSocket_, &set, nullptr, 0))
        return;

    ENetAddress clientAddress;

    ENetBuffer recvbuf;
    ClientRequest request;
    recvbuf.data = &request;
    recvbuf.dataLength = sizeof(request);
    const int recvlen = enet_socket_receive(lanDiscoveryServerSocket_, &clientAddress, &recvbuf, 1);
    if (recvlen != sizeof(ClientRequest) || (SDL_memcmp(request.magic, DiscoveryMagic, sizeof(DiscoveryMagic)) != 0))
    {
        spdlog::debug("NetINetwork: Invalid discovery request");
        return;
    }
    const uint32_t reqVersion = System::fromBigEndian(request.version);
    if (reqVersion != DiscoveryVersion)
    {
        spdlog::warn("NetINetwork: Unexpected discovery request version {}", reqVersion);
        return;
    }

    ServerReply reply;
    static_assert(sizeof(reply.magic) == sizeof(DiscoveryMagic));
    SDL_memcpy(reply.magic, DiscoveryMagic, sizeof(DiscoveryMagic));
    reply.version = System::toBigEndian(DiscoveryVersion);
    gameName_.copy(reply.serverName, sizeof(reply.serverName) - 1);
    reply.port = pHost_->address.port;

    ENetBuffer sendbuf;
    sendbuf.data = &reply;
    sendbuf.dataLength = sizeof(reply);
    if (enet_socket_send(lanDiscoveryServerSocket_, &clientAddress, &sendbuf, 1) != static_cast<int>(sendbuf.dataLength))
    {
        spdlog::warn("NetINetwork: Unable to send LAN server announce broadcast");
        return;
    }
}

void NetINetwork::determineStandardSendFlags()
{
    NETWORK_STREAM("NetINetwork::determineStandardSendFlags\n");
    NETWORK_INDENT(2);
    NETWORK_INDENT(-2);
}

void NetINetwork::messageThrottlingActive(bool value)
{
    messageThrottlingActive_ = value;
}

bool NetINetwork::messageThrottlingActive() const
{
    return messageThrottlingActive_;
}

void NetINetwork::autoAdjustMaxSentMessagesPerSecond(size_t numberOfPlayers)
{
    NETWORK_STREAM("NetINetwork::autoAdjustMaxSentMessagesPerSecond " << numberOfPlayers << std::endl);
    // ensure values are correctly initialised from the registry
    maxSentMessagesPerSecond();

    int thresholdValue = 50;

    if (currentProtocol() != NetNetwork::NetworkProtocol::IPX)
    {
        thresholdValue = 40;
    }

    maxSentMessagesPerSecond_ = originalMaxSentMessagesPerSecond_;
    // Check for lower threshold value up front.
    if (maxSentMessagesPerSecond_ <= thresholdValue)
        return;
    if (numberOfPlayers < 3)
        return;
    maxSentMessagesPerSecond_ *= (2.0f / numberOfPlayers);
    // Check for lowest threshold value.
    if (maxSentMessagesPerSecond_ < thresholdValue)
        maxSentMessagesPerSecond_ = thresholdValue;
    NETWORK_STREAM(
        " maxSentMessagesPerSecond " << maxSentMessagesPerSecond_ << " orig " << originalMaxSentMessagesPerSecond_
                                     << " threshold " << thresholdValue << std::endl);
    NETWORK_STREAM("NetINetwork::autoAdjustMaxSentMessagesPerSecond DONE\n");
}
