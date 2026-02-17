#pragma once

#include "render/internal/IRenderBackend.hpp"

#include <array>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

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

    PipelineId createPipeline(const PipelineDesc& desc) override;
    void releasePipeline(PipelineId id) override;
    UniformLocationId pipelineUniformLocation(PipelineId id, std::string_view name) const override;
    AttributeLocationId pipelineAttribLocation(PipelineId id, std::string_view name) const override;

    RenderPassId createRenderPass(const RenderPassDesc& desc) override;
    void releaseRenderPass(RenderPassId id) override;

    BufferId createBuffer() override;
    void releaseBuffer(BufferId id) override;

    void bindBuffer(BufferTarget target, BufferId id) override;
    void bufferData(
        BufferTarget target, BufferId id, std::size_t sizeBytes, const void* data, BufferUsage usage) override;

    FramebufferId createFramebuffer() override;
    void releaseFramebuffer(FramebufferId id) override;

    void bindFramebuffer(FramebufferId id) override;
    void framebufferAttachColorTexture(FramebufferId fbo, BackendTextureHandle colorTexture) override;
    void framebufferAttachDepthTexture(FramebufferId fbo, BackendTextureHandle depthTexture) override;
    void framebufferAttachDepthRenderbuffer(FramebufferId fbo, int width, int height) override;
    bool isFramebufferComplete(FramebufferId fbo) override;

    void endRenderToTexture() override;

    void pushFramebuffer() override;
    void popFramebuffer() override;

    void readPixelsFloat(int x, int y, int width, int height, float* rgba) override;
    void readPixelsUByte(int x, int y, int width, int height, unsigned char* rgba) override;

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
    ProgramId createProgramFromFiles(
        std::string_view vertexShaderPath,
        std::string_view fragmentShaderPath,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName);
    void releaseProgram(ProgramId id);
    void useProgram(ProgramId id);
    UniformLocationId uniformLocation(ProgramId id, std::string_view name) const;
    AttributeLocationId attribLocation(ProgramId id, std::string_view name) const;

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
    void executeCommand(const BackendCommandSetMultisample& command);
    void executeCommand(const BackendCommandDraw& command);
    void executeCommand(const BackendCommandDrawIndexed& command);
    void executeCommand(const BackendCommandSetBlendState& command);
    void executeCommand(const BackendCommandSetCullFace& command);
    void executeCommand(const BackendCommandSetCullFaceMode& command);
    void executeCommand(const BackendCommandSetPolygonOffsetFill& command);
    void executeCommand(const BackendCommandSetPolygonOffset& command);
    void executeCommand(const BackendCommandSetAlphaTest& command);
    void executeCommand(const BackendCommandSetDepthMask& command);
    void executeCommand(const BackendCommandSetDepthFunc& command);
    void executeCommand(const BackendCommandSetDepthTest& command);
    void executeCommand(const BackendCommandSetUniform1i& command);
    void executeCommand(const BackendCommandSetUniform1f& command);
    void executeCommand(const BackendCommandSetUniform1fv& command);
    void executeCommand(const BackendCommandSetUniform2f& command);
    void executeCommand(const BackendCommandSetUniform3f& command);
    void executeCommand(const BackendCommandSetUniform3fv& command);
    void executeCommand(const BackendCommandSetUniformMatrix4fv& command);
    void executeCommand(const BackendCommandSetVertexAttribPointer& command);
    void executeCommand(const BackendCommandSetProgram& command);
    void executeCommand(const BackendCommandBindPipeline& command);
    void executeCommand(const BackendCommandBindTexture2D& command);
    void executeCommand(const BackendCommandBufferData& command);
    void executeCommand(const BackendCommandBindBuffer& command);
    void executeCommand(const BackendCommandBeginRenderToTexture& command);
    void executeCommand(const BackendCommandEndRenderToTexture& command);
    void executeCommand(const BackendCommandSetLineWidth& command);
    void executeCommand(const BackendCommandBeginRenderPass& command);
    void executeCommand(const BackendCommandEndRenderPass& command);
    void executeCommand(const BackendCommandBindDefaultFramebuffer& command);
    void executeCommand(const BackendCommandBindFramebuffer& command);

    void flushPendingDeletes();
    std::size_t activeCommandBufferCount() const;

    struct Pipeline
    {
        bool alive{};
        ProgramId programId{};
        std::vector<VertexAttributeDesc> vertexAttributes{};
        std::vector<std::pair<std::string, UniformLocationId>> uniforms{};
        std::vector<std::pair<std::string, AttributeLocationId>> attributes{};
    };

    std::vector<GLuint> programs_{};
    std::vector<Pipeline> pipelines_{};

    struct RenderPass
    {
        bool alive{};
        RenderPassDesc desc{};
    };
    std::vector<RenderPass> renderPasses_{};

    std::vector<GLuint> buffers_{};
    std::vector<GLuint> framebuffers_{};
    std::vector<GLuint> framebufferStack_{};
    std::vector<CommandBuffer> commandBuffers_{};

    GLuint fallbackTexture2D_{};
    std::vector<GLuint> pendingTextureDeletes_{};
    bool initialized_{};

    static constexpr int MaxVertexAttribs = 16;
    static constexpr int MaxTextureUnits = 4;

    class StateCache
    {
    public:
        void reset();
        void resetTextureUnits();

        GLuint currentProgram_{};
        std::bitset<MaxVertexAttribs> enabledAttribs_{};

        struct TextureUnitState
        {
            GLuint texture{};
            GLenum minFilter{};
            GLenum magFilter{};
        };
        std::array<TextureUnitState, MaxTextureUnits> textureUnits_{};
    };
    StateCache stateCache_{};

    SDL_Window* window_{};
    SDL_GLContext glContext_{};
};

} // namespace OpenGL

} // namespace Ren
