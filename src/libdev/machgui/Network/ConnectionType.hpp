#pragma once

#include <string_view>

#include "gui/ResolvedUiString.hpp"

enum class ConnectionType
{
    LAN,
    InternetP2P,
};

inline constexpr ConnectionType AllConnectionTypes[] = {
    ConnectionType::LAN,
    ConnectionType::InternetP2P,
};

std::string_view toString(ConnectionType type);
ResolvedUiString toDisplayString(ConnectionType type);
