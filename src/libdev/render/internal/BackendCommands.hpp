#pragma once

#include <variant>

#include "render/Colour.hpp"
#include "render/render.hpp"

namespace Ren
{

struct BackendCommandClear
{
    float r{};
    float g{};
    float b{};
    float a{};
    std::uint32_t mask{};
};

struct BackendCommandSetViewport
{
    int x{};
    int y{};
    int width{};
    int height{};
};

using BackendCommand = std::variant<BackendCommandClear, BackendCommandSetViewport>;

namespace Command
{

inline BackendCommand clear(RenColour colour, std::uint32_t mask)
{
    return BackendCommandClear{colour.r(), colour.g(), colour.b(), colour.a(), mask};
}

inline BackendCommand setViewport(Size size)
{
    return BackendCommandSetViewport{0, 0, size.width, size.height};
}

} // namespace Command

} // namespace Ren
