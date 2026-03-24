#ifndef _NETINETWORK_HPP
#define _NETINETWORK_HPP

#define IDIRECTPLAY2_OR_GREATER

#include "ctl/Vector.hpp"
#include "ctl/List.hpp"

#include "device/Timer.hpp"

#include "network/NetDefs.hpp"
#include "network/Network.hpp"
#include "network/StunClient.hpp"

#include <enet/enet.h>

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class RendezvousWorker;

class NetNode;
class NetNodeUid;
class NetProcessUid;
class NetAppSession;
class NetAppSessionUid;
class NetSystemMessageHandler;

struct NetMessageShortInfo
{
    double time_;
    int length_;
};
bool operator<(const NetMessageShortInfo& a, const NetMessageShortInfo& b);
bool operator==(const NetMessageShortInfo& a, const NetMessageShortInfo& b);

class NetINetwork
{
public:
    ///////////////////////////////

    enum class Update
    {
        No,
        Yes,
    };

    enum class JoinState
    {
        Idle,
        WaitingPunch,
        Connecting,
        WaitingInit,
        Done,
    };

    enum ObjectActionType
    {
        RETURN,
        RESET
    };

    ///////////////////////////////

    NetINetwork();
    ~NetINetwork();

    ///////////////////////////////

    const NetNetwork::ProtocolList& availableProtocols(Update update = Update::Yes);
    void chooseProtocol(NetNetwork::NetworkProtocol, NetNetwork::InitialiseConnection);
    void initialiseConnection();

    static const NetNetwork::NetNetworkStatus& currentStatus();
    static void currentStatus(NetNetwork::NetNetworkStatus);
    static void resetStatus();

    ///////////////////////////////

    NetAppSession* createAppSession(const std::string& gameName);
    void beginJoinAppSession(const std::string& addressStr);
    void updateJoin();
    void abortJoin();
    JoinState joinState() const;
    NetAppSession* connectAppSession();
    void resetAppSession();

    bool hasAppSession(const NetAppSessionUid&) const;
    //  Use the no record version in assertions
    bool hasAppSessionNoRecord(const NetAppSessionUid&) const;
    NetAppSession& session();
    const std::optional<StunClient::Result>& publicEndpoint() const;
    void update();
    void clearSessions();
    void updateSessions();
    void pollMessages();
    const NetNetwork::Sessions& sessions() const;

    bool isValid() const;
    bool hasActiveSession() const;

    void systemMessageHandler(NetSystemMessageHandler*);
    bool hasSystemMessageHandler() const;
    NetSystemMessageHandler& systemMessageHandler();

    // mainly used with lobby technology - but the fields should be valid for normal create/join mechanism as well
    bool isLobbiedGame() const;
    bool isLogicalHost() const;
    const std::string& localPlayerName() const;
    void setLocalPlayerName(const std::string&);

    ///////////////////////////////
    bool imStuffed() const;
    bool imStuffedNoRecord() const;

    bool deterministicPingDropoutAllowed() const;
    void setDeterministicPingDropoutAllowed(bool);

private:
    ///////////////////////////////

    using Peers = std::vector<ENetPeer*>;
    using NetMessageBuffer = ctl_pvector<NetMessage>;

    ///////////////////////////////

    void clearProtocols();
    void setAppUid();

    ///////////////////////////////

    bool hasAppSession(const NetAppSessionName&) const;
    //  Use the no record version in assertions
    bool hasAppSessionNoRecord(const NetAppSessionName&) const;
    void enterAppSession(NetNode*, NetAppSessionUid);
    void doAbort(const std::string& reasonCode);
    void initHost(bool asServer = false);
    void resetHost();
    NetMessage* getMessage();
    bool haveMessages();
    void sendMessage(const NetPriority& priority, const NetMessageRecipients& /*to*/, const NetMessageBody& body);

    NetNetwork::NetworkProtocol currentProtocol() const;

    // Ip addresses may be in the form of numerical IP addresses or domain net
    const std::string& IPAddress() const;
    void setIPAddress(const std::string& newIPAddress);
    const std::string& selectedRendezvousSessionId() const;
    void setSelectedRendezvousSessionId(const std::string& sessionId);

    bool isValidNoRecord() const;
    bool hasLocalNodeNoRecord(const NetNode*) const;
    static NetNetwork::NetNetworkStatus& currentStatusNoRecord();

