#pragma once

#include <string_view>

namespace Ren
{

// A dialect of GLSL, and with it the directory holding the sources written in
// it. A backend offers the sets it is able to compile; choosing between them
// decides where sources are read from and nothing else about how a scene is
// drawn, so the same scene rendered from either set must look the same.
enum class ShaderSet
{
    GLSL120,
    GLSL310ES,
};

inline constexpr ShaderSet AllShaderSets[] = {
    ShaderSet::GLSL120,
    ShaderSet::GLSL310ES,
};

inline std::string_view toString(ShaderSet set)
{
    switch (set)
    {
    case ShaderSet::GLSL120:
        return "GLSL120";
    case ShaderSet::GLSL310ES:
        return "GLSL310ES";
    }

    return {};
}

// Where this set's sources live, relative to the data root.
inline std::string_view shaderDirectory(ShaderSet set)
{
    switch (set)
    {
    case ShaderSet::GLSL120:
        return "data/shaders/120/";
    case ShaderSet::GLSL310ES:
        return "data/shaders/310es/";
    }

    return {};
}

} // namespace Ren
