/*
 * N E T W O R K . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "machlog/Messaging/Network.hpp"

#include "profiler/StackTracer.hpp"
#include "mathex/Point2d.hpp"
#include "utility/LineTokeniser.hpp"
#include "network/NetDefs.hpp"
#include "network/Network.hpp"
#include "network/Session.hpp"
#include "network/NodeUid.hpp"
#include "machlog/Messaging/MessageBroker.hpp"
#include "machlog/World/Scenario.hpp"
#include "machlog/Internal/NetworkImpl.hpp"
#include "machphys/Random.hpp"

#include <memory>

#include "spdlog/spdlog.h"

// static
MachLogNetwork& MachLogNetwork::instance()
{
    static MachLogNetwork instance_;
    return instance_;
}

MachLogNetwork::MachLogNetwork()
    : pImpl_(new MachLogNetworkDataImpl)
{
    CB_MachLogNetwork_DEPIMPL();

    expectedPlayers_ = 2;
    pBroker_ = nullptr;
    terminateAndReset();
    //  NetNetwork::instance().directXProtocol( true );
    //  NetNetwork::instance().asyncMessaging( true );
    //  NetNetwork::instance().guaranteed( true );
    DEBUG_STREAM(DIAG_NETWORK, "MachLogNetwork::CTOR\n");
    TEST_INVARIANT;
}

MachLogNetwork::~MachLogNetwork()
{
    TEST_INVARIANT;

    CB_MachLogNetwork_DEPIMPL();

    DEBUG_STREAM(DIAG_NETWORK, "MachLogNetwork::DTOR\n");
    terminateAndReset();

    delete pImpl_;
}

void MachLogNetwork::terminateAndReset()
{
    CB_MachLogNetwork_DEPIMPL();

    spdlog::debug("MachLogNetwork::terminateAndReset()");
    NETWORK_STREAM(ProStackTracer());

    // do not delete the broker!
    //   if( pBroker_ )
    //       delete pBroker_;
    //   pBroker_ = NULL;
    isNetworkGame_ = false;
    isNodeLogicalHost_ = false;
    for (bool& status : pImpl_->readyStatus_)
        status = false;
    desiredProtocol_.reset();
    protocolChosen_ = false;
    NetNetwork::instance().resetStatus();
    DEBUG_STREAM(DIAG_NETWORK, "MachLogNetwork::terminateAndReset done\n");
}

MachPhys::Race MachLogNetwork::localRace() const
{
    CB_DEPIMPL(MachPhys::Race, localRace_);

    return localRace_;
}

bool MachLogNetwork::isNetworkGame() const
{
    CB_DEPIMPL(bool, isNetworkGame_);

    return isNetworkGame_;
}

bool MachLogNetwork::isNodeLogicalHost() const
{
    CB_DEPIMPL(bool, isNodeLogicalHost_);

    return isNodeLogicalHost_;
}

MachLogNetwork::Remote MachLogNetwork::remoteStatus(MachPhys::Race race) const
{
    CB_DEPIMPL_ARRAY(MachLogNetwork::Remote, remote_);

    return remote_[race];
}

void MachLogNetwork::remoteStatus(MachPhys::Race race, MachLogNetwork::Remote remote)
{
    CB_DEPIMPL_ARRAY(MachLogNetwork::Remote, remote_);

    remote_[race] = remote;
}

std::optional<NetNetwork::NetworkProtocol> MachLogNetwork::desiredProtocol() const
{
    CB_DEPIMPL_AUTO(desiredProtocol_);

    return desiredProtocol_;
}

void MachLogNetwork::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogNetwork& t)
{

    o << "MachLogNetwork " << static_cast<const void*>(&t) << " start" << std::endl;
    //  o << " local Race " << t.localRace_ << std::endl;
    o << "MachLogNetwork " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachLogNetwork::setBroker(MachLogMessageBroker* pBroker)
{
    CB_DEPIMPL(MachLogMessageBroker*, pBroker_);

    pBroker_ = pBroker;
}

void MachLogNetwork::update()
{
    CB_MachLogNetwork_DEPIMPL();

    if (! isNetworkGame_)
        return;

    // NETWORK_STREAM( "MachLogNetwork::update " );
    // NETWORK_STREAM( "NetNetwork::instance().session().nodes().size() " <<
    // NetNetwork::instance().session().nodes().size() << std::endl );
    //   I am removing this call to update the nodes as I am hoping this will give us more messages coming through
    //   NetNetwork::instance().session().updateNodes();
    NetNetwork::instance().pollMessages();
    if (NetNetwork::instance().currentStatus() == NetNetwork::NETNET_CONNECTIONERROR)
    {
        // NETWORK_STREAM("NetNetwork has gone to NETNET_CONNECTIONERROR after pollmessages call\n" );
    }
    // remember that the network game may have dropped out due to system messages
    // in the pollmessages call above...therefore check that game is still running.
    if (! isNetworkGame())
        return;
    if (pBroker_)
    {
        //  ASSERT( pBroker_ != NULL,"message broker is NULL and trying to call messageBroker method\n" );
        MachLogMessageBroker& broker = messageBroker();
        // network game status _may_ be changed by processing a message
        while (isNetworkGame_ && NetNetwork::instance().haveMessages())
        {
            // NETWORK_STREAM( " gunna process a message..\n" );
            broker.processMessage(NetNetwork::instance().getMessage());
        }

        // send any messages that the broker had to cache.
        // NETWORK_STREAM( "MLNetwork::update has broker cached messages? " << broker.hasCachedOutgoingMessages() <<
        // std::endl );
        if (isNetworkGame_ && broker.hasCachedOutgoingMessages())
            broker.sendCachedOutgoingMessages();
    }

    // NETWORK_STREAM(  "MachLogNetwrok::update DONE\n" );
}

MachLogMessageBroker& MachLogNetwork::messageBroker()
{
    CB_DEPIMPL(MachLogMessageBroker*, pBroker_);

    return *pBroker_;
}

void MachLogNetwork::ready(MachPhys::Race race, bool newValue)
{
    CB_DEPIMPL_ARRAY(bool, readyStatus_);

    readyStatus_[race] = newValue;
}

bool MachLogNetwork::ready(MachPhys::Race race) const
{
    CB_DEPIMPL_ARRAY(bool, readyStatus_);

    return readyStatus_[race];
}

int MachLogNetwork::expectedPlayers() const
{
    CB_DEPIMPL(int, expectedPlayers_);

    return expectedPlayers_;
}

bool MachLogNetwork::setDesiredProtocol(NetNetwork::NetworkProtocol protocol)
{
    CB_MachLogNetwork_DEPIMPL();

    const NetNetwork::ProtocolList& availableProtocols = NetNetwork::availableProtocols();
    bool protocolAvailable = false;
    for (NetNetwork::NetworkProtocol availableProtocol : availableProtocols)
    {
        if (availableProtocol == protocol)
        {
            protocolAvailable = true;
            break;
        }
    }
    if (!protocolAvailable)
    {
        return false;
    }

    NetNetwork::chooseProtocol(protocol, NetNetwork::DO_NOT_INITIALISE_CONNECTION);
    desiredProtocol_ = protocol;
    protocolChosen_ = true;
    return true;
}

void MachLogNetwork::initialiseConnection()
{
    NetNetwork::instance().initialiseConnection();
}

bool MachLogNetwork::hostSession(const std::string& gameName, const std::string& playerName)
{
    CB_MachLogNetwork_DEPIMPL();

    PRE(protocolChosen_);
    NetNetwork::instance().createAppSession(gameName);
    //  DWORD dwNameSize = 200;
    //  char szSessionName[200];
    //  GetComputerName(szSessionName, &dwNameSize);
    // NetNodeName name(playerName);
    NetNetwork::instance().setLocalPlayerName(playerName);
    if (NetNetwork::instance().currentStatus() == NetNetwork::NETNET_OK)
    {
        isNetworkGame_ = true;
        isNodeLogicalHost_ = true;
    }
    return true;
}

void MachLogNetwork::beginJoinSession(const std::string& address, const std::string& playerName)
{
    CB_MachLogNetwork_DEPIMPL();

    NetNetwork::instance().setLocalPlayerName(playerName);
    NetNetwork::instance().beginJoinAppSession(address);
    isNetworkGame_ = true;
}

void MachLogNetwork::resetSession()
{
    NetNetwork::instance().resetAppSession();
    pImpl_->isNetworkGame_ = false;
}

void MachLogNetwork::localRace(MachPhys::Race newLocalRace)
{
    CB_DEPIMPL(MachPhys::Race, localRace_);

    localRace_ = newLocalRace;
}

bool MachLogNetwork::launchFromLobbyInfo()
{
    NETWORK_STREAM("MachLogNetwork::launchFromLobbyInfo()\n");
    pImpl_->isNetworkGame_ = true;
    NETWORK_STREAM(" call connect App Session\n");
    NETWORK_INDENT(2);
    NetNetwork::instance().connectAppSession();
    if (NetNetwork::instance().isLogicalHost())
        pImpl_->isNodeLogicalHost_ = true;

    NetNodeName name(NetNetwork::instance().localPlayerName());
    NetNetwork::instance().setLocalPlayerName(name);
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachLogNetwork::launchFromLobbyInfo() DONE\n");
    return true;
}

void MachLogNetwork::isNodeLogicalHost(bool newValue)
{
    pImpl_->isNodeLogicalHost_ = newValue;
}

/* End NETWORK.CPP **************************************************/
