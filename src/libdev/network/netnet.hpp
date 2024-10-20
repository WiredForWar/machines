#ifndef _NETNETWORK_HPP
#define _NETNETWORK_HPP

#include "ctl/pvector.hpp"
#include "ctl/map.hpp"

#include "network/netdefs.hpp"
#include "network/message.hpp"
#include "network/NetSessionInfo.hpp"

#include <vector>
#include <memory>

class NetNode;
class NetNodeUid;
class NetProcessUid;
class NetAppSession;
class NetAppSessionUid;
class NetINetwork;
class NetSystemMessageHandler;
class NetComPortAddress;

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
    };

    enum class NetworkProtocol
    {
        UDP,
        IPX,
        TCPIP,
        MODEM,
        SERIAL,
        ZONE,
        OTHER
    };

    using ProtocolSpec = std::pair<const std::string, NetworkProtocol>;
    using ProtocolMap = ctl_map<ProtocolSpec::first_type, ProtocolSpec::second_type, std::less<std::string>>;
    using Sessions = std::vector<NetSessionInfo>;
    using Modems = ctl_vector<std::string>;

    ///////////////////////////////

    ~NetNetwork();

    ///////////////////////////////

    static NetNetwork& instance();
    static NetInterProcessUid nextInterProcessUid();
    static const ProtocolMap& availableProtocols();

    // Normally a protocol may be chosen with initialise set and the default dialogs will be displayed.
    // If you wish to override the default dialogs then set to NOT_INITIALISE, set the extra parameters via the
    // interface below and then finally call initialiseConnection explicitly.
    enum InitialiseConnection
    {
        INITIALISE_CONNECTION,
        DO_NOT_INITIALISE_CONNECTION
    };
    static void chooseProtocol(const std::string&, InitialiseConnection);
    void initialiseConnection();

    static NetNetworkStatus currentStatus();
    //  Use the no record version in assertions
    static NetNetworkStatus currentStatusNoRecord();
    static void resetStatus();

    ///////////////////////////////

    NetAppUid appUid() const;
    NetAppUid appUidNoRecord() const;
    bool hasAppSession(NetAppSessionUid) const;
    void update();
    void refreshSessions();
    void updateSessions();
    void pollMessages();
    const Sessions& sessions() const;
    // If hosting conventional game then use createAppSession
    NetAppSession* createAppSession(const std::string& gameName);
    // If joining a conventional game then use joinAppSession
    NetAppSession* joinAppSession(const std::string& addressStr);
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

    // Ip addresses may be in the form of numerical IP addresses or domain net
    const std::string& IPAddress() const;
    void setIPAddress(const std::string& newAddress);

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

#endif //_NETNETWORK_HPP
