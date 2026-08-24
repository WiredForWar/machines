#include "render/OpenGL/RenderBackendGL21.hpp"
#include "render/OpenGL/BackendGL21.hpp"
#include "render/OpenGL/ShaderProgram.hpp"
#include "render/OpenGL/Utils.hpp"

#include "render/internal/SurfaceManagerImpl.hpp"
#include "render/internal/SurfaceBody.hpp"
#include "render/SurfaceManager.hpp"

#include "base/PrePost.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <variant>

// Immediate mode is more performant, at least for the legacy renderer.
// Yet we want to have this switch to debug commands buffer implementation.
constexpr bool ImmediateExecution{true};

// Gather the vertex and index payloads of a queued command buffer and upload
// each in one call at submit, instead of one glBufferData per draw call. Only
// meaningful when the commands are queued: immediate execution has nothing to
// gather, since every command runs before the next is recorded.
constexpr bool CoalesceUploads{!ImmediateExecution};

namespace Ren
{

namespace OpenGL
{

namespace
{

// Point light arrays are borrowed from the caller, so a queued command needs
// its own copy of them. Three floats per light for the vec3 arrays, one for
// the scalars.
void stagePointLights(StandardObjectUniforms& uniforms, CommandArena& arena)
{
    const std::size_t lights = static_cast<std::size_t>(uniforms.numPointLights);
    if (lights == 0)
        return;

    const std::size_t vec3Bytes = lights * 3 * sizeof(float);
    const std::size_t scalarBytes = lights * sizeof(float);

    const auto stage = [&arena](const float* source, std::size_t bytes) -> const float* {
        if (source == nullptr)
            return nullptr;
        return static_cast<const float*>(arena.append(source, bytes));
    };

    uniforms.pointLightPos = stage(uniforms.pointLightPos, vec3Bytes);
    uniforms.pointLightColor = stage(uniforms.pointLightColor, vec3Bytes);
    uniforms.pointLightAtten = stage(uniforms.pointLightAtten, vec3Bytes);
    uniforms.pointLightRange = stage(uniforms.pointLightRange, scalarBytes);
    uniforms.pointLightOmni = stage(uniforms.pointLightOmni, scalarBytes);
}

} // namespace

RenderBackendGL21::RenderBackendGL21()
    : programs_{0,}
    , buffers_{0,}
    , framebuffers_{0,}
{
    // Slot 0 is the reserved invalid handle. Constructed in place because a
    // command buffer owns its arena and so is not copyable.
    commandBuffers_.emplace_back();
}

BackendType RenderBackendGL21::backendType() const
{
    return BackendType::GL21;
}

bool RenderBackendGL21::initialize(IRenderSurface* surface, const IShaderSource* shaders)
{
    if (initialized_)
        return false;

    auto* gl = dynamic_cast<IGLRenderSurface*>(surface);
    if (!gl)
    {
        spdlog::error("RenderBackendGL21: surface cannot provide an OpenGL context");
        return false;
    }

    if (shaders == nullptr)
    {
        spdlog::error("RenderBackendGL21: no shader source given");
        return false;
    }

    glSurface_ = gl;
    shaders_ = shaders;

    if (!glSurface_->createGLContext({
            .majorVersion = 2,
            .minorVersion = 1,
        }))
    {
        spdlog::error("RenderBackendGL21: createGLContext() failed");
        glSurface_ = nullptr;
        return false;
    }

    const auto getGlStringAsConstChar = [](GLenum name)
    {
        auto pString = glGetString(name);
        return pString ? reinterpret_cast<const char*>(pString) : "<null>";
    };

    {
        spdlog::info("GL_RENDERER: {}", getGlStringAsConstChar(GL_RENDERER));
        spdlog::info("GL_VERSION: {}", getGlStringAsConstChar(GL_VERSION));
        spdlog::info("GL_VENDOR: {}", getGlStringAsConstChar(GL_VENDOR));
        spdlog::info("GL_SHADING_LANGUAGE_VERSION: {}", getGlStringAsConstChar(GL_SHADING_LANGUAGE_VERSION));
    }

    spdlog::info("Initializing GLEW...");
    GLenum glew_status = glewInit();
#if defined(GLEW_ERROR_NO_GLX_DISPLAY)
    if (glew_status == GLEW_ERROR_NO_GLX_DISPLAY)
    {
        spdlog::info("GLEW: no GLX display (assuming native Wayland); continuing without GLX extensions");
        glew_status = GLEW_OK;
    }
#endif
    if (glew_status != GLEW_OK)
    {
        spdlog::error("Fatal in glewInit: {}", reinterpret_cast<const char*>(glewGetErrorString(glew_status)));
        shutdown();
        return false;
    }

    if (!GLEW_VERSION_2_1)
    {
        spdlog::error("GLEW reports that OpenGL 2.1 is not available");
        shutdown();
        return false;
    }

    glGenTextures(1, &fallbackTexture2D_);
    if (fallbackTexture2D_ == 0)
    {
        shutdown();
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, fallbackTexture2D_);

    const std::uint32_t data = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data);

    if (glGetError() != GL_NO_ERROR)
    {
        shutdown();
        return false;
    }

