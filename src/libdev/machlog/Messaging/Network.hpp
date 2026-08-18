/*
 * N E T W O R K . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogNetwork

    MachLog network messaging controlling class
*/

#ifndef _MACHLOG_NETWORK_HPP
#define _MACHLOG_NETWORK_HPP

#include "base/base.hpp"
#include "machphys/machphys.hpp"
#include "network/Network.hpp"

#include <optional>

class MachLogNetworkDataImpl;

class MachLogNetwork
// Canonical form revoked
{
public:
    enum Remote
    {
        LOCAL_PROCESS,
        REMOTE_PROCESS
    };
    enum InitialiseConnection
    {
        INITIALISE_CONNECTION,
        DO_NOT_INITIALISE_CONNECTION
    };
    //  Singleton class
    static MachLogNetwork& instance();
    ~MachLogNetwork();

    void terminateAndReset();

    MachPhys::Race localRace() const;
    void localRace(MachPhys::Race);

    bool isNetworkGame() const;
    bool isNodeLogicalHost() const;
    void isNodeLogicalHost(bool);
    Remote remoteStatus(MachPhys::Race) const;
    void remoteStatus(MachPhys::Race, Remote);
    void update();
    int expectedPlayers() const;
    bool simUpdateNeeded(MachPhys::Race) const;
    void simUpdateNeeded(MachPhys::Race, bool);
    bool simUpdateReceived(MachPhys::Race) const;
    void simUpdateReceived(MachPhys::Race, bool);

    void ready(MachPhys::Race, bool newValue);
    bool ready(MachPhys::Race) const;

    std::optional<NetNetwork::NetworkProtocol> desiredProtocol() const;
    bool setDesiredProtocol(NetNetwork::NetworkProtocol);
    void initialiseConnection();

    bool hostSession(const std::string& gameName, const std::string& playerName);
    void beginJoinSession(const std::string& address, const std::string& playerName);
    void resetSession();
    bool launchFromLobbyInfo();

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogNetwork& t);

private:
    MachLogNetwork(const MachLogNetwork&);
    MachLogNetwork& operator=(const MachLogNetwork&);
    bool operator==(const MachLogNetwork&);

    MachLogNetwork();

    // Sets whether a network game is running. The sink that transmits to the other hosts
    // subscribes to the broker for exactly as long as there is a network game to transmit
    // to, so this is also where it attaches and detaches.
    void setNetworkGame(bool);

    MachLogNetworkDataImpl* pImpl_;
};

#endif

/* End NETWORK.HPP **************************************************/
