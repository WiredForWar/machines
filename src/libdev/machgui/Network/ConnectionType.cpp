#include "ConnectionType.hpp"

#include "machgui/internal/strings.hpp"

std::string_view toString(ConnectionType type)
{
    switch (type)
    {
    case ConnectionType::LAN:
        return "LAN";
    case ConnectionType::InternetP2P:
        return "InternetP2P";
    }

    return {};
}

ResolvedUiString toDisplayString(ConnectionType type)
{
    switch (type)
    {
    case ConnectionType::LAN:
        return IDS_CONNECTION_TYPE_LAN_GAME;
    case ConnectionType::InternetP2P:
        return IDS_CONNECTION_TYPE_INTERNET_P2P;
    }

    return {};
}