    glFrontFace(GL_CW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    stateCache_.reset();
    currentPipelineId_ = 0;

    // Re-initialising means a new context, so any program that outlived the old
    // one has its uniforms back at zero and nothing recorded as already sent
    // still holds.
    for (Pipeline& pipeline : pipelines_)
        pipeline.standard.forgetSentValues();

    initialized_ = true;
    spdlog::info("RenderBackendGL21 initialized ({}x{})", glSurface_->width(), glSurface_->height());
    return true;
}

void RenderBackendGL21::StateCache::reset()
{
    currentProgram_ = 0;
    enabledAttribs_.reset();
    pendingAttribDisables_.reset();

    resetTextureUnits();

    depthTestEnabled_.reset();
    depthMaskWritable_.reset();
    depthFunc_.reset();
    blend_.reset();
    cullFaceEnabled_.reset();
    cullFaceMode_.reset();
    alphaTest_.reset();
    polygonOffset_.reset();
    multisampleEnabled_.reset();
    boundArrayBuffer_ = 0;
    boundElementBuffer_ = 0;
    viewport_.reset();
}

void RenderBackendGL21::StateCache::resetTextureUnits()
{
    textureUnits_ = {};
}

void RenderBackendGL21::shutdown()
{
    flushPendingDeletes();

    if (fallbackTexture2D_ != 0)
    {
        glDeleteTextures(1, &fallbackTexture2D_);
        fallbackTexture2D_ = 0;
    }

    initialized_ = false;

    if (glSurface_)
    {
        glSurface_->destroyGLContext();
        glSurface_ = nullptr;
    }
}

bool RenderBackendGL21::isInitialized() const
{
    return initialized_;
}

bool RenderBackendGL21::setVSync(bool enabled)
{
    if (!isInitialized())
    {
        spdlog::warn("Cannot set VSync: backend not initialised yet");
        return false;
    }

    if (!glSurface_)
        return false;

    using VSyncMode = IRenderSurface::VSyncMode;
    bool success{};

    if (enabled)
    {
        if (glSurface_->setVSyncMode(VSyncMode::Adaptive))
        {
            spdlog::info("Adaptive VSync enabled");
            success = true;
        }
        else if (glSurface_->setVSyncMode(VSyncMode::On))
        {
            spdlog::info("Standard VSync enabled (adaptive unavailable)");
            success = true;
        }
    }
    else if (glSurface_->setVSyncMode(VSyncMode::Off))
    {
        spdlog::info("VSync disabled");
        success = true;
    }

    if (!success)
    {
        spdlog::warn("Failed to apply VSync setting (enabled={})", enabled);
    }

    return success;
}

GLuint RenderBackendGL21::programHandle(ProgramId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= programs_.size())
        return 0;

    return programs_[idx];
}

GLuint RenderBackendGL21::bufferHandle(BufferId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= buffers_.size())
        return 0;

    return buffers_[idx];
}

GLuint RenderBackendGL21::framebufferHandle(FramebufferId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= framebuffers_.size())
        return 0;

    return framebuffers_[idx];
}

ProgramId RenderBackendGL21::addProgram(
    const std::string& vertexShaderCode,
    const std::string& fragmentShaderCode,
    std::string_view vertexShaderDebugName,
    std::string_view fragmentShaderDebugName)
{
    const GLuint program = createProgramFromSources(
        vertexShaderCode, fragmentShaderCode, vertexShaderDebugName, fragmentShaderDebugName);
    if (program == 0)
        return 0;

    programs_.push_back(program);
    return static_cast<ProgramId>(programs_.size() - 1);
}

void RenderBackendGL21::useProgram(ProgramId id)
{
    const GLuint handle = programHandle(id);
    if (handle == stateCache_.currentProgram_)
        return;
    stateCache_.currentProgram_ = handle;
    glUseProgram(handle);
}

UniformLocationId RenderBackendGL21::uniformLocation(ProgramId id, std::string_view name) const
{
    return UniformLocationId(glGetUniformLocation(programHandle(id), std::string(name).c_str()));
}

AttributeLocationId RenderBackendGL21::attribLocation(ProgramId id, std::string_view name) const
{
    return AttributeLocationId(glGetAttribLocation(programHandle(id), std::string(name).c_str()));
}

void RenderBackendGL21::releaseProgram(ProgramId id)
{
    if (id == 0)
        return;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= programs_.size())
        return;

    const GLuint program = programs_[idx];
    if (program != 0)
    {
        glDeleteProgram(program);
        programs_[idx] = 0;
    }
}

RenderBackendGL21::Pipeline* RenderBackendGL21::boundPipeline()
{
    if (currentPipelineId_ == 0 || currentPipelineId_ > pipelines_.size())
        return nullptr;

    return &pipelines_[currentPipelineId_ - 1];
}

std::vector<ShaderSet> RenderBackendGL21::supportedShaderSets() const
{
    return { ShaderSet::GLSL120 };
}

ShaderSet RenderBackendGL21::shaderSet() const
{
    return shaderSet_;
}

bool RenderBackendGL21::setShaderSet(ShaderSet set)
{
    const std::vector<ShaderSet> supported = supportedShaderSets();
    if (std::find(supported.begin(), supported.end(), set) == supported.end())
    {
        spdlog::error("The {} backend cannot compile the {} shader set", toString(backendType()), toString(set));
        return false;
    }

    shaderSet_ = set;
    return true;
}

