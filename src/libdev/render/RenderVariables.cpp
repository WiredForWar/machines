#include "RenderVariables.hpp"

#include "BackendType.hpp"
#include "FogMode.hpp"
#include "LightingMode.hpp"
#include "ShadowQuality.hpp"

#include "system/Variable_p.hpp"

namespace Config
{

namespace Impl
{

template <>
std::string toString(const Ren::BackendType& value)
{
    return std::string(Ren::toString(value));
}

template <>
std::optional<Ren::BackendType> toValue(const std::string& asString)
{
    for (Ren::BackendType type : Ren::AllBackendTypes)
    {
        if (asString == Ren::toString(type))
            return type;
    }

    return std::nullopt;
}

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
std::string toString(const FogMode& value)
{
    return std::string(::toString(value));
}

template <>
std::optional<FogMode> toValue(const std::string& asString)
{
    for (FogMode mode : AllFogModes)
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

template class Config::Variable<Ren::BackendType>;
template class Config::Variable<FogMode>;
template class Config::Variable<LightingMode>;
template class Config::Variable<ShadowQuality>;

Variable<Ren::BackendType> gfxBackendType("Options/Graphics/Backend", {});
Variable<LightingMode> gfxLightingMode("Options/Graphics Complexity/Lighting Mode", LightingMode::Legacy);
Variable<ShadowQuality> gfxShadowQuality("Options/Graphics Complexity/Shadow Quality", ShadowQuality::Static);
Variable<bool> gfxToneMapping("Options/Graphics Complexity/Tone Mapping", false);
Variable<FogMode> gfxFogMode("Options/Graphics Complexity/Fog Mode", FogMode::Linear);

} // namespace Config
