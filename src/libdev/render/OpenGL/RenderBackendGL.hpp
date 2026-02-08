#pragma once

#include "render/internal/IRenderBackend.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <SDL.h>

#include <GL/glew.h>

namespace Ren
{

namespace OpenGL
{

class RenderBackendGL final : public IRenderBackend
{
public:
    RenderBackendGL();

    bool initialize(SDL_Window* window) override;
    void shutdown() override;

    bool isInitialized() const override;

    bool setVSync(bool enabled) override;

    ProgramId createProgramFromFiles(
        std::string_view vertexShaderPath,
        std::string_view fragmentShaderPath,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName) override;
    void releaseProgram(ProgramId id) override;

    void useProgram(ProgramId id) override;

    int uniformLocation(ProgramId id, std::string_view name) const override;
    int attribLocation(ProgramId id, std::string_view name) const override;

    BufferId createBuffer() override;
    void releaseBuffer(BufferId id) override;

    void bindBuffer(BufferTarget target, BufferId id) override;
    void bufferData(
        BufferTarget target, BufferId id, std::size_t sizeBytes, const void* data, BufferUsage usage) override;

    FramebufferId createFramebuffer() override;
    void releaseFramebuffer(FramebufferId id) override;

    void bindFramebuffer(FramebufferId id) override;
    void framebufferTexture2D(FramebufferAttachment attachment, TexId texture) override;

    bool beginRenderToTexture(FramebufferId framebuffer, TexId targetTexture) override;
    void endRenderToTexture() override;

    void pushFramebuffer() override;
    void popFramebuffer() override;
    void bindTexture2D(TexId id, std::uint32_t unit) override;

    BackendCommandBufferHandle createCommandBuffer() override;
    void destroyCommandBuffer(BackendCommandBufferHandle handle) override;
    void beginCommandBuffer(BackendCommandBufferHandle handle) override;
    void recordCommand(BackendCommandBufferHandle handle, BackendCommand&& command) override;
    void endCommandBuffer(BackendCommandBufferHandle handle) override;
    void submitCommandBuffer(BackendCommandBufferHandle handle) override;

    BackendTextureHandle createTexture2D() override;
    void destroyTexture2D(BackendTextureHandle handle) override;
    void textureStorage2D(BackendTextureHandle handle, int width, int height, TextureFormat format) override;
    void textureSubImage2D(
        BackendTextureHandle handle, int x, int y, int width, int height, TextureFormat format, const void* pixels)
        override;
    void textureSetMinMagFilter(BackendTextureHandle handle, TextureFilter minFilter, TextureFilter magFilter) override;
    void textureSetWrap(BackendTextureHandle handle, TextureWrap wrapS, TextureWrap wrapT) override;
    void textureGenerateMipmap(BackendTextureHandle handle) override;

private:
    static std::string readTextFile(const std::string& path);

    static GLuint createProgramFromSources(
        const std::string& vertexShaderCode,
        const std::string& fragmentShaderCode,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName);

    GLuint programHandle(ProgramId id) const;

    GLuint bufferHandle(BufferId id) const;

    GLuint framebufferHandle(FramebufferId id) const;

    struct CommandBuffer
    {
        bool alive{};
        bool recording{};
        std::vector<BackendCommand> commands{};
    };
    CommandBuffer* commandBufferFromHandle(BackendCommandBufferHandle handle);
    const CommandBuffer* commandBufferFromHandle(BackendCommandBufferHandle handle) const;
    void executeCommand(const BackendCommand& command);
    void executeCommand(const BackendCommandClear& command);
    void executeCommand(const BackendCommandSetViewport& command);
    void executeCommand(const BackendCommandDraw& command);
    void executeCommand(const BackendCommandDrawIndexed& command);
    void executeCommand(const BackendCommandSetBlendState& command);
    void executeCommand(const BackendCommandSetCullFace& command);
    void executeCommand(const BackendCommandSetAlphaTest& command);
    void executeCommand(const BackendCommandSetDepthMask& command);
    void executeCommand(const BackendCommandSetDepthTest& command);

    void flushPendingDeletes();
    std::size_t activeCommandBufferCount() const;

    std::vector<GLuint> programs_{};
    std::vector<GLuint> buffers_{};
    std::vector<GLuint> framebuffers_{};
    std::vector<GLuint> framebufferStack_{};
    std::vector<CommandBuffer> commandBuffers_{};

    GLuint fallbackTexture2D_{};
    std::vector<GLuint> pendingTextureDeletes_{};
    bool initialized_{};

    SDL_Window* window_{};
    SDL_GLContext glContext_{};
};

} // namespace OpenGL

} // namespace Ren
