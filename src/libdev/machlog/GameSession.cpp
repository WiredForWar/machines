#include "machlog/GameSession.hpp"

#include "machlog/Messaging/Network.hpp"

#include "spdlog/spdlog.h"

// static
MachLogGameSession& MachLogGameSession::instance()
{
    static MachLogGameSession instance_;
    return instance_;
}

MachLogGameSession::MachLogGameSession()
{
}

MachLogGameSession::~MachLogGameSession()
{
}

void MachLogGameSession::begin()
{
    spdlog::debug("MachLogGameSession::begin()");

    isActive_ = true;
}

void MachLogGameSession::end()
{
    spdlog::debug("MachLogGameSession::end()");

    isActive_ = false;
    MachLogNetwork::instance().terminateAndReset();
}

bool MachLogGameSession::isActive() const
{
    return isActive_;
}
