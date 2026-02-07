#pragma once

#include <variant>

#include "render/render.hpp"

namespace Ren
{

struct BackendCommandSetViewport
{
    int x{};
    int y{};
    int width{};
    int height{};
};

using BackendCommand = std::variant<BackendCommandSetViewport>;

namespace Command
{

inline BackendCommand setViewport(Size size)
{
    return BackendCommandSetViewport{0, 0, size.width, size.height};
}

} // namespace Command

} // namespace Ren
