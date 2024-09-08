#include "network/netnet.hpp"

#include "base/diag.hpp"

#include "profiler/profiler.hpp"

#include "network/node.hpp"
#include "network/nodeuid.hpp"
#include "network/sessuid.hpp"
#include "network/session.hpp"
#include "network/messbody.hpp"

#include "network/internal/netinet.hpp"

#include "system/registry.hpp"

#define MAXNAMELEN 200

#define INITGUID

//////////////////////////////////////////////////////////////

NetNetwork::NetNetwork()
    : pNetINetwork_(nullptr)
{
    PRE(isValidNoRecord());
    pNetINetwork_ = new NetINetwork();
    NETWORK_STREAM("pNetINetwork_ is " << (void*)pNetINetwork_ << std::endl);

    std::string ipAddress = SysRegistry::instance().queryStringValue("Network", "IP Address");
    if (ipAddress.empty())
        ipAddress = "localhost";
    pNetINetwork_->setIPAddress(ipAddress);

    POST(isValidNoRecord());
}

NetNetwork::~NetNetwork()
{
    delete pNetINetwork_;
}

//////////////////////////////////////////////////////////////

// static
NetNetwork& NetNetwork::instance()
{
    static NetNetwork instance;
    return instance;
}

// static
// NetInterProcessUid NetNetwork::nextInterProcessUid()
//{
//   return NetINetwork::nextInterProcessUid();
// }

// static
const NetNetwork::ProtocolMap& NetNetwork::availableProtocols()
{
    return NetNetwork::instance().pNetINetwork_->availableProtocols();
}

// static
void NetNetwork::chooseProtocol(const std::string& protocolName, NetNetwork::InitialiseConnection initConnection)
{
    NetNetwork::instance().pNetINetwork_->chooseProtocol(protocolName, initConnection);
}

// static
void NetNetwork::resetStatus()
{
    NetINetwork::resetStatus();
}

// static
NetNetwork::NetNetworkStatus NetNetwork::currentStatus()
{
    NetNetwork::NetNetworkStatus status;

    status = NetINetwork::currentStatus();

    return status;
}

// static
NetNetwork::NetNetworkStatus NetNetwork::currentStatusNoRecord()
{
    NetNetwork::NetNetworkStatus status;

    status = NetINetwork::currentStatusNoRecord();

    return status;
}

//////////////////////////////////////////////////////////////

NetAppUid NetNetwork::appUid() const
{
    PRE(isValidNoRecord());

    NetAppUid appUid;

    appUid = pNetINetwork_->appUid();

    return appUid;
}

NetAppUid NetNetwork::appUidNoRecord() const
{
    PRE(isValidNoRecord());

    NetAppUid appUid;

    appUid = pNetINetwork_->appUidNoRecord();

    return appUid;
}

bool NetNetwork::hasAppSession(NetAppSessionUid aUid) const
{
    PRE(isValidNoRecord());

    bool result;

    result = pNetINetwork_->hasAppSession(aUid);

    return result;
}

void NetNetwork::update()
{
    PRE(isValidNoRecord());
    pNetINetwork_->update();
    POST(isValidNoRecord());
}

void NetNetwork::pollMessages()
{
    if (isValid())
        pNetINetwork_->pollMessages();
}

const NetNetwork::Sessions& NetNetwork::sessions() const
{
    PRE(isValidNoRecord());
    return pNetINetwork_->sessions();
}

NetAppSession* NetNetwork::createAppSession()
{
    PRE(isValidNoRecord());
    return pNetINetwork_->createAppSession();
}

NetAppSession* NetNetwork::joinAppSession(const std::string& addressStr)
{
    PRE(isValidNoRecord());
    return pNetINetwork_->joinAppSession(addressStr);
}

NetAppSession* NetNetwork::connectAppSession()
{
    PRE(isValidNoRecord());
    return pNetINetwork_->connectAppSession();
}

//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////

bool NetNetwork::isValid() const
{
    return (currentStatus() == NETNET_OK);
}

bool NetNetwork::isValidNoRecord() const
{
    return (currentStatusNoRecord() == NETNET_OK);
}

//////////////////////////////////////////////////////////////
void NetNetwork::systemMessageHandler(NetSystemMessageHandler* pMessageHandler)
{
    pNetINetwork_->systemMessageHandler(pMessageHandler);
}

void NetNetwork::doAbort(const string& reason)
{
    pNetINetwork_->doAbort(reason);
}

NetINetwork& NetNetwork::netINetwork() const
{
    //  NETWORK_STREAM("NetNetwork::netINetwork() returning " << (void*)pNetINetwork_ << std::endl );

    return *pNetINetwork_;
}

bool NetNetwork::isLobbiedGame() const
{
    return pNetINetwork_->isLobbiedGame();
}

bool NetNetwork::isLogicalHost() const
{
    return pNetINetwork_->isLogicalHost();
}

