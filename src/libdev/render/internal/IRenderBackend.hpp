#pragma once

#include "render/BackendType.hpp"
#include "render/IRenderSurface.hpp"
#include "render/ShaderSet.hpp"
#include "render/render.hpp"
#include "render/internal/BackendCommands.hpp"
#include "render/internal/PipelineSpec.hpp"
#include "render/internal/RenderPassSpec.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace Ren
{

class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;

    // Which backends are compiled in, regardless of whether the host can
    // present them.
    static std::vector<BackendType> availableBackends();

    // Which of those the given surface can actually present, and the best of
    // them. A null surface means "compiled in", i.e. the unfiltered list.
    static std::vector<BackendType> supportedBackends(IRenderSurface* surface);
    static BackendType resolveAutoBackend(IRenderSurface* surface);

    // Create a backend of the given type. Auto picks the best the surface
    // supports. Returns nullptr if the type is not compiled in or the surface
    // cannot present it.
    static std::unique_ptr<IRenderBackend> create(IRenderSurface* surface, BackendType type = BackendType::Auto);

    // Which type this instance is.
    virtual BackendType backendType() const = 0;

    // Initialize the backend against the surface it was created for. The
    // backend dynamic_casts to the subinterface it needs -- IGLRenderSurface
    // for the GL backends -- so the check belongs to the backend, not the
    // factory.
    virtual bool initialize(IRenderSurface* surface) = 0;

    virtual void shutdown() = 0;

    virtual bool isInitialized() const = 0;

    virtual bool setVSync(bool enabled) = 0;

    // The GLSL dialects this backend can compile, best first. Never empty.
    virtual std::vector<ShaderSet> supportedShaderSets() const = 0;

    // Which set createPipeline() reads its sources from. A change applies to
    // pipelines created after it, so those already built have to be rebuilt for
    // it to be visible. Returns false, and changes nothing, for a set this
    // backend does not support.
    virtual ShaderSet shaderSet() const = 0;
    virtual bool setShaderSet(ShaderSet set) = 0;

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
