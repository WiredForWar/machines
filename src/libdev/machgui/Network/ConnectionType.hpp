#pragma once

#include <string_view>

#include "gui/ResolvedUiString.hpp"

enum class ConnectionType
{
    LAN,
};

inline constexpr ConnectionType AllConnectionTypes[] = {
    ConnectionType::LAN,
};

std::string_view toString(ConnectionType type);
ResolvedUiString toDisplayString(ConnectionType type);