PipelineId RenderBackendGL21::createPipeline(const PipelineDesc& desc)
{
    const std::optional<std::string> vertexCode
        = shaders_->source(shaderSet_, ShaderStage::Vertex, desc.vertexShader);
    const std::optional<std::string> fragmentCode
        = shaders_->source(shaderSet_, ShaderStage::Fragment, desc.fragmentShader);

    if (!vertexCode.has_value() || !fragmentCode.has_value())
    {
        if (desc.optional)
            spdlog::info("The {} shaders are not in the {} set", desc.vertexShader, toString(shaderSet_));
        else
            spdlog::error("The {} shaders are missing from the {} set", desc.vertexShader, toString(shaderSet_));

        return 0;
    }

    const ProgramId programId
        = addProgram(*vertexCode, *fragmentCode, desc.vertexShader, desc.fragmentShader);
    if (programId == 0)
        return 0;

    Pipeline pipeline;
    pipeline.alive = true;
    pipeline.programId = programId;
    pipeline.vertexAttributes = desc.vertexAttributes;

    for (const auto& uniformName : desc.uniformNames)
    {
        const auto loc = uniformLocation(programId, uniformName);
        if (!loc.isValid())
            spdlog::warn("Uniform '{}' not found in shader program", uniformName);
        pipeline.uniforms.emplace_back(uniformName, loc);
    }

    for (const auto& attr : desc.vertexAttributes)
    {
        pipeline.attributes.emplace_back(attr.name, attribLocation(programId, attr.name));
    }

    pipeline.standard.resolve(programHandle(programId));

    pipelines_.push_back(std::move(pipeline));
    return static_cast<PipelineId>(pipelines_.size());
}

void RenderBackendGL21::releasePipeline(PipelineId id)
{
    if (id == 0 || id > pipelines_.size())
        return;

    Pipeline& pipeline = pipelines_[id - 1];
    if (!pipeline.alive)
        return;

    releaseProgram(pipeline.programId);
    pipeline.alive = false;
    pipeline.programId = 0;
}

UniformLocationId RenderBackendGL21::pipelineUniformLocation(PipelineId id, std::string_view name) const
{
    if (id == 0 || id > pipelines_.size())
        return UniformLocationId{};

    const Pipeline& pipeline = pipelines_[id - 1];
    for (const auto& [uniformName, location] : pipeline.uniforms)
    {
        if (uniformName == name)
            return location;
    }
    spdlog::error("Uniform '{}' not registered in pipeline descriptor", name);
    return UniformLocationId{};
}

AttributeLocationId RenderBackendGL21::pipelineAttribLocation(PipelineId id, std::string_view name) const
{
    if (id == 0 || id > pipelines_.size())
        return AttributeLocationId{};

    const Pipeline& pipeline = pipelines_[id - 1];
    for (const auto& [attrName, location] : pipeline.attributes)
    {
        if (attrName == name)
            return location;
    }
    return AttributeLocationId{};
}

RenderPassId RenderBackendGL21::createRenderPass(const RenderPassDesc& desc)
{
    RenderPass pass;
    pass.alive = true;
    pass.desc = desc;

    renderPasses_.push_back(std::move(pass));
    return static_cast<RenderPassId>(renderPasses_.size());
}

void RenderBackendGL21::releaseRenderPass(RenderPassId id)
{
    if (id == 0 || id > renderPasses_.size())
        return;

    renderPasses_[id - 1].alive = false;
}

BufferId RenderBackendGL21::createBuffer()
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    if (buffer == 0)
        return 0;

    buffers_.push_back(buffer);
    return static_cast<BufferId>(buffers_.size() - 1);
}

void RenderBackendGL21::bindBuffer(BufferTarget target, BufferId id)
{
    const GLuint buffer = bufferHandle(id);
    if (target == BufferTarget::Array)
    {
        if (stateCache_.boundArrayBuffer_ == buffer)
            return;
        stateCache_.boundArrayBuffer_ = buffer;
    }
    else
    {
        if (stateCache_.boundElementBuffer_ == buffer)
            return;
        stateCache_.boundElementBuffer_ = buffer;
    }
    const GLenum glTarget = (target == BufferTarget::Array) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    glBindBuffer(glTarget, buffer);
}

void RenderBackendGL21::bufferData(
    BufferTarget target, BufferId id, std::size_t sizeBytes, const void* data, BufferUsage usage)
{
    bindBuffer(target, id);

    const GLenum glTarget = (target == BufferTarget::Array) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    const GLenum glUsage = (usage == BufferUsage::StreamDraw) ? GL_STREAM_DRAW : GL_STREAM_DRAW;
    glBufferData(glTarget, static_cast<GLsizeiptr>(sizeBytes), data, glUsage);
}

void RenderBackendGL21::releaseBuffer(BufferId id)
{
    if (id == 0)
        return;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= buffers_.size())
        return;

    const GLuint buffer = buffers_[idx];
    if (buffer != 0)
    {
        glDeleteBuffers(1, &buffer);
        buffers_[idx] = 0;
    }

    stateCache_.boundArrayBuffer_ = 0;
    stateCache_.boundElementBuffer_ = 0;
}

