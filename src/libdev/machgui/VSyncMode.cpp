#include "VSyncMode.hpp"

#include "system/Variable_p.hpp"

namespace Config
{

namespace Impl
{

template <>
std::string toString(const MachGui::VSyncMode& value)
{
    return MachGui::toString(value);
}

template <>
std::optional<MachGui::VSyncMode> toValue(const std::string& asString)
{
    using VSyncMode = MachGui::VSyncMode;
    for (VSyncMode layout : {
             VSyncMode::Auto,
             VSyncMode::Disabled,
             VSyncMode::Enabled,
         })
    {
        if (asString == toString(layout))
            return layout;
    }

    return std::nullopt;
}

} // namespace Impl

template class Config::Variable<MachGui::VSyncMode>;

Variable<MachGui::VSyncMode> gfxVSyncMode("Screen Resolution/VSync", {});

} // namespace Config

namespace MachGui
{

std::string toString(const VSyncMode& value)
{
    switch (value)
    {
    case MachGui::VSyncMode::Auto:
        return "Auto";
    case MachGui::VSyncMode::Disabled:
        return "Disabled";
    case MachGui::VSyncMode::Enabled:
        return "Enabled";
    }

    return std::to_string(static_cast<int32_t>(value));
}

} // namespace MachGui
