#include "RenderVariables.hpp"

#include "LightingMode.hpp"
#include "ShadowQuality.hpp"

#include "system/Variable_p.hpp"

namespace Config
{

namespace Impl
{

template <>
std::string toString(const LightingMode& value)
{
    return std::string(::toString(value));
}

template <>
std::optional<LightingMode> toValue(const std::string& asString)
{
    for (LightingMode mode : AllLightingModes)
    {
        if (asString == ::toString(mode))
            return mode;
    }

    return std::nullopt;
}

template <>
std::string toString(const ShadowQuality& value)
{
    return std::string(::toString(value));
}

template <>
std::optional<ShadowQuality> toValue(const std::string& asString)
{
    for (ShadowQuality quality : AllShadowQualities)
    {
        if (asString == ::toString(quality))
            return quality;
    }

    return std::nullopt;
}

} // namespace Impl

template class Config::Variable<LightingMode>;
template class Config::Variable<ShadowQuality>;

Variable<LightingMode> gfxLightingMode("Options/Graphics Complexity/Lighting Mode", LightingMode::Legacy);
Variable<ShadowQuality> gfxShadowQuality("Options/Graphics Complexity/Shadow Quality", ShadowQuality::Static);

} // namespace Config