FramebufferId RenderBackendGL21::createFramebuffer()
{
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    if (framebuffer == 0)
        return 0;

    framebuffers_.push_back(framebuffer);
    return static_cast<FramebufferId>(framebuffers_.size() - 1);
}

void RenderBackendGL21::bindFramebuffer(FramebufferId id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle(id));
}

void RenderBackendGL21::framebufferAttachDepthTexture(FramebufferId fbo, BackendTextureHandle depthTexture)
{
    const GLuint fboHandle = framebufferHandle(fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    const GLuint texHandle = depthTexture.isValid() ? depthTexture.value() : 0;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texHandle, 0);

    // Depth-only FBO: no color attachment
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (texHandle != 0)
    {
        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            spdlog::error("Depth-only framebuffer incomplete (status=0x{:X})", static_cast<unsigned int>(status));
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderBackendGL21::framebufferAttachColorTexture(FramebufferId fbo, BackendTextureHandle colorTexture)
{
    const GLuint fboHandle = framebufferHandle(fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    const GLuint texHandle = colorTexture.isValid() ? colorTexture.value() : 0;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texHandle, 0);

    if (texHandle != 0)
    {
        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            spdlog::error("Color framebuffer incomplete (status=0x{:X})", static_cast<unsigned int>(status));
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderBackendGL21::framebufferAttachDepthRenderbuffer(FramebufferId fbo, int width, int height)
{
    const GLuint fboHandle = framebufferHandle(fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    GLuint rbo{};
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Framebuffer with depth renderbuffer incomplete (status=0x{:X})", static_cast<unsigned int>(status));
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool RenderBackendGL21::isFramebufferComplete(FramebufferId fbo)
{
    const GLuint fboHandle = framebufferHandle(fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return status == GL_FRAMEBUFFER_COMPLETE;
}

void RenderBackendGL21::endRenderToTexture()
{
    if (!framebufferStack_.empty())
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    }
    currentFboColorAttachment_ = 0;
    popFramebuffer();
}

void RenderBackendGL21::pushFramebuffer()
{
    GLint current = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current);
    framebufferStack_.push_back(static_cast<GLuint>(current));
}

void RenderBackendGL21::popFramebuffer()
{
    if (framebufferStack_.empty())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    const GLuint restore = framebufferStack_.back();
    framebufferStack_.pop_back();
    glBindFramebuffer(GL_FRAMEBUFFER, restore);
}

Viewport RenderBackendGL21::getViewport() const
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    return {vp[0], vp[1], vp[2], vp[3]};
}

void RenderBackendGL21::clearDisplay(int width, int height)
{
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RenderBackendGL21::readPixelsFloat(int x, int y, int width, int height, float* rgba)
{
    glReadPixels(x, y, width, height, GL_RGBA, GL_FLOAT, rgba);
}

void RenderBackendGL21::readPixelsUByte(int x, int y, int width, int height, unsigned char* rgba)
{
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

void RenderBackendGL21::releaseFramebuffer(FramebufferId id)
{
    if (id == 0)
        return;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= framebuffers_.size())
        return;

    const GLuint framebuffer = framebuffers_[idx];
    if (framebuffer != 0)
    {
        glDeleteFramebuffers(1, &framebuffer);
        framebuffers_[idx] = 0;
    }
}


BackendCommandBufferHandle RenderBackendGL21::createCommandBuffer()
{
    if (commandBuffers_.empty())
    {
        commandBuffers_.emplace_back();
    }

    for (std::size_t idx = 1; idx < commandBuffers_.size(); ++idx)
    {
        CommandBuffer& buffer = commandBuffers_[idx];
        if (!buffer.alive)
        {
            buffer.alive = true;
            buffer.recording = false;
            buffer.resetRecording();
            return BackendCommandBufferHandle(static_cast<std::uint32_t>(idx));
        }
    }

    commandBuffers_.emplace_back();
    CommandBuffer& buffer = commandBuffers_.back();
    buffer.alive = true;
    buffer.recording = false;
    buffer.resetRecording();
    return BackendCommandBufferHandle(static_cast<std::uint32_t>(commandBuffers_.size() - 1));
}

void RenderBackendGL21::destroyCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr)
        return;

    buffer->alive = false;
    buffer->recording = false;
    buffer->resetRecording();
}

void RenderBackendGL21::beginCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr)
        return;

    buffer->recording = true;
    buffer->resetRecording();
}

void RenderBackendGL21::recordCommand(BackendCommandBufferHandle handle, BackendCommand&& command)
{
    if (!handle.isValid())
        return;

    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr || !buffer->recording)
        return;

    if constexpr (ImmediateExecution)
    {
        executeCommand(command);
    }
    else
    {
        // Commands only borrow their payloads, so anything queued past the end
        // of this call has to be given storage that lives until submit.
        if (auto* bufferDataCommand = std::get_if<BackendCommandBufferData>(&command))
        {
            if constexpr (CoalesceUploads)
            {
                std::vector<std::byte>& staging = bufferDataCommand->target == BufferTarget::Array
                    ? buffer->arrayStaging
                    : buffer->elementStaging;
                bufferDataCommand->stagingOffset
                    = stagePayload(staging, bufferDataCommand->data, bufferDataCommand->sizeBytes);
                bufferDataCommand->data = nullptr;
            }
            else
            {
                bufferDataCommand->data
                    = buffer->arena.append(bufferDataCommand->data, bufferDataCommand->sizeBytes);
            }
        }
        else if (auto* objectUniforms = std::get_if<BackendCommandSetStandardObjectUniforms>(&command))
        {
            stagePointLights(objectUniforms->uniforms, buffer->arena);
        }

        buffer->commands.push_back(std::move(command));
    }
}

