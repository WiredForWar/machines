#pragma once

#include "render/OpenGL/StandardUniforms.hpp"
#include "render/internal/CommandArena.hpp"
#include "render/internal/IRenderBackend.hpp"

#include <array>
#include <bitset>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <GL/glew.h>

namespace Ren
{

namespace OpenGL
{

class RenderBackendGL21 final : public IRenderBackend
{
public:
    RenderBackendGL21();

    BackendType backendType() const override;

    bool initialize(IRenderSurface* surface, const IShaderSource* shaders) override;
    void shutdown() override;

    bool isInitialized() const override;

    bool setVSync(bool enabled) override;

    std::vector<ShaderSet> supportedShaderSets() const override;
    ShaderSet shaderSet() const override;
    bool setShaderSet(ShaderSet set) override;

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

    Viewport getViewport() const override;
    void clearDisplay(int width, int height) override;

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
    ProgramId addProgram(
        const std::string& vertexShaderCode,
        const std::string& fragmentShaderCode,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName);
    void releaseProgram(ProgramId id);
    void useProgram(ProgramId id);
    UniformLocationId uniformLocation(ProgramId id, std::string_view name) const;
    AttributeLocationId attribLocation(ProgramId id, std::string_view name) const;

    GLuint programHandle(ProgramId id) const;

    GLuint bufferHandle(BufferId id) const;

    GLuint framebufferHandle(FramebufferId id) const;

    struct CommandBuffer
    {
        bool alive{};
        bool recording{};
        std::vector<BackendCommand> commands{};
        // Backs the payloads of queued commands; unused while executing
        // immediately, since then nothing outlives its recordCommand() call.
        CommandArena arena{};
        // Vertex and index payloads gathered in record order when uploads are
        // coalesced, so that submit can push each in a single glBufferData.
        std::vector<std::byte> arrayStaging{};
        std::vector<std::byte> elementStaging{};

        // Drops what was recorded while keeping the capacity of everything
        // backing it, ready for the next recording.
        void resetRecording()
        {
            commands.clear();
            arena.reset();
            arrayStaging.clear();
            elementStaging.clear();
        }
    };
    CommandBuffer* commandBufferFromHandle(BackendCommandBufferHandle handle);
    const CommandBuffer* commandBufferFromHandle(BackendCommandBufferHandle handle) const;

    // Coalesced upload support. Payloads are staged at record time and uploaded
    // once per target per submit; the commands that reference them are then
    // replayed against the shared buffer with their offsets rebased.
    static std::size_t stagePayload(std::vector<std::byte>& staging, const void* data, std::size_t sizeBytes);
    void uploadStagedPayloads(CommandBuffer& buffer);
    void submitCoalesced(CommandBuffer& buffer);
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
    void executeCommand(const BackendCommandSetVertexAttribPointer& command);
    void executeCommand(const BackendCommandSetProgram& command);
    void executeCommand(const BackendCommandBindPipeline& command);
    void executeCommand(const BackendCommandBindTexture2D& command);
    void executeCommand(const BackendCommandBufferData& command);
    void executeCommand(const BackendCommandBindBuffer& command);
    void executeCommand(const BackendCommandBeginRenderToTexture& command);
    void executeCommand(const BackendCommandEndRenderToTexture& command);
    void executeCommand(const BackendCommandSetLineWidth& command);
    void executeCommand(const BackendCommandSetPointSize& command);
    void executeCommand(const BackendCommandBeginRenderPass& command);
    void executeCommand(const BackendCommandEndRenderPass& command);
    void executeCommand(const BackendCommandBindDefaultFramebuffer& command);
    void executeCommand(const BackendCommandBindFramebuffer& command);
    void executeCommand(const BackendCommandSetGui2DUniforms& command);
    void executeCommand(const BackendCommandSetStandardFrameUniforms& command);
    void executeCommand(const BackendCommandSetStandardObjectUniforms& command);
    void executeCommand(const BackendCommandSetBillboardUniforms& command);
    void executeCommand(const BackendCommandSetShadowDepthUniforms& command);
    void executeCommand(const BackendCommandSetPostProcessUniforms& command);

    void applyPendingAttribDisables();
    void flushPendingDeletes();
    std::size_t activeCommandBufferCount() const;

    // Locations of the uniforms making up the standard blocks, resolved once
    // when the pipeline is created. -1 for the ones a given program does not
    // declare, which is how the setters know to skip them.

    // The last value sent for each uniform of the standard blocks. Zero
    // initialised, which matches the state of a freshly linked program: GL
    // defaults its uniforms to zero, so a cache that starts at zero is already
    // in step with the driver and the first redundant send can be skipped too.
    struct Pipeline
    {
        bool alive{};
        ProgramId programId{};
        std::vector<VertexAttributeDesc> vertexAttributes{};
        std::vector<std::pair<std::string, UniformLocationId>> uniforms{};
        std::vector<std::pair<std::string, AttributeLocationId>> attributes{};
        StandardUniforms standard{};
    };


    // The pipeline bound by the last BindPipeline command. Every uniform block
    // command is recorded after one, so this is whose locations they use and
    // whose sent-value cache they update. Uniform values live in the program
    // object, so keeping the cache per pipeline needs no invalidation when
    // another pipeline is bound in between.
    Pipeline* boundPipeline();

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
    ShaderSet shaderSet_{ ShaderSet::GLSL120 };

    static constexpr int MaxVertexAttribs = 16;
    static constexpr int MaxTextureUnits = 4;

    class StateCache
    {
    public:
        void reset();
        void resetTextureUnits();

        GLuint currentProgram_{};
        std::bitset<MaxVertexAttribs> enabledAttribs_{};
        // Attributes a draw asked to disable, held back until the next draw
        // so that a disable followed by a re-enable costs nothing.
        std::bitset<MaxVertexAttribs> pendingAttribDisables_{};

        struct TextureUnitState
        {
            GLuint texture{};
            GLenum minFilter{};
            GLenum magFilter{};
        };
        std::array<TextureUnitState, MaxTextureUnits> textureUnits_{};

        std::optional<bool> depthTestEnabled_{};
        std::optional<bool> depthMaskWritable_{};
        std::optional<GLenum> depthFunc_{};

        struct BlendState
        {
            bool enabled{};
            GLenum srcFactor{};
            GLenum dstFactor{};
        };
        std::optional<BlendState> blend_{};

        std::optional<bool> cullFaceEnabled_{};
        std::optional<GLenum> cullFaceMode_{};

        struct AlphaTestState
        {
            bool enabled{};
            float reference{};
        };
        std::optional<AlphaTestState> alphaTest_{};

        struct PolygonOffsetState
        {
            bool fillEnabled{};
            float factor{};
            float units{};
        };
        std::optional<PolygonOffsetState> polygonOffset_{};

        std::optional<bool> multisampleEnabled_{};

        GLuint boundArrayBuffer_{};
        GLuint boundElementBuffer_{};

        std::optional<Viewport> viewport_{};
    };
    StateCache stateCache_{};

    GLuint currentFboColorAttachment_{};

    PipelineId currentPipelineId_{};

    // Shared buffers the coalesced payloads are uploaded into, created lazily.
    BufferId streamArrayBuffer_{};
    BufferId streamElementBuffer_{};

    IGLRenderSurface* glSurface_{};
    const IShaderSource* shaders_{};
};

} // namespace OpenGL

} // namespace Ren
