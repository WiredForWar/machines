#pragma once

#include "render/BackendType.hpp"
#include "render/render.hpp"
#include "render/internal/BackendCommands.hpp"
#include "render/internal/PipelineSpec.hpp"
#include "render/internal/RenderPassSpec.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

struct SDL_Window;

namespace Ren
{

class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;

    // Query which backends are compiled in and available on this system.
    static std::vector<BackendType> availableBackends();
    static BackendType resolveAutoBackend();

    // Create a backend of the given type. Auto picks the best available.
    static std::unique_ptr<IRenderBackend> create(BackendType type = BackendType::Auto);

    // Which type this instance is.
    virtual BackendType backendType() const = 0;

    virtual bool initialize(SDL_Window* window) = 0;
    virtual void shutdown() = 0;

    virtual bool isInitialized() const = 0;

    virtual bool setVSync(bool enabled) = 0;

    virtual PipelineId createPipeline(const PipelineDesc& desc) = 0;
    virtual void releasePipeline(PipelineId id) = 0;
    virtual UniformLocationId pipelineUniformLocation(PipelineId id, std::string_view name) const = 0;
    virtual AttributeLocationId pipelineAttribLocation(PipelineId id, std::string_view name) const = 0;

    virtual RenderPassId createRenderPass(const RenderPassDesc& desc) = 0;
    virtual void releaseRenderPass(RenderPassId id) = 0;

    virtual BufferId createBuffer() = 0;
    virtual void releaseBuffer(BufferId id) = 0;

    virtual void bindBuffer(BufferTarget target, BufferId id) = 0;
    virtual void bufferData(
        BufferTarget target, BufferId id, std::size_t sizeBytes, const void* data, BufferUsage usage)
        = 0;

    virtual FramebufferId createFramebuffer() = 0;
    virtual void releaseFramebuffer(FramebufferId id) = 0;

    virtual void bindFramebuffer(FramebufferId id) = 0;
    virtual void framebufferAttachColorTexture(FramebufferId fbo, BackendTextureHandle colorTexture) = 0;
    virtual void framebufferAttachDepthTexture(FramebufferId fbo, BackendTextureHandle depthTexture) = 0;
    virtual void framebufferAttachDepthRenderbuffer(FramebufferId fbo, int width, int height) = 0;
    virtual bool isFramebufferComplete(FramebufferId fbo) = 0;

    virtual void endRenderToTexture() = 0;

    virtual void pushFramebuffer() = 0;
    virtual void popFramebuffer() = 0;

    virtual BackendCommandBufferHandle createCommandBuffer() = 0;
    virtual void destroyCommandBuffer(BackendCommandBufferHandle handle) = 0;
    virtual void beginCommandBuffer(BackendCommandBufferHandle handle) = 0;
    virtual void recordCommand(BackendCommandBufferHandle handle, BackendCommand&& command) = 0;
    virtual void endCommandBuffer(BackendCommandBufferHandle handle) = 0;
    virtual void submitCommandBuffer(BackendCommandBufferHandle handle) = 0;

    // Query the current viewport dimensions.
    virtual Viewport getViewport() const = 0;

    // Set the viewport and clear the colour buffer to black.
    // Used by the display layer after a mode change.
    virtual void clearDisplay(int width, int height) = 0;

    // Synchronous readback — not a recorded command.
    virtual void readPixelsFloat(int x, int y, int width, int height, float* rgba) = 0;
    virtual void readPixelsUByte(int x, int y, int width, int height, unsigned char* rgba) = 0;

    virtual BackendTextureHandle createTexture2D() = 0;
    virtual void destroyTexture2D(BackendTextureHandle handle) = 0;
    virtual void textureStorage2D(BackendTextureHandle handle, int width, int height, TextureFormat format) = 0;
    virtual void textureSubImage2D(
        BackendTextureHandle handle, int x, int y, int width, int height, TextureFormat format, const void* pixels)
        = 0;
    virtual void textureSetMinMagFilter(BackendTextureHandle handle, TextureFilter minFilter, TextureFilter magFilter)
        = 0;
    virtual void textureSetWrap(BackendTextureHandle handle, TextureWrap wrapS, TextureWrap wrapT) = 0;
    virtual void textureGenerateMipmap(BackendTextureHandle handle) = 0;
};

} // namespace Ren