void RenderBackendGL21::endCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr || !buffer->recording)
        return;

    buffer->recording = false;
}

void RenderBackendGL21::submitCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr || buffer->recording)
        return;

    if constexpr (CoalesceUploads)
    {
        submitCoalesced(*buffer);
    }
    else
    {
        for (const BackendCommand& command : buffer->commands)
        {
            std::visit([this](const auto& cmd) { executeCommand(cmd); }, command);
        }
    }

    buffer->resetRecording();

    // Only flush pending deletes when no other buffer is still recording,
    // to avoid deleting resources that queued commands may still reference.
    if (activeCommandBufferCount() == 0)
        flushPendingDeletes();
}

std::size_t RenderBackendGL21::stagePayload(
    std::vector<std::byte>& staging, const void* data, std::size_t sizeBytes)
{
    // Vertex attribute pointers into the shared buffer have to stay naturally
    // aligned for their component type, so start every payload on a 16-byte
    // boundary rather than packing them tightly.
    constexpr std::size_t alignment = 16;
    const std::size_t padded = (staging.size() + alignment - 1) & ~(alignment - 1);
    staging.resize(padded + sizeBytes);
    if (sizeBytes != 0)
        std::memcpy(staging.data() + padded, data, sizeBytes);

    return padded;
}

void RenderBackendGL21::uploadStagedPayloads(CommandBuffer& buffer)
{
    if (!buffer.arrayStaging.empty())
    {
        if (streamArrayBuffer_ == 0)
            streamArrayBuffer_ = createBuffer();

        bufferData(
            BufferTarget::Array,
            streamArrayBuffer_,
            buffer.arrayStaging.size(),
            buffer.arrayStaging.data(),
            BufferUsage::StreamDraw);
    }

    if (!buffer.elementStaging.empty())
    {
        if (streamElementBuffer_ == 0)
            streamElementBuffer_ = createBuffer();

        bufferData(
            BufferTarget::ElementArray,
            streamElementBuffer_,
            buffer.elementStaging.size(),
            buffer.elementStaging.data(),
            BufferUsage::StreamDraw);
    }
}

void RenderBackendGL21::submitCoalesced(CommandBuffer& buffer)
{
    uploadStagedPayloads(buffer);

    // Every payload now lives in one of the two shared buffers, so the commands
    // that referenced the per-draw buffers have to be rebased onto it: a bind
    // targets the shared buffer, and the offsets of anything reading through
    // that binding shift by where the payload landed.
    std::unordered_map<BufferId, std::size_t> coalescedOffset;
    std::size_t arrayBase = 0;
    std::size_t elementBase = 0;

    const auto rebind = [&](BufferTarget target, BufferId bufferId) {
        const auto it = coalescedOffset.find(bufferId);
        const bool coalesced = it != coalescedOffset.end();
        if (target == BufferTarget::Array)
        {
            arrayBase = coalesced ? it->second : 0;
            bindBuffer(target, coalesced ? streamArrayBuffer_ : bufferId);
        }
        else
        {
            elementBase = coalesced ? it->second : 0;
            bindBuffer(target, coalesced ? streamElementBuffer_ : bufferId);
        }
    };

    for (const BackendCommand& command : buffer.commands)
    {
        if (const auto* upload = std::get_if<BackendCommandBufferData>(&command))
        {
            // Already uploaded. bufferData() used to leave its buffer bound and
            // the element path relies on that, so bind here in its place.
            coalescedOffset[upload->bufferId] = upload->stagingOffset;
            rebind(upload->target, upload->bufferId);
            continue;
        }

        if (const auto* bind = std::get_if<BackendCommandBindBuffer>(&command))
        {
            rebind(bind->target, bind->bufferId);
            continue;
        }

        if (const auto* attrib = std::get_if<BackendCommandSetVertexAttribPointer>(&command))
        {
            BackendCommandSetVertexAttribPointer rebased = *attrib;
            rebased.offset += arrayBase;
            executeCommand(rebased);
            continue;
        }

        if (const auto* drawIndexed = std::get_if<BackendCommandDrawIndexed>(&command))
        {
            BackendCommandDrawIndexed rebased = *drawIndexed;
            rebased.indexBufferOffset += elementBase;
            executeCommand(rebased);
            continue;
        }

        std::visit([this](const auto& cmd) { executeCommand(cmd); }, command);
    }
}

void RenderBackendGL21::flushPendingDeletes()
{
    PRE(activeCommandBufferCount() == 0);

    if (!pendingTextureDeletes_.empty())
    {
        glDeleteTextures(
            static_cast<GLsizei>(pendingTextureDeletes_.size()),
            pendingTextureDeletes_.data());
        pendingTextureDeletes_.clear();

        // Invalidate texture cache — GL may reuse deleted handles.
        stateCache_.textureUnits_ = {};
    }
}