const std::string& NetNetwork::localPlayerName() const
{
    return pNetINetwork_->localPlayerName();
}

void NetNetwork::setLocalPlayerName(const std::string& name)
{
    pNetINetwork_->setLocalPlayerName(name);
}

void NetNetwork::instantiateLobby()
{
    // call into the lobby instantiation function
    //   pNetINetwork_->LPDirectPlayLobby();
}

NetNetwork::NetworkProtocol NetNetwork::currentProtocol() const
{
    return pNetINetwork_->currentProtocol();
}

const std::string& NetNetwork::IPAddress() const
{
    return pNetINetwork_->IPAddress();
}

void NetNetwork::setIPAddress(const std::string& newAddress)
{
    SysRegistry::instance().setStringValue("Network", "IP Address", newAddress);

    pNetINetwork_->setIPAddress(newAddress);
}

void NetNetwork::initialiseConnection()
{
    pNetINetwork_->initialiseConnection();
}

bool NetNetwork::imStuffed() const
{
    return pNetINetwork_->imStuffed();
}

bool NetNetwork::imStuffedNoRecord() const
{
    return pNetINetwork_->imStuffedNoRecord();
}

void NetNetwork::imStuffed(bool newImStuffed)
{
    //  if( pNetINetwork_->imStuffed_ != newImStuffed )
    //  {
    //      NETWORK_STREAM("STUFFED change of state to " << newImStuffed << " called from:\n" );
    //      ProProfiler::instance().traceStack( Diag::instance().networkStream(), true, 0, "NetNetwork::imStuffed" );
    //  }
    pNetINetwork_->imStuffed_ = newImStuffed;
}

std::ostream& operator<<(std::ostream& o, NetNetwork::NetworkProtocol protocol)
{
    switch (protocol)
    {
        case NetNetwork::NetworkProtocol::IPX:
            o << "IPX";
            break;
        case NetNetwork::NetworkProtocol::TCPIP:
            o << "TCPIP";
            break;
        case NetNetwork::NetworkProtocol::MODEM:
            o << "MODEM";
            break;
        case NetNetwork::NetworkProtocol::SERIAL:
            o << "SERIAL";
            break;
        case NetNetwork::NetworkProtocol::ZONE:
            o << "ZONE";
            break;
        case NetNetwork::NetworkProtocol::OTHER:
            o << "OTHER";
            break;
        default:
            o << "Unknown PROTOCOL type " << (int)protocol << std::endl;
    }
    return o;
}

bool NetNetwork::hasSystemMessageHandler() const
{
    return pNetINetwork_->hasSystemMessageHandler();
}

NetSystemMessageHandler& NetNetwork::systemMessageHandler()
{
    return pNetINetwork_->systemMessageHandler();
}

bool NetNetwork::deterministicPingDropoutAllowed() const
{
    return pNetINetwork_->deterministicPingDropoutAllowed();
}

void NetNetwork::setDeterministicPingDropoutAllowed(bool newValue)
{
    pNetINetwork_->setDeterministicPingDropoutAllowed(newValue);
}

void NetNetwork::disableNewPlayers()
{
    pNetINetwork_->disableNewPlayers();
}

void NetNetwork::outputStuffedInfo(std::ostream& o)
{
    o << "******************************************\n";
    o << "*      N E T W O R K  S T U F F E D      *\n";
    o << "******************************************\n";
    o << "Messages: " << pNetINetwork_->sentMessages_.size() << std::endl;

    if (pNetINetwork_->pLocalSession_)
    {
    }
}

void NetNetwork::outputCurrentStatistics(std::ostream& o)
{
    o << "Network Current Statistics:\n";
    o << " sentMessages " << pNetINetwork_->sentMessages_.size() << std::endl;
    if (pNetINetwork_->pLocalSession_)
    {
    }
}

void NetNetwork::messageThrottlingActive(bool value)
{
    pNetINetwork_->messageThrottlingActive(value);
}

bool NetNetwork::messageThrottlingActive() const
{
    return pNetINetwork_->messageThrottlingActive();
}

void NetNetwork::autoAdjustMaxPacketsPerSecond(size_t numberOfPlayers)
{
    pNetINetwork_->autoAdjustMaxSentMessagesPerSecond(numberOfPlayers);
}

int NetNetwork::maxSentMessagesPerSecond() const
{
    return pNetINetwork_->maxSentMessagesPerSecond_;
}
NetMessage* NetNetwork::getMessage()
{
    return pNetINetwork_->getMessage();
}
bool NetNetwork::haveMessages()
{
    return pNetINetwork_->haveMessages();
}
void NetNetwork::sendMessage(const NetPriority& priority, const NetMessageBody& body)
{
    NetMessageRecipients to;
    pNetINetwork_->sendMessage(priority, to, body);
}