    NetProcessUid& processUidMaster() const;
    void addSentMessage(int length);
    void computeSentMessageStuffedNess();
    double deterministicTimeoutPeriod() const;
    size_t maxSentMessagesPerSecond() const;
    void autoAdjustMaxSentMessagesPerSecond(size_t numberOfPlayers);

    void disableNewPlayers();
    bool pingAllAllowed() const;

    void sendInitPacket(ENetPeer* pPeer);

    void initServersDiscoverySocket();
    void deinitServersDiscoverySocket();
    void sendLocalServersDiscoveryBroadcast();
    void acceptLocalServersReplies();
    bool registerServer();
    bool initLocalServerDiscovery();
    void deinitLocalServerDiscovery();
    void replyToServerDiscoveryRequests();

    void updateRendezvousSessions();
    void pollRendezvousResults();
    RendezvousWorker* ensureWorker();
    void destroyWorker();
    void beginStunQuery();
    void beginRegisterSession();
    void beginHeartbeat();
    void beginListPunchRequests();
    void clearRendezvousRegistration();
    void sendUdpPunch(const std::string& ipAddress, uint16_t port) const;

    void determineStandardSendFlags();

    void messageThrottlingActive(bool);
    bool messageThrottlingActive() const;

    bool isStunProtocolSelected() const;
    void clearPublicEndpoint();

    ///////////////////////////////

    NetMessageBuffer messageBuffer_;
    NetNetwork::Sessions sessions_;
    double lastSessionsUpdate_{};
    Peers peers_;
    NetAppSession* pLocalSession_{};
    NetSystemMessageHandler* pSystemMessageHandler_{};

    bool lobbyAware_{};

    // lobby description fields
    bool isLobbiedGame_{};
    bool isLogicalHost_{};
    std::string localPlayerName_;
    std::string gameName_;

    std::string IPAddress_;
    std::string selectedRendezvousSessionId_{};

    NetNetwork::NetworkProtocol currentProtocol_;

    bool imStuffed_{};

    //  NetPingHelper                   pingHelper_;

    ENetSocket lanDiscoveryServerSocket_{ENET_SOCKET_NULL};
    ENetSocket lanDiscoveryClientSocket_{ENET_SOCKET_NULL};
    ENetAddress address_{};
    ENetHost* pHost_{};
    uint32_t remoteVersion_{};
    JoinState joinState_{JoinState::Idle};
    ENetPeer* joinPeer_{};
    double joinStartTime_{};
    ENetAddress joinAddress_{};

    bool deterministicPingDropoutAllowed_{};

    std::optional<StunClient::Result> publicEndpoint_{};
    std::string stunServerHost_{};
    uint16_t stunServerPort_{};

    bool rendezvousConfigValid_{};
    std::string rendezvousHost_{};
    uint16_t rendezvousPort_{};
    bool rendezvousUseHttps_{};
    std::string rendezvousApiPathPrefix_{};
    std::chrono::milliseconds rendezvousNetworkTimeout_{};
    std::string rendezvousSessionId_{};
    std::chrono::seconds rendezvousHeartbeatInterval_{};
    std::chrono::steady_clock::time_point rendezvousNextHeartbeat_{};
    std::chrono::steady_clock::time_point rendezvousNextPunchPoll_{};

    std::unique_ptr<RendezvousWorker> worker_{};

    // Pending async state for createAppSession / beginJoinAppSession
    bool stunQueryPending_{};
    bool registerSessionPending_{};
    bool heartbeatPending_{};
    bool listSessionsPending_{};
    bool listPunchRequestsPending_{};
    bool registerPunchPending_{};

    ///////////////////////////////

    friend class NetNode;
    friend class NetNetwork;
    friend class NetIAppSession;
    friend class NetSystemMessageHandler;
    ///////////////////////////////
    DevTimer timer_;

    using SentMessages = ctl_list<NetMessageShortInfo*>;
    SentMessages sentMessages_;

    int maxBytesPerSecond_;
    bool pingAllAllowed_;
    int maxSentMessagesPerSecond_;
    int originalMaxSentMessagesPerSecond_;

    bool messageThrottlingActive_{};
    NetNetwork::ProtocolList availableProtocols_;
};

#endif //_NetINetwork_HPP
