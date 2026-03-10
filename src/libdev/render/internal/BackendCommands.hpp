#pragma once

#include <variant>

#include "render/Colour.hpp"
#include "render/PrimitiveTopology.hpp"
#include "render/Colour.hpp"
#include "render/internal/BackendTypes.hpp"
#include "render/internal/PipelineSpec.hpp"
#include "render/internal/RenderPassSpec.hpp"
#include "render/internal/UniformBlocks.hpp"
#include "render/render.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

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

enum class BackendCullFaceMode
{
    Back,
    Front,
};

struct BackendCommandSetCullFaceMode
{
    BackendCullFaceMode mode{BackendCullFaceMode::Back};
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

struct BackendCommandSetUniform1f
{
    UniformLocationId location{};
    float value{};
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

struct BackendCommandSetUniform1fv
{
    UniformLocationId location{};
    std::vector<float> values{};
};

struct BackendCommandSetUniform3fv
{
    UniformLocationId location{};
    std::vector<float> values{}; // count*3 floats
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

struct BackendCommandBindPipeline
{
    PipelineId pipelineId{};
};

struct BackendCommandBindTexture2D
{
    BackendTextureHandle textureHandle{};
    std::uint32_t unit{};
    TextureFilter minFilter{TextureFilter::Linear};
    TextureFilter magFilter{TextureFilter::Linear};
};

struct BackendCommandBufferData
{
    BufferTarget target{};
    BufferId bufferId{};
    std::vector<std::byte> data{};
    BufferUsage usage{};
};

struct BackendCommandBindBuffer
{
    BufferTarget target{};
    BufferId bufferId{};
};

struct BackendCommandBeginRenderToTexture
{
    FramebufferId framebufferId{};
    BackendTextureHandle targetTexture{};
};

struct BackendCommandEndRenderToTexture
{
};

struct BackendCommandSetLineWidth
{
    float width{};
};

struct BackendCommandSetPointSize
{
    float size{};
};

struct BackendCommandBeginRenderPass
{
    RenderPassId renderPassId{};
    FramebufferId framebufferId{};
    bool overrideClearColor{};
    float clearR{};
    float clearG{};
    float clearB{};
    float clearA{};
};

struct BackendCommandEndRenderPass
{
};

struct BackendCommandBindDefaultFramebuffer
{
};

struct BackendCommandBindFramebuffer
{
    FramebufferId framebufferId{};
};

struct BackendCommandSetGui2DUniforms
{
    Gui2DUniforms uniforms{};
};

struct BackendCommandSetStandardFrameUniforms
{
    StandardFrameUniforms uniforms{};
};

struct BackendCommandSetStandardObjectUniforms
{
    StandardObjectUniforms uniforms{};
};

struct BackendCommandSetBillboardUniforms
{
    BillboardUniforms uniforms{};
};

struct BackendCommandSetShadowDepthUniforms
{
    ShadowDepthUniforms uniforms{};
};

struct BackendCommandSetPostProcessUniforms
{
    PostProcessUniforms uniforms{};
};

using BackendCommand = std::variant<
    BackendCommandClear,
    BackendCommandSetViewport,
    BackendCommandSetMultisample,
    BackendCommandDraw,
    BackendCommandDrawIndexed,
    BackendCommandSetBlendState,
    BackendCommandSetCullFace,
    BackendCommandSetCullFaceMode,
    BackendCommandSetPolygonOffsetFill,
    BackendCommandSetPolygonOffset,
    BackendCommandSetAlphaTest,
    BackendCommandSetDepthMask,
    BackendCommandSetDepthFunc,
    BackendCommandSetDepthTest,
    BackendCommandSetUniform1i,
    BackendCommandSetUniform1f,
    BackendCommandSetUniform1fv,
    BackendCommandSetUniform2f,
    BackendCommandSetUniform3f,
    BackendCommandSetUniform3fv,
    BackendCommandSetUniformMatrix4fv,
    BackendCommandSetVertexAttribPointer,
    BackendCommandSetProgram,
    BackendCommandBindPipeline,
    BackendCommandBindTexture2D,
    BackendCommandBufferData,
    BackendCommandBindBuffer,
    BackendCommandBeginRenderToTexture,
    BackendCommandEndRenderToTexture,
    BackendCommandSetLineWidth,
    BackendCommandSetPointSize,
    BackendCommandBeginRenderPass,
    BackendCommandBindDefaultFramebuffer,
    BackendCommandBindFramebuffer,
    BackendCommandEndRenderPass,
    BackendCommandSetGui2DUniforms,
    BackendCommandSetStandardFrameUniforms,
    BackendCommandSetStandardObjectUniforms,
    BackendCommandSetBillboardUniforms,
    BackendCommandSetShadowDepthUniforms,
    BackendCommandSetPostProcessUniforms>;

namespace Command
{

inline BackendCommand clear(RenColour colour, std::uint32_t mask)
{
    return BackendCommandClear{ colour.r(), colour.g(), colour.b(), colour.a(), mask };
}

inline BackendCommand bindDefaultFramebuffer()
{
    return BackendCommandBindDefaultFramebuffer{};
}

inline BackendCommand bindFramebuffer(FramebufferId id)
{
    return BackendCommandBindFramebuffer{id};
}

inline BackendCommand setViewport(Size size)
{
    return BackendCommandSetViewport{ 0, 0, size.width, size.height };
}

inline BackendCommand setViewport(int x, int y, int width, int height)
{
    return BackendCommandSetViewport{ x, y, width, height };
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

inline BackendCommand setCullFaceMode(BackendCullFaceMode mode)
{
    return BackendCommandSetCullFaceMode{mode};
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

inline BackendCommand setUniform1f(UniformLocationId location, float value)
{
    return BackendCommandSetUniform1f{location, value};
}

inline BackendCommand setUniform2f(UniformLocationId location, float x, float y)
{
    return BackendCommandSetUniform2f{location, x, y};
}

inline BackendCommand setUniform1fv(UniformLocationId location, std::vector<float> values)
{
    return BackendCommandSetUniform1fv{location, std::move(values)};
}

inline BackendCommand setUniform3f(UniformLocationId location, float x, float y, float z)
{
    return BackendCommandSetUniform3f{location, x, y, z};
}

inline BackendCommand setUniform3fv(UniformLocationId location, std::vector<float> values)
{
    return BackendCommandSetUniform3fv{location, std::move(values)};
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

inline BackendCommand bindPipeline(PipelineId pipelineId)
{
    return BackendCommandBindPipeline{ pipelineId };
}

inline BackendCommand bindTexture2D(
    BackendTextureHandle textureHandle,
    std::uint32_t unit,
    TextureFilter minFilter = TextureFilter::Linear,
    TextureFilter magFilter = TextureFilter::Linear)
{
    return BackendCommandBindTexture2D{ textureHandle, unit, minFilter, magFilter };
}

inline BackendCommand bufferData(BufferTarget target, BufferId bufferId, const void* data, std::size_t sizeBytes, BufferUsage usage)
{
    auto bytes = static_cast<const std::byte*>(data);
    return BackendCommandBufferData{ target, bufferId, std::vector<std::byte>(bytes, bytes + sizeBytes), usage };
}

inline BackendCommand bindBuffer(BufferTarget target, BufferId bufferId)
{
    return BackendCommandBindBuffer{ target, bufferId };
}

inline BackendCommand beginRenderToTexture(FramebufferId framebufferId, BackendTextureHandle targetTexture)
{
    return BackendCommandBeginRenderToTexture{ framebufferId, targetTexture };
}

inline BackendCommand endRenderToTexture()
{
    return BackendCommandEndRenderToTexture{};
}

inline BackendCommand setLineWidth(float width)
{
    return BackendCommandSetLineWidth{ width };
}

inline BackendCommand setPointSize(float size)
{
    return BackendCommandSetPointSize{ size };
}

inline BackendCommand beginRenderPass(RenderPassId renderPassId, FramebufferId framebufferId = 0)
{
    return BackendCommandBeginRenderPass{ renderPassId, framebufferId };
}

inline BackendCommand beginRenderPass(RenderPassId renderPassId, RenColour clearColor, FramebufferId framebufferId = 0)
{
    return BackendCommandBeginRenderPass{
        renderPassId, framebufferId, true, clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a() };
}

inline BackendCommand endRenderPass()
{
    return BackendCommandEndRenderPass{};
}

inline BackendCommand setGui2DUniforms(Gui2DUniforms uniforms)
{
    return BackendCommandSetGui2DUniforms{std::move(uniforms)};
}

inline BackendCommand setStandardFrameUniforms(StandardFrameUniforms uniforms)
{
    return BackendCommandSetStandardFrameUniforms{std::move(uniforms)};
}

inline BackendCommand setStandardObjectUniforms(StandardObjectUniforms uniforms)
{
    return BackendCommandSetStandardObjectUniforms{std::move(uniforms)};
}

inline BackendCommand setBillboardUniforms(BillboardUniforms uniforms)
{
    return BackendCommandSetBillboardUniforms{std::move(uniforms)};
}

inline BackendCommand setShadowDepthUniforms(ShadowDepthUniforms uniforms)
{
    return BackendCommandSetShadowDepthUniforms{std::move(uniforms)};
}

inline BackendCommand setPostProcessUniforms(PostProcessUniforms uniforms)
{
    return BackendCommandSetPostProcessUniforms{std::move(uniforms)};
}

} // namespace Command

} // namespace Ren