std::size_t RenderBackendGL21::activeCommandBufferCount() const
{
    std::size_t count{};
    for (const auto& buffer : commandBuffers_)
    {
        if (buffer.alive && buffer.recording)
            ++count;
    }
    return count;
}

RenderBackendGL21::CommandBuffer* RenderBackendGL21::commandBufferFromHandle(BackendCommandBufferHandle handle)
{
    if (!handle.isValid())
        return nullptr;

    const std::uint32_t idx = handle.value();
    if (idx >= commandBuffers_.size())
        return nullptr;

    CommandBuffer& buffer = commandBuffers_[idx];
    if (!buffer.alive)
        return nullptr;

    return &buffer;
}

const RenderBackendGL21::CommandBuffer* RenderBackendGL21::commandBufferFromHandle(BackendCommandBufferHandle handle) const
{
    if (!handle.isValid())
        return nullptr;

    const std::uint32_t idx = handle.value();
    if (idx >= commandBuffers_.size())
        return nullptr;

    const CommandBuffer& buffer = commandBuffers_[idx];
    if (!buffer.alive)
        return nullptr;

    return &buffer;
}

void RenderBackendGL21::executeCommand(const BackendCommand& command)
{
    std::visit([this](const auto& cmd) { executeCommand(cmd); }, command);
}

void RenderBackendGL21::executeCommand(const BackendCommandClear& command)
{
    const GLbitfield mask = toClearMask(command.mask);
    if (mask == 0)
        return;

    GLfloat previousClearColor[4]{};
    const bool affectsColour = (mask & GL_COLOR_BUFFER_BIT) != 0;
    if (affectsColour)
    {
        glGetFloatv(GL_COLOR_CLEAR_VALUE, previousClearColor);
        glClearColor(command.r, command.g, command.b, command.a);
    }

    glClear(mask);

    if (affectsColour)
    {
        glClearColor(previousClearColor[0], previousClearColor[1], previousClearColor[2], previousClearColor[3]);
    }
}

void RenderBackendGL21::executeCommand(const BackendCommandSetViewport& command)
{
    if (stateCache_.viewport_.has_value())
    {
        const auto& v = *stateCache_.viewport_;
        if (v.x == command.x && v.y == command.y && v.width == command.width && v.height == command.height)
            return;
    }
    stateCache_.viewport_ = {command.x, command.y, command.width, command.height};
    glViewport(command.x, command.y, command.width, command.height);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetMultisample& command)
{
    if (stateCache_.multisampleEnabled_ == command.enabled)
        return;
    stateCache_.multisampleEnabled_ = command.enabled;
    if (command.enabled)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}

void RenderBackendGL21::executeCommand(const BackendCommandDraw& command)
{
    applyPendingAttribDisables();

    const GLenum mode = toDrawMode(command.topology);
    glDrawArrays(mode, command.first, command.count);
}

void RenderBackendGL21::executeCommand(const BackendCommandDrawIndexed& command)
{
    applyPendingAttribDisables();

    const GLenum mode = toDrawMode(command.topology);
    const GLenum indexType = toIndexType(command.indexType);
    glDrawElements(mode, command.count, indexType, reinterpret_cast<const void*>(command.indexBufferOffset));
}

