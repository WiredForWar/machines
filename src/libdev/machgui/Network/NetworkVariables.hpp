#pragma once

#include "system/Variable.hpp"

#include <string>

enum class ConnectionType;

namespace Config
{

extern Variable<ConnectionType> netSelectedProtocol;
extern Variable<std::string> netPlayerName;

} // namespace Config
