#pragma once

#include "render/render.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

enum class RenBufferTarget
{
    Array,
    ElementArray,
};

enum class RenBufferUsage
{
    StreamDraw,
};

enum class RenFramebufferAttachment
{
    Color0,
};

class RenIRenderBackend
{
public:
    virtual ~RenIRenderBackend() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual bool isInitialized() const = 0;

    virtual Ren::ProgramId createProgramFromFiles(
        std::string_view vertexShaderPath,
        std::string_view fragmentShaderPath,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName)
        = 0;
    virtual void releaseProgram(Ren::ProgramId id) = 0;

    virtual void useProgram(Ren::ProgramId id) = 0;

    virtual int uniformLocation(Ren::ProgramId id, std::string_view name) const = 0;
    virtual int attribLocation(Ren::ProgramId id, std::string_view name) const = 0;

    virtual Ren::BufferId createBuffer() = 0;
    virtual void releaseBuffer(Ren::BufferId id) = 0;

    virtual void bindBuffer(RenBufferTarget target, Ren::BufferId id) = 0;
    virtual void bufferData(
        RenBufferTarget target, Ren::BufferId id, std::size_t sizeBytes, const void* data, RenBufferUsage usage)
        = 0;

    virtual Ren::FramebufferId createFramebuffer() = 0;
    virtual void releaseFramebuffer(Ren::FramebufferId id) = 0;

    virtual void bindFramebuffer(Ren::FramebufferId id) = 0;
    virtual void framebufferTexture2D(RenFramebufferAttachment attachment, std::uint32_t textureHandle) = 0;

    virtual void pushFramebuffer() = 0;
    virtual void popFramebuffer() = 0;
};
