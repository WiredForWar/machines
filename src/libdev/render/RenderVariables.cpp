#include "RenderVariables.hpp"

#include "BackendType.hpp"
#include "FogMode.hpp"
#include "LightingMode.hpp"
#include "ShaderSet.hpp"
#include "ShadowQuality.hpp"
#include "WindowMode.hpp"

#include "system/Registry.hpp"
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
std::string toString(const Ren::ShaderSet& value)
{
    return std::string(Ren::toString(value));
}

template <>
std::optional<Ren::ShaderSet> toValue(const std::string& asString)
{
    for (Ren::ShaderSet set : Ren::AllShaderSets)
    {
        if (asString == Ren::toString(set))
            return set;
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
std::string toString(const Ren::WindowMode& value)
{
    return std::string(Ren::toString(value));
}

template <>
std::optional<Ren::WindowMode> toValue(const std::string& asString)
{
    for (Ren::WindowMode mode : Ren::AllWindowModes)
    {
        if (asString == Ren::toString(mode))
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
template class Config::Variable<Ren::ShaderSet>;
template class Config::Variable<Ren::WindowMode>;
template class Config::Variable<ShadowQuality>;

Variable<Ren::BackendType> gfxBackendType("Options/Graphics/Backend", {});
Variable<Ren::ShaderSet> gfxShaderSet("Options/Graphics/Shader Set", Ren::ShaderSet::GLSL120);
Variable<Ren::WindowMode> gfxWindowMode("Screen Resolution/Window Mode", Ren::WindowMode::Borderless);

bool windowModeIsUnset()
{
    std::string asString;
    SysRegistry::instance().queryValueNoRecord(std::string(gfxWindowMode.name()), asString);

    return asString.empty();
}

Variable<LightingMode> gfxLightingMode("Options/Graphics Complexity/Lighting Mode", LightingMode::Legacy);
Variable<ShadowQuality> gfxShadowQuality("Options/Graphics Complexity/Shadow Quality", ShadowQuality::Static);
Variable<bool> gfxToneMapping("Options/Graphics Complexity/Tone Mapping", false);
Variable<FogMode> gfxFogMode("Options/Graphics Complexity/Fog Mode", FogMode::Linear);
Variable<bool> gfxModernRendering("Options/Graphics Complexity/Modern Rendering", false);

} // namespace Config
