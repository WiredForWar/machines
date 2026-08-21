#pragma once

#include <string_view>

namespace Ren
{

// A dialect of GLSL, and with it the directory holding the sources written in
// it. A backend offers the sets it is able to compile; choosing between them
// decides where sources are read from and nothing else about how a scene is
// drawn, so the same scene rendered from any of them must look the same.
enum class ShaderSet
{
    GLSL120,
};

inline constexpr ShaderSet AllShaderSets[] = {
    ShaderSet::GLSL120,
};

inline std::string_view toString(ShaderSet set)
{
    switch (set)
    {
    case ShaderSet::GLSL120:
        return "GLSL120";
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
    }

    return {};
}

} // namespace Ren