void RenderBackendGL21::executeCommand(const BackendCommandSetBlendState& command)
{
    const GLenum src = command.enabled ? toBlendFactor(command.srcFactor) : 0;
    const GLenum dst = command.enabled ? toBlendFactor(command.dstFactor) : 0;
    if (stateCache_.blend_.has_value())
    {
        const auto& b = *stateCache_.blend_;
        if (b.enabled == command.enabled && b.srcFactor == src && b.dstFactor == dst)
            return;
    }
    stateCache_.blend_ = {command.enabled, src, dst};
    if (command.enabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(src, dst);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void RenderBackendGL21::executeCommand(const BackendCommandSetCullFace& command)
{
    if (stateCache_.cullFaceEnabled_ == command.enabled)
        return;
    stateCache_.cullFaceEnabled_ = command.enabled;
    if (command.enabled)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetCullFaceMode& command)
{
    const GLenum mode = command.mode == BackendCullFaceMode::Front ? GL_FRONT : GL_BACK;
    if (stateCache_.cullFaceMode_ == mode)
        return;
    stateCache_.cullFaceMode_ = mode;
    glCullFace(mode);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetPolygonOffsetFill& command)
{
    if (stateCache_.polygonOffset_.has_value() && stateCache_.polygonOffset_->fillEnabled == command.enabled)
        return;
    if (!stateCache_.polygonOffset_.has_value())
        stateCache_.polygonOffset_ = {command.enabled, 0.0f, 0.0f};
    else
        stateCache_.polygonOffset_->fillEnabled = command.enabled;
    if (command.enabled)
        glEnable(GL_POLYGON_OFFSET_FILL);
    else
        glDisable(GL_POLYGON_OFFSET_FILL);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetPolygonOffset& command)
{
    if (stateCache_.polygonOffset_.has_value()
        && stateCache_.polygonOffset_->factor == command.factor
        && stateCache_.polygonOffset_->units == command.units)
        return;
    if (!stateCache_.polygonOffset_.has_value())
        stateCache_.polygonOffset_ = {false, command.factor, command.units};
    else
    {
        stateCache_.polygonOffset_->factor = command.factor;
        stateCache_.polygonOffset_->units = command.units;
    }
    glPolygonOffset(command.factor, command.units);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetAlphaTest& command)
{
    if (stateCache_.alphaTest_.has_value())
    {
        const auto& a = *stateCache_.alphaTest_;
        if (a.enabled == command.enabled && (!command.enabled || a.reference == command.reference))
            return;
    }
    stateCache_.alphaTest_ = {command.enabled, command.reference};
    if (command.enabled)
    {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, command.reference);
    }
    else
    {
        glDisable(GL_ALPHA_TEST);
    }
}

void RenderBackendGL21::executeCommand(const BackendCommandSetDepthMask& command)
{
    if (stateCache_.depthMaskWritable_ == command.writable)
        return;
    stateCache_.depthMaskWritable_ = command.writable;
    glDepthMask(command.writable ? GL_TRUE : GL_FALSE);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetDepthFunc& command)
{
    const GLenum func = toDepthFunc(command.function);
    if (stateCache_.depthFunc_ == func)
        return;
    stateCache_.depthFunc_ = func;
    glDepthFunc(func);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetDepthTest& command)
{
    if (stateCache_.depthTestEnabled_ == command.enabled)
        return;
    stateCache_.depthTestEnabled_ = command.enabled;
    if (command.enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetVertexAttribPointer& command)
{
    if (!command.index.isValid())
        return;

    const GLuint index = static_cast<GLuint>(command.index.value());

    if (command.enabled)
    {
        if (index < MaxVertexAttribs)
        {
            // Whatever this attribute was queued to be disabled for, it is
            // wanted again before the next draw, so the disable is moot.
            stateCache_.pendingAttribDisables_.reset(index);

            if (!stateCache_.enabledAttribs_.test(index))
            {
                stateCache_.enabledAttribs_.set(index);
                glEnableVertexAttribArray(index);
            }
        }

        glVertexAttribPointer(
            index,
            command.size,
            toVertexAttribType(command.type),
            command.normalized ? GL_TRUE : GL_FALSE,
            static_cast<GLsizei>(command.stride),
            reinterpret_cast<const void*>(command.offset));
    }
    else
    {
        // Deferred to the next draw rather than issued now. Every draw call
        // disables the layout it just used and the next one re-enables the
        // same attributes, so acting immediately would spend two driver calls
        // per attribute per draw to arrive back where it started.
        if (index < MaxVertexAttribs && stateCache_.enabledAttribs_.test(index))
            stateCache_.pendingAttribDisables_.set(index);
    }
}

// Brings the enabled attribute set to what the pending disables asked for. Run
// before every draw, so the state a draw sees is the same as if each disable
// had been issued when it was recorded.
void RenderBackendGL21::applyPendingAttribDisables()
{
    if (stateCache_.pendingAttribDisables_.none())
        return;

    for (std::size_t index = 0; index != MaxVertexAttribs; ++index)
    {
        if (!stateCache_.pendingAttribDisables_.test(index))
            continue;

        stateCache_.enabledAttribs_.reset(index);
        glDisableVertexAttribArray(static_cast<GLuint>(index));
    }

    stateCache_.pendingAttribDisables_.reset();
}

void RenderBackendGL21::executeCommand(const BackendCommandSetProgram& command)
{
    useProgram(command.programId);
}

void RenderBackendGL21::executeCommand(const BackendCommandBindPipeline& command)
{
    const PipelineId id = command.pipelineId;
    if (id == 0 || id > pipelines_.size())
        return;

    const Pipeline& pipeline = pipelines_[id - 1];
    if (!pipeline.alive)
        return;

    currentPipelineId_ = id;
    useProgram(pipeline.programId);
}

void RenderBackendGL21::executeCommand(const BackendCommandBindTexture2D& command)
{
    const GLuint textureHandle = command.textureHandle.isValid() ? command.textureHandle.value() : fallbackTexture2D_;
    const GLenum minF = toFilter(command.minFilter);
    const GLenum magF = toFilter(command.magFilter);
    const int unit = static_cast<int>(command.unit);

    ASSERT(
        currentFboColorAttachment_ == 0 || textureHandle != currentFboColorAttachment_,
        "Sampling from a texture that is the current FBO color attachment (GL feedback loop)");

    if (unit < MaxTextureUnits)
    {
        StateCache::TextureUnitState& cached = stateCache_.textureUnits_[unit];
        if (cached.texture == textureHandle && cached.minFilter == minF && cached.magFilter == magF)
            return;

        cached.texture = textureHandle;
        cached.minFilter = minF;
        cached.magFilter = magF;
    }

    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + command.unit));
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);
}

void RenderBackendGL21::executeCommand(const BackendCommandBufferData& command)
{
    bufferData(command.target, command.bufferId, command.sizeBytes, command.data, command.usage);
}

void RenderBackendGL21::executeCommand(const BackendCommandBindBuffer& command)
{
    bindBuffer(command.target, command.bufferId);
}

