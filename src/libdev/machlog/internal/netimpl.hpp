/*
 * N E T I M P L . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogNetworkDataImpl

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_NETIMPL_HPP
#define _MACHLOG_NETIMPL_HPP

#include "base/base.hpp"
#include "machlog/network.hpp"

class MachLogNetworkDataImpl
// Canonical form revoked
{
public:
    ~MachLogNetworkDataImpl();

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachLogNetworkDataImpl& t);
    friend class MachLogNetwork;

    MachLogNetworkDataImpl();

    MachLogNetworkDataImpl(const MachLogNetworkDataImpl&);
    MachLogNetworkDataImpl& operator=(const MachLogNetworkDataImpl&);

    bool isNetworkGame_;
    bool isNodeLogicalHost_;
    MachLogNetwork::Remote remote_[MachPhys::N_RACES];
    MachPhys::Race localRace_;
    std::string desiredProtocol_;
    std::string sessionId_;
    NetNode* pNode_;
    MachLogMessageBroker* pBroker_;
    bool readyStatus_[MachPhys::N_RACES];
    bool syncSimCycles_;
    bool simUpdateReceived_[MachPhys::N_RACES];
    bool simUpdateNeeded_[MachPhys::N_RACES];
    int expectedPlayers_;
    bool processedStartup_[MachPhys::N_RACES];
    bool lockedForStartup_;
    bool protocolChosen_;
};

#define CB_MachLogNetwork_DEPIMPL()                                                                                    \
    CB_DEPIMPL_AUTO(isNetworkGame_);                                                                                   \
    CB_DEPIMPL_AUTO(isNodeLogicalHost_);                                                                               \
    CB_DEPIMPL_AUTO(remote_);                                                                                          \
    CB_DEPIMPL_AUTO(localRace_);                                                                                       \
    CB_DEPIMPL_AUTO(desiredProtocol_);                                                                                 \
    CB_DEPIMPL_AUTO(pNode_);                                                                                           \
    CB_DEPIMPL_AUTO(pBroker_);                                                                                         \
    CB_DEPIMPL_AUTO(readyStatus_);                                                                                     \
    CB_DEPIMPL_AUTO(syncSimCycles_);                                                                                   \
    CB_DEPIMPL_AUTO(simUpdateReceived_);                                                                               \
    CB_DEPIMPL_AUTO(simUpdateNeeded_);                                                                                 \
    CB_DEPIMPL_AUTO(expectedPlayers_);                                                                                 \
    CB_DEPIMPL_AUTO(processedStartup_);                                                                                \
    CB_DEPIMPL_AUTO(lockedForStartup_);                                                                                \
    CB_DEPIMPL_AUTO(protocolChosen_);

#endif

/* End NETIMPL.HPP **************************************************/
