#pragma once

#include <variant>

#include "render/Colour.hpp"
#include "render/PrimitiveTopology.hpp"
#include "render/internal/BackendTypes.hpp"
#include "render/render.hpp"

#include <cstddef>
#include <cstdint>
#include <variant>

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

struct BackendCommandDrawIndexed
{
    PrimitiveTopology topology{};
    BackendIndexType indexType{};
    int count{};
    std::size_t indexBufferOffset{};
};

struct BackendCommandSetBlendState
{
    bool enabled{};
    BackendBlendFactor srcFactor{BackendBlendFactor::One};
    BackendBlendFactor dstFactor{BackendBlendFactor::Zero};
};

struct BackendCommandSetAlphaTest
{
    bool enabled{};
    float reference{};
};

struct BackendCommandSetDepthMask
{
    bool writable{};
};

struct BackendCommandSetDepthTest
{
    bool enabled{};
};

using BackendCommand = std::variant<
    BackendCommandClear,
    BackendCommandSetViewport,
    BackendCommandDraw,
    BackendCommandDrawIndexed,
    BackendCommandSetBlendState,
    BackendCommandSetAlphaTest,
    BackendCommandSetDepthMask,
    BackendCommandSetDepthTest>;

namespace Command
{

inline BackendCommand clear(RenColour colour, std::uint32_t mask)
{
    return BackendCommandClear{ colour.r(), colour.g(), colour.b(), colour.a(), mask };
}

inline BackendCommand setViewport(Size size)
{
    return BackendCommandSetViewport{ 0, 0, size.width, size.height };
}

inline BackendCommand draw(PrimitiveTopology topology, int first, std::size_t count)
{
    return BackendCommandDraw{ topology, first, static_cast<int>(count) };
}

inline BackendCommand drawIndexed(
    PrimitiveTopology topology, BackendIndexType indexType, int count, std::size_t indexBufferOffset = 0)
{
    return BackendCommandDrawIndexed{ topology, indexType, count, indexBufferOffset };
}

inline BackendCommand setBlendStateEnabled(BackendBlendFactor srcFactor, BackendBlendFactor dstFactor)
{
    return BackendCommandSetBlendState{ .enabled = true, .srcFactor = srcFactor, .dstFactor = dstFactor };
}

inline BackendCommand setAlphaTestEnabled(float reference)
{
    return BackendCommandSetAlphaTest{ .enabled = true, .reference = reference };
}

inline BackendCommand setAlphaTestDisabled()
{
    return BackendCommandSetAlphaTest{ .enabled = false };
}

inline BackendCommand setBlendStateDisabled()
{
    return BackendCommandSetBlendState{ .enabled = false };
}

inline BackendCommand setDepthMaskWritable(bool writable)
{
    return BackendCommandSetDepthMask{writable};
}

inline BackendCommand setDepthTest(bool enabled)
{
    return BackendCommandSetDepthTest{enabled};
}

} // namespace Command

} // namespace Ren
