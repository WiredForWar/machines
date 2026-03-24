#ifndef _NETNETWORK_HPP
#define _NETNETWORK_HPP

#include "ctl/PtrVector.hpp"

#include "network/NetDefs.hpp"
#include "network/Message.hpp"
#include "network/NetSessionInfo.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class NetNode;
class NetNodeUid;
class NetProcessUid;
class NetAppSession;
class NetAppSessionUid;
class NetINetwork;
class NetSystemMessageHandler;
class NetComPortAddress;

using NetMessageBuffer = ctl_pvector<NetMessage>;

class NetNetwork
{
public:
    ///////////////////////////////

    enum NetNetworkStatus
    {
        NETNET_OK,
        NETNET_CONNECTIONERROR,
        NETNET_INVALIDCONNECTIONDATA,
        NETNET_INVALIDDATA,
        NETNET_UNDEFINEDERROR,
        NETNET_CANTINITIALIZEDIRECTPLAY,
        NETNET_MEMORYERROR,
        NETNET_SESSIONERROR,
        NETNET_NODEERROR,
        NETNET_VERSIONMISMATCH,
    };

    enum class NetworkProtocol
    {
        UDP,
        UDP_STUN,
        IPX,
        TCPIP,
        MODEM,
        SERIAL,
        ZONE,
        OTHER
    };

    using ProtocolList = std::vector<NetworkProtocol>;
    using Sessions = std::vector<NetSessionInfo>;
    using Modems = ctl_vector<std::string>;

    ///////////////////////////////

    ~NetNetwork();

    ///////////////////////////////

    static NetNetwork& instance();
    static NetInterProcessUid nextInterProcessUid();
    static const ProtocolList& availableProtocols();

    // Normally a protocol may be chosen with initialise set and the default dialogs will be displayed.
    // If you wish to override the default dialogs then set to NOT_INITIALISE, set the extra parameters via the
    // interface below and then finally call initialiseConnection explicitly.
    enum InitialiseConnection
    {
        INITIALISE_CONNECTION,
        DO_NOT_INITIALISE_CONNECTION
    };
    static void chooseProtocol(NetworkProtocol, InitialiseConnection);
    void initialiseConnection();

    static NetNetworkStatus currentStatus();
    //  Use the no record version in assertions
    static NetNetworkStatus currentStatusNoRecord();
    static void resetStatus();

    ///////////////////////////////

    bool hasAppSession(NetAppSessionUid) const;
    void update();
    void refreshSessions();
    void updateSessions();
    void pollMessages();
    const Sessions& sessions() const;
    // If hosting conventional game then use createAppSession
    NetAppSession* createAppSession(const std::string& gameName);
    void beginJoinAppSession(const std::string& addressStr);
    void updateJoin();
    void abortJoin();

    enum class JoinState
    {
        Idle,
        Connecting,
        WaitingInit,
        Done,
    };
    JoinState joinState() const;
    // if connecting via lobby then use connectAppSession (handles join and create)
    NetAppSession* connectAppSession();
    void resetAppSession();

    void systemMessageHandler(NetSystemMessageHandler*);
    bool hasSystemMessageHandler() const;
    NetSystemMessageHandler& systemMessageHandler();

    void doAbort(const std::string& reason);
    NetINetwork& netINetwork() const;

    bool isLobbiedGame() const;
    bool isLogicalHost() const;
    const std::string& localPlayerName() const;
    void setLocalPlayerName(const std::string&);
    // All this does is call into NetINetwork lpDiretcPlayLobby -
    //  which will ensure that internal flags are set correctly
    void instantiateLobby();

    NetworkProtocol currentProtocol() const;

    // Version number reported by the remote peer (server) during init handshake
    uint32_t remoteVersionNumber() const;

    // Ip addresses may be in the form of numerical IP addresses or domain net
    const std::string& IPAddress() const;
    void setIPAddress(const std::string& newAddress);
    const std::string& selectedRendezvousSessionId() const;
    void setSelectedRendezvousSessionId(const std::string& sessionId);

    bool imStuffed() const;
    bool imStuffedNoRecord() const;
    void imStuffed(bool);

    bool deterministicPingDropoutAllowed() const;
    void setDeterministicPingDropoutAllowed(bool);

    void disableNewPlayers();
    void outputStuffedInfo(std::ostream&);
    void outputCurrentStatistics(std::ostream&);

    void messageThrottlingActive(bool);
    bool messageThrottlingActive() const;

    void autoAdjustMaxPacketsPerSecond(size_t numberOfPlayers);

    int maxSentMessagesPerSecond() const;
    ///////////////////////////////
    NetMessage* getMessage();
    bool haveMessages();
    void sendMessage(const NetPriority& priority, const NetMessageBody& body);

private:
    friend std::ostream& operator<<(std::ostream&, NetworkProtocol);
    ///////////////////////////////

    NetNetwork();

    ///////////////////////////////

    bool isValid() const;
    bool isValidNoRecord() const;

    ///////////////////////////////

    NetINetwork* pNetINetwork_;

    ///////////////////////////////

    friend class NetINetwork;
    friend class NetNode;

    ///////////////////////////////
};

std::string_view toString(NetNetwork::NetworkProtocol);

#endif //_NETNETWORK_HPP
