#pragma once

#include <variant>

#include "render/PrimitiveTopology.hpp"
#include "render/colour.hpp"
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

struct BackendCommandDraw
{
    PrimitiveTopology topology{};
    int first{};
    int count{};
};

using BackendCommand = std::variant<BackendCommandClear, BackendCommandSetViewport, BackendCommandDraw>;

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

inline BackendCommand draw(PrimitiveTopology topology, int first, std::size_t count)
{
    return BackendCommandDraw{topology, first, static_cast<int>(count)};
}

} // namespace Command

} // namespace Ren
