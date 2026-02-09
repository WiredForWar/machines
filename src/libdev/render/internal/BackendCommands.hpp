#pragma once

#include <variant>

#include "render/Colour.hpp"
#include "render/PrimitiveTopology.hpp"
#include "render/internal/BackendTypes.hpp"
#include "render/render.hpp"

#include <array>
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

struct BackendCommandSetMultisample
{
    bool enabled{};
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

struct BackendCommandSetCullFace
{
    bool enabled{};
};

struct BackendCommandSetPolygonOffsetFill
{
    bool enabled{};
};

struct BackendCommandSetPolygonOffset
{
    float factor{};
    float units{};
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

struct BackendCommandSetDepthFunc
{
    BackendDepthFunc function{};
};

struct BackendCommandSetDepthTest
{
    bool enabled{};
};

struct BackendCommandSetUniform1i
{
    UniformLocationId location{};
    int value{};
};

struct BackendCommandSetUniform2f
{
    UniformLocationId location{};
    float x{};
    float y{};
};

struct BackendCommandSetUniform3f
{
    UniformLocationId location{};
    float x{};
    float y{};
    float z{};
};

struct BackendCommandSetUniformMatrix4fv
{
    UniformLocationId location{};
    std::array<float, 16> values{};
    bool transpose{};
};

struct BackendCommandSetVertexAttribPointer
{
    bool enabled{};
    AttributeLocationId index{};
    int size{};
    BackendVertexAttribType type{BackendVertexAttribType::Float};
    bool normalized{};
    std::size_t stride{};
    std::size_t offset{};
};

struct BackendCommandSetProgram
{
    ProgramId programId{};
};

using BackendCommand = std::variant<
    BackendCommandClear,
    BackendCommandSetViewport,
    BackendCommandSetMultisample,
    BackendCommandDraw,
    BackendCommandDrawIndexed,
    BackendCommandSetBlendState,
    BackendCommandSetCullFace,
    BackendCommandSetPolygonOffsetFill,
    BackendCommandSetPolygonOffset,
    BackendCommandSetAlphaTest,
    BackendCommandSetDepthMask,
    BackendCommandSetDepthFunc,
    BackendCommandSetDepthTest,
    BackendCommandSetUniform1i,
    BackendCommandSetUniform2f,
    BackendCommandSetUniform3f,
    BackendCommandSetUniformMatrix4fv,
    BackendCommandSetVertexAttribPointer,
    BackendCommandSetProgram>;

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

inline BackendCommand setMultisample(bool enabled)
{
    return BackendCommandSetMultisample{enabled};
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

inline BackendCommand setCullFace(bool enabled)
{
    return BackendCommandSetCullFace{enabled};
}

inline BackendCommand setPolygonOffsetFill(bool enabled)
{
    return BackendCommandSetPolygonOffsetFill{enabled};
}

inline BackendCommand setPolygonOffset(float factor, float units)
{
    return BackendCommandSetPolygonOffset{factor, units};
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

inline BackendCommand setDepthFunc(BackendDepthFunc function)
{
    return BackendCommandSetDepthFunc{function};
}

inline BackendCommand setDepthTest(bool enabled)
{
    return BackendCommandSetDepthTest{enabled};
}

inline BackendCommand setUniform1i(UniformLocationId location, int value)
{
    return BackendCommandSetUniform1i{location, value};
}

inline BackendCommand setUniform2f(UniformLocationId location, float x, float y)
{
    return BackendCommandSetUniform2f{location, x, y};
}

inline BackendCommand setUniform3f(UniformLocationId location, float x, float y, float z)
{
    return BackendCommandSetUniform3f{location, x, y, z};
}

inline BackendCommand setUniformMatrix4fv(UniformLocationId location, const std::array<float, 16>& values, bool transpose)
{
    return BackendCommandSetUniformMatrix4fv{location, values, transpose};
}

inline BackendCommand enableVertexAttribPointer(
    AttributeLocationId index,
    int size,
    BackendVertexAttribType type,
    bool normalized,
    std::size_t stride,
    std::size_t offset)
{
    return BackendCommandSetVertexAttribPointer{ .enabled = true,
                                                 .index = index,
                                                 .size = size,
                                                 .type = type,
                                                 .normalized = normalized,
                                                 .stride = stride,
                                                 .offset = offset };
}

inline BackendCommand disableVertexAttribPointer(AttributeLocationId index)
{
    return BackendCommandSetVertexAttribPointer{ .enabled = false, .index = index };
}

inline BackendCommand setProgram(ProgramId programId)
{
    return BackendCommandSetProgram{ programId };
}

} // namespace Command

} // namespace Ren