void RenderBackendGL21::executeCommand(const BackendCommandBeginRenderToTexture& command)
{
    if (command.framebufferId == 0 || !command.targetTexture.isValid())
        return;

    pushFramebuffer();
    bindFramebuffer(command.framebufferId);

    const GLuint textureHandle = command.targetTexture.value();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureHandle, 0);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Framebuffer incomplete (status=0x{:X})", static_cast<unsigned int>(status));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        currentFboColorAttachment_ = 0;
        popFramebuffer();
    }
    else
    {
        currentFboColorAttachment_ = textureHandle;
    }
}

void RenderBackendGL21::executeCommand(const BackendCommandBeginRenderPass& command)
{
    const RenderPassId id = command.renderPassId;
    if (id == 0 || id > renderPasses_.size())
        return;

    const RenderPass& pass = renderPasses_[id - 1];
    if (!pass.alive)
        return;

    // Bind framebuffer (0 = default framebuffer)
    if (command.framebufferId != 0)
        bindFramebuffer(command.framebufferId);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Apply load operations
    GLbitfield clearMask{};

    if (pass.desc.hasColorAttachment && pass.desc.colorAttachment.loadOp == LoadOp::Clear)
    {
        if (command.overrideClearColor)
            glClearColor(command.clearR, command.clearG, command.clearB, command.clearA);
        else
            glClearColor(
                pass.desc.colorAttachment.clearR,
                pass.desc.colorAttachment.clearG,
                pass.desc.colorAttachment.clearB,
                pass.desc.colorAttachment.clearA);
        clearMask |= GL_COLOR_BUFFER_BIT;
    }

    if (pass.desc.hasDepthAttachment && pass.desc.depthAttachment.loadOp == LoadOp::Clear)
    {
        // GL requires depth writes enabled for glClear to affect the depth buffer.
        stateCache_.depthMaskWritable_ = true;
        glDepthMask(GL_TRUE);
        clearMask |= GL_DEPTH_BUFFER_BIT;
    }

    if (clearMask != 0)
        glClear(clearMask);
}

void RenderBackendGL21::executeCommand(const BackendCommandEndRenderPass& /*command*/)
{
    // In GL 2.1, ending a render pass is a no-op.
    // In Vulkan, this would end the VkRenderPass.
}

void RenderBackendGL21::executeCommand(const BackendCommandBindDefaultFramebuffer& /*command*/)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderBackendGL21::executeCommand(const BackendCommandBindFramebuffer& command)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle(command.framebufferId));
}

void RenderBackendGL21::executeCommand(const BackendCommandEndRenderToTexture& /*command*/)
{
    endRenderToTexture();
}

void RenderBackendGL21::executeCommand(const BackendCommandSetLineWidth& command)
{
    glLineWidth(command.width);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetPointSize& command)
{
    glPointSize(command.size);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetGui2DUniforms& command)
{
    if (Pipeline* pipeline = boundPipeline())
        pipeline->standard.apply(command.uniforms);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetStandardFrameUniforms& command)
{
    if (Pipeline* pipeline = boundPipeline())
        pipeline->standard.apply(command.uniforms);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetStandardObjectUniforms& command)
{
    if (Pipeline* pipeline = boundPipeline())
        pipeline->standard.apply(command.uniforms);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetBillboardUniforms& command)
{
    if (Pipeline* pipeline = boundPipeline())
        pipeline->standard.apply(command.uniforms);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetShadowDepthUniforms& command)
{
    if (Pipeline* pipeline = boundPipeline())
        pipeline->standard.apply(command.uniforms);
}

void RenderBackendGL21::executeCommand(const BackendCommandSetPostProcessUniforms& command)
{
    if (Pipeline* pipeline = boundPipeline())
        pipeline->standard.apply(command.uniforms);
}

BackendTextureHandle RenderBackendGL21::createTexture2D()
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    return BackendTextureHandle(texture);
}

void RenderBackendGL21::destroyTexture2D(BackendTextureHandle handle)
{
    if (!handle.isValid())
        return;

    pendingTextureDeletes_.push_back(handle.value());
}

void RenderBackendGL21::textureStorage2D(BackendTextureHandle handle, int width, int height, TextureFormat format)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexImage2D(
        GL_TEXTURE_2D, 0, toStorageFormat(format), width, height, 0, toPixelFormat(format),
        toPixelDataType(format), nullptr);
}

void RenderBackendGL21::textureSubImage2D(
    BackendTextureHandle handle, int x, int y, int width, int height, TextureFormat format, const void* pixels)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, toPixelFormat(format), GL_UNSIGNED_BYTE, pixels);
}

void RenderBackendGL21::textureSetMinMagFilter(
    BackendTextureHandle handle, TextureFilter minFilter, TextureFilter magFilter)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toFilter(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toFilter(magFilter));
}

void RenderBackendGL21::textureSetWrap(BackendTextureHandle handle, TextureWrap wrapS, TextureWrap wrapT)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toWrap(wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toWrap(wrapT));
}

void RenderBackendGL21::textureGenerateMipmap(BackendTextureHandle handle)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glGenerateMipmap(GL_TEXTURE_2D);
}

bool canUseGL21(IRenderSurface* surface)
{
    return dynamic_cast<IGLRenderSurface*>(surface) != nullptr;
}

std::unique_ptr<IRenderBackend> createGL21()
{
    return std::make_unique<RenderBackendGL21>();
}

} // namespace OpenGL

} // namespace Ren
