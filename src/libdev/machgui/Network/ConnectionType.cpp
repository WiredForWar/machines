#include "ConnectionType.hpp"

#include "machgui/internal/strings.hpp"

std::string_view toString(ConnectionType type)
{
    switch (type)
    {
    case ConnectionType::LAN:
        return "LAN";
    }

    return {};
}

ResolvedUiString toDisplayString(ConnectionType type)
{
    switch (type)
    {
    case ConnectionType::LAN:
        return IDS_CONNECTION_TYPE_LAN_GAME;
    }

    return {};
}
