#pragma once

#include <string_view>

namespace Ren
{

enum class BackendType
{
    Auto,
    GL21,
};

inline constexpr BackendType AllBackendTypes[] = {
    BackendType::Auto,
    BackendType::GL21,
};

inline std::string_view toString(BackendType type)
{
    switch(type)
    {
    case BackendType::Auto:
        return "Auto";
    case BackendType::GL21:
        return "OpenGL 2.1";
    }

    return {};
}

} // namespace Ren
