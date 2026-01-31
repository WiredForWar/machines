#include "machgui/Network/NetworkVariables.hpp"

#include "ConnectionType.hpp"

#include "system/Variable_p.hpp"

namespace Config
{

namespace Impl
{

template <>
std::string toString(const ConnectionType& value)
{
    return std::string(::toString(value));
}

template <>
std::optional<ConnectionType> toValue(const std::string& asString)
{
    for (ConnectionType type : AllConnectionTypes)
    {
        if (asString == ::toString(type))
            return type;
    }

    return std::nullopt;
}

} // namespace Impl

template class Config::Variable<ConnectionType>;

Variable<ConnectionType> netSelectedProtocol("Misc/Connection Type", ConnectionType::LAN);
Variable<std::string> netPlayerName("Misc/Players Name", {});

} // namespace Config
