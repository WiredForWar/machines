#include "render/OpenGL/RenderBackendGL.hpp"
#include "render/OpenGL/Utils.hpp"

#include "render/internal/SurfaceManagerImpl.hpp"
#include "render/internal/SurfaceBody.hpp"
#include "render/SurfaceManager.hpp"

#include "base/PrePost.hpp"

#include "spdlog/spdlog.h"

#include <SDL3/SDL.h>

#include <fstream>
#include <variant>

// Immediate mode is more performant, at least for the legacy renderer.
// Yet we want to have this switch to debug commands buffer implementation.
constexpr bool ImmediateExecution{true};

namespace Ren
{

namespace OpenGL
{

namespace
{

bool compileShader(GLuint shaderID, const std::string& code)
{
    const char* const sourcePointer = code.c_str();
    glShaderSource(shaderID, 1, &sourcePointer, nullptr);
    glCompileShader(shaderID);

    GLint result = GL_FALSE;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        std::string errorMessage;
        int infoLogLength{};
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 2)
        {
            errorMessage.resize(static_cast<std::size_t>(infoLogLength));
            glGetShaderInfoLog(shaderID, infoLogLength, nullptr, &errorMessage[0]);
        }
        spdlog::error("Shader compile error: {}", errorMessage);
    }

    return result == GL_TRUE;
}

} // namespace

RenderBackendGL::RenderBackendGL()
    : programs_{0,}
    , buffers_{0,}
    , framebuffers_{0,}
    , commandBuffers_{CommandBuffer{},}
{
}

bool RenderBackendGL::initialize(SDL_Window* window)
{
    if (initialized_)
        return false;

    if (window == nullptr)
        return false;

    window_ = window;

    constexpr int contextMajorVersion = 2;
    constexpr int contextMinorVersion = 1;
    constexpr int contextProfile = 0; // Also consider SDL_GL_CONTEXT_PROFILE_CORE (1)

    spdlog::info("Context version: {}.{} (SDL profile: {})", contextMajorVersion, contextMinorVersion, contextProfile);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, contextMajorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, contextMinorVersion);
    if (contextProfile)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, contextProfile);
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (glContext_ == nullptr)
    {
        spdlog::error("Fatal in SDL_GL_CreateContext: {}", SDL_GetError());
        window_ = nullptr;
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
        spdlog::error("GLEW reports that OpengGL 2.1 is not available");
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

    initialized_ = true;
    return true;
}

void RenderBackendGL::StateCache::reset()
{
    currentProgram_ = 0;
    enabledAttribs_.reset();

    resetTextureUnits();
}

void RenderBackendGL::StateCache::resetTextureUnits()
{
    textureUnits_ = {};
}

void RenderBackendGL::shutdown()
{
    flushPendingDeletes();

    if (fallbackTexture2D_ != 0)
    {
        glDeleteTextures(1, &fallbackTexture2D_);
        fallbackTexture2D_ = 0;
    }

    initialized_ = false;

    if (glContext_ != nullptr)
    {
        SDL_GL_MakeCurrent(nullptr, nullptr);
        SDL_GL_DestroyContext(glContext_);
        glContext_ = nullptr;
    }

    window_ = nullptr;
}

bool RenderBackendGL::isInitialized() const
{
    return initialized_;
}

bool RenderBackendGL::setVSync(bool enabled)
{
    if (!isInitialized())
    {
        spdlog::warn("Cannot set VSync: backend not initialised yet");
        return false;
    }

    bool success{};

    if (enabled)
    {
        if (SDL_GL_SetSwapInterval(-1))
        {
            spdlog::info("Adaptive VSync enabled");
            success = true;
        }
        else if (SDL_GL_SetSwapInterval(1))
        {
            spdlog::info("Standard VSync enabled (adaptive unavailable: {})", SDL_GetError());
            success = true;
        }
    }
    else if (SDL_GL_SetSwapInterval(0))
    {
        spdlog::info("VSync disabled");
        success = true;
    }

    if (!success)
    {
        spdlog::warn("Failed to apply VSync setting (enabled={}): {}", enabled, SDL_GetError());
    }

    return success;
}

GLuint RenderBackendGL::programHandle(ProgramId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= programs_.size())
        return 0;

    return programs_[idx];
}

GLuint RenderBackendGL::bufferHandle(BufferId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= buffers_.size())
        return 0;

    return buffers_[idx];
}

GLuint RenderBackendGL::framebufferHandle(FramebufferId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= framebuffers_.size())
        return 0;

    return framebuffers_[idx];
}

std::string RenderBackendGL::readTextFile(const std::string& path)
{
    spdlog::debug("Loading file {}", path);

    std::string fileContents;
    std::ifstream stream(path, std::ios::in);
    if (!stream.is_open())
    {
        return std::string();
    }

    std::string line;
    while (getline(stream, line))
    {
        fileContents += line + "\n";
    }

    return fileContents;
}

GLuint RenderBackendGL::createProgramFromSources(
    const std::string& vertexShaderCode,
    const std::string& fragmentShaderCode,
    std::string_view vertexShaderDebugName,
    std::string_view fragmentShaderDebugName)
{
    if (vertexShaderCode.empty() || fragmentShaderCode.empty())
        return 0;

    spdlog::debug("Compiling the vx shader {}", vertexShaderDebugName);
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(vertexShaderID, vertexShaderCode))
        return 0;

    spdlog::debug("Compiling the fg shader {}", fragmentShaderDebugName);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragmentShaderID, fragmentShaderCode))
        return 0;

    spdlog::debug("Linking the shader program");
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    GLint result = GL_FALSE;
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        std::string errorMessage;
        int infoLogLength{};
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 2)
        {
            errorMessage.resize(static_cast<std::size_t>(infoLogLength));
            glGetProgramInfoLog(programID, infoLogLength, nullptr, &errorMessage[0]);
        }
        spdlog::error("Shader program link error: {}", errorMessage);
    }

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    if (result == GL_FALSE)
    {
        glDeleteProgram(programID);
        return 0;
    }

    return programID;
}

ProgramId RenderBackendGL::createProgramFromFiles(
    std::string_view vertexShaderPath,
    std::string_view fragmentShaderPath,
    std::string_view vertexShaderDebugName,
    std::string_view fragmentShaderDebugName)
{
    const std::string vertexCode = readTextFile(std::string(vertexShaderPath));
    if (vertexCode.empty())
    {
        spdlog::error("Unable to read the vertex shader file {}", vertexShaderPath);
        return 0;
    }

    const std::string fragmentCode = readTextFile(std::string(fragmentShaderPath));
    if (fragmentCode.empty())
    {
        spdlog::error("Unable to read the fragment shader file {}", fragmentShaderPath);
        return 0;
    }

    const GLuint program
        = createProgramFromSources(vertexCode, fragmentCode, vertexShaderDebugName, fragmentShaderDebugName);
    if (program == 0)
        return 0;

    programs_.push_back(program);
    return static_cast<ProgramId>(programs_.size() - 1);
}

void RenderBackendGL::useProgram(ProgramId id)
{
    const GLuint handle = programHandle(id);
    if (handle == stateCache_.currentProgram_)
        return;
    stateCache_.currentProgram_ = handle;
    glUseProgram(handle);
}

UniformLocationId RenderBackendGL::uniformLocation(ProgramId id, std::string_view name) const
{
    return UniformLocationId(glGetUniformLocation(programHandle(id), std::string(name).c_str()));
}

AttributeLocationId RenderBackendGL::attribLocation(ProgramId id, std::string_view name) const
{
    return AttributeLocationId(glGetAttribLocation(programHandle(id), std::string(name).c_str()));
}

void RenderBackendGL::releaseProgram(ProgramId id)
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

PipelineId RenderBackendGL::createPipeline(const PipelineDesc& desc)
{
    // Resolve logical shader names to backend-specific file paths
    static constexpr const char* shadersDir = "data/shaders/120/";
    static constexpr const char* vertexExt = ".vxgls";
    static constexpr const char* fragmentExt = ".fggls";

    const std::string vertexShaderFile = shadersDir + desc.vertexShader + vertexExt;
    const std::string fragmentShaderFile = shadersDir + desc.fragmentShader + fragmentExt;

    const ProgramId programId = createProgramFromFiles(
        vertexShaderFile, fragmentShaderFile, desc.vertexShader, desc.fragmentShader);
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

    pipelines_.push_back(std::move(pipeline));
    return static_cast<PipelineId>(pipelines_.size());
}

void RenderBackendGL::releasePipeline(PipelineId id)
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

UniformLocationId RenderBackendGL::pipelineUniformLocation(PipelineId id, std::string_view name) const
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

AttributeLocationId RenderBackendGL::pipelineAttribLocation(PipelineId id, std::string_view name) const
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

RenderPassId RenderBackendGL::createRenderPass(const RenderPassDesc& desc)
{
    RenderPass pass;
    pass.alive = true;
    pass.desc = desc;

    renderPasses_.push_back(std::move(pass));
    return static_cast<RenderPassId>(renderPasses_.size());
}

void RenderBackendGL::releaseRenderPass(RenderPassId id)
{
    if (id == 0 || id > renderPasses_.size())
        return;

    renderPasses_[id - 1].alive = false;
}

BufferId RenderBackendGL::createBuffer()
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    if (buffer == 0)
        return 0;

    buffers_.push_back(buffer);
    return static_cast<BufferId>(buffers_.size() - 1);
}

void RenderBackendGL::bindBuffer(BufferTarget target, BufferId id)
{
    const GLuint buffer = bufferHandle(id);
    const GLenum glTarget = (target == BufferTarget::Array) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    glBindBuffer(glTarget, buffer);
}

void RenderBackendGL::bufferData(
    BufferTarget target, BufferId id, std::size_t sizeBytes, const void* data, BufferUsage usage)
{
    bindBuffer(target, id);

    const GLenum glTarget = (target == BufferTarget::Array) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    const GLenum glUsage = (usage == BufferUsage::StreamDraw) ? GL_STREAM_DRAW : GL_STREAM_DRAW;
    glBufferData(glTarget, static_cast<GLsizeiptr>(sizeBytes), data, glUsage);
}

void RenderBackendGL::releaseBuffer(BufferId id)
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
}

FramebufferId RenderBackendGL::createFramebuffer()
{
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    if (framebuffer == 0)
        return 0;

    framebuffers_.push_back(framebuffer);
    return static_cast<FramebufferId>(framebuffers_.size() - 1);
}

void RenderBackendGL::bindFramebuffer(FramebufferId id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle(id));
}

void RenderBackendGL::framebufferAttachDepthTexture(FramebufferId fbo, BackendTextureHandle depthTexture)
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

void RenderBackendGL::endRenderToTexture()
{
    if (!framebufferStack_.empty())
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    }
    popFramebuffer();
}

void RenderBackendGL::pushFramebuffer()
{
    GLint current = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current);
    framebufferStack_.push_back(static_cast<GLuint>(current));
}

void RenderBackendGL::popFramebuffer()
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

void RenderBackendGL::readPixelsFloat(int x, int y, int width, int height, float* rgba)
{
    glReadPixels(x, y, width, height, GL_RGBA, GL_FLOAT, rgba);
}

void RenderBackendGL::readPixelsUByte(int x, int y, int width, int height, unsigned char* rgba)
{
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

void RenderBackendGL::releaseFramebuffer(FramebufferId id)
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


BackendCommandBufferHandle RenderBackendGL::createCommandBuffer()
{
    if (commandBuffers_.empty())
    {
        commandBuffers_.push_back(CommandBuffer{});
    }

    for (std::size_t idx = 1; idx < commandBuffers_.size(); ++idx)
    {
        CommandBuffer& buffer = commandBuffers_[idx];
        if (!buffer.alive)
        {
            buffer.alive = true;
            buffer.recording = false;
            buffer.commands.clear();
            return BackendCommandBufferHandle(static_cast<std::uint32_t>(idx));
        }
    }

    commandBuffers_.push_back(CommandBuffer{});
    CommandBuffer& buffer = commandBuffers_.back();
    buffer.alive = true;
    buffer.recording = false;
    buffer.commands.clear();
    return BackendCommandBufferHandle(static_cast<std::uint32_t>(commandBuffers_.size() - 1));
}

void RenderBackendGL::destroyCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr)
        return;

    buffer->alive = false;
    buffer->recording = false;
    buffer->commands.clear();
}

void RenderBackendGL::beginCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr)
        return;

    buffer->recording = true;
    buffer->commands.clear();
}

void RenderBackendGL::recordCommand(BackendCommandBufferHandle handle, BackendCommand&& command)
{
    if (!handle.isValid())
        return;

    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr || !buffer->recording)
        return;

    if constexpr (ImmediateExecution)
        executeCommand(command);
    else
        buffer->commands.push_back(std::move(command));
}

void RenderBackendGL::endCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr || !buffer->recording)
        return;

    buffer->recording = false;
}

void RenderBackendGL::submitCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr || buffer->recording)
        return;

    for (const BackendCommand& command : buffer->commands)
    {
        std::visit([this](const auto& cmd) { executeCommand(cmd); }, command);
    }

    buffer->commands.clear();

    // Only flush pending deletes when no other buffer is still recording,
    // to avoid deleting resources that queued commands may still reference.
    if (activeCommandBufferCount() == 0)
        flushPendingDeletes();
}

void RenderBackendGL::flushPendingDeletes()
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

std::size_t RenderBackendGL::activeCommandBufferCount() const
{
    std::size_t count{};
    for (const auto& buffer : commandBuffers_)
    {
        if (buffer.alive && buffer.recording)
            ++count;
    }
    return count;
}

RenderBackendGL::CommandBuffer* RenderBackendGL::commandBufferFromHandle(BackendCommandBufferHandle handle)
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

const RenderBackendGL::CommandBuffer* RenderBackendGL::commandBufferFromHandle(BackendCommandBufferHandle handle) const
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

void RenderBackendGL::executeCommand(const BackendCommand& command)
{
    std::visit([this](const auto& cmd) { executeCommand(cmd); }, command);
}

void RenderBackendGL::executeCommand(const BackendCommandClear& command)
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

void RenderBackendGL::executeCommand(const BackendCommandSetViewport& command)
{
    glViewport(command.x, command.y, command.width, command.height);
}

void RenderBackendGL::executeCommand(const BackendCommandSetMultisample& command)
{
    if (command.enabled)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}

void RenderBackendGL::executeCommand(const BackendCommandDraw& command)
{
    const GLenum mode = toDrawMode(command.topology);
    glDrawArrays(mode, command.first, command.count);
}

void RenderBackendGL::executeCommand(const BackendCommandDrawIndexed& command)
{
    const GLenum mode = toDrawMode(command.topology);
    const GLenum indexType = toIndexType(command.indexType);
    glDrawElements(mode, command.count, indexType, reinterpret_cast<const void*>(command.indexBufferOffset));
}

void RenderBackendGL::executeCommand(const BackendCommandSetBlendState& command)
{
    if (command.enabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(toBlendFactor(command.srcFactor), toBlendFactor(command.dstFactor));
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void RenderBackendGL::executeCommand(const BackendCommandSetCullFace& command)
{
    if (command.enabled)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

void RenderBackendGL::executeCommand(const BackendCommandSetCullFaceMode& command)
{
    glCullFace(command.mode == BackendCullFaceMode::Front ? GL_FRONT : GL_BACK);
}

void RenderBackendGL::executeCommand(const BackendCommandSetPolygonOffsetFill& command)
{
    if (command.enabled)
        glEnable(GL_POLYGON_OFFSET_FILL);
    else
        glDisable(GL_POLYGON_OFFSET_FILL);
}

void RenderBackendGL::executeCommand(const BackendCommandSetPolygonOffset& command)
{
    glPolygonOffset(command.factor, command.units);
}

void RenderBackendGL::executeCommand(const BackendCommandSetAlphaTest& command)
{
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

void RenderBackendGL::executeCommand(const BackendCommandSetDepthMask& command)
{
    glDepthMask(command.writable ? GL_TRUE : GL_FALSE);
}

void RenderBackendGL::executeCommand(const BackendCommandSetDepthFunc& command)
{
    glDepthFunc(toDepthFunc(command.function));
}

void RenderBackendGL::executeCommand(const BackendCommandSetDepthTest& command)
{
    if (command.enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void RenderBackendGL::executeCommand(const BackendCommandSetUniform1i& command)
{
    if (!command.location.isValid())
        return;

    glUniform1i(command.location.value(), command.value);
}

void RenderBackendGL::executeCommand(const BackendCommandSetUniform2f& command)
{
    if (!command.location.isValid())
        return;

    glUniform2f(command.location.value(), command.x, command.y);
}

void RenderBackendGL::executeCommand(const BackendCommandSetUniform1fv& command)
{
    if (!command.location.isValid())
        return;

    glUniform1fv(command.location.value(), static_cast<GLsizei>(command.values.size()), command.values.data());
}

void RenderBackendGL::executeCommand(const BackendCommandSetUniform3f& command)
{
    if (!command.location.isValid())
        return;

    glUniform3f(command.location.value(), command.x, command.y, command.z);
}

void RenderBackendGL::executeCommand(const BackendCommandSetUniform3fv& command)
{
    if (!command.location.isValid())
        return;

    glUniform3fv(command.location.value(), static_cast<GLsizei>(command.values.size() / 3), command.values.data());
}

void RenderBackendGL::executeCommand(const BackendCommandSetUniformMatrix4fv& command)
{
    if (!command.location.isValid())
        return;

    glUniformMatrix4fv(command.location.value(), 1, command.transpose ? GL_TRUE : GL_FALSE, command.values.data());
}

void RenderBackendGL::executeCommand(const BackendCommandSetVertexAttribPointer& command)
{
    if (!command.index.isValid())
        return;

    const GLuint index = static_cast<GLuint>(command.index.value());

    if (command.enabled)
    {
        if (index < MaxVertexAttribs && !stateCache_.enabledAttribs_.test(index))
        {
            stateCache_.enabledAttribs_.set(index);
            glEnableVertexAttribArray(index);
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
        if (index < MaxVertexAttribs && stateCache_.enabledAttribs_.test(index))
        {
            stateCache_.enabledAttribs_.reset(index);
            glDisableVertexAttribArray(index);
        }
    }
}

void RenderBackendGL::executeCommand(const BackendCommandSetProgram& command)
{
    useProgram(command.programId);
}

void RenderBackendGL::executeCommand(const BackendCommandBindPipeline& command)
{
    const PipelineId id = command.pipelineId;
    if (id == 0 || id > pipelines_.size())
        return;

    const Pipeline& pipeline = pipelines_[id - 1];
    if (!pipeline.alive)
        return;

    useProgram(pipeline.programId);
}

void RenderBackendGL::executeCommand(const BackendCommandBindTexture2D& command)
{
    const GLuint textureHandle = command.textureHandle.isValid() ? command.textureHandle.value() : fallbackTexture2D_;
    const GLenum minF = toFilter(command.minFilter);
    const GLenum magF = toFilter(command.magFilter);
    const int unit = static_cast<int>(command.unit);

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

void RenderBackendGL::executeCommand(const BackendCommandBufferData& command)
{
    bufferData(command.target, command.bufferId, command.data.size(), command.data.data(), command.usage);
}

void RenderBackendGL::executeCommand(const BackendCommandBindBuffer& command)
{
    bindBuffer(command.target, command.bufferId);
}

void RenderBackendGL::executeCommand(const BackendCommandBeginRenderToTexture& command)
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
        popFramebuffer();
    }
}

void RenderBackendGL::executeCommand(const BackendCommandBeginRenderPass& command)
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
        glDepthMask(GL_TRUE);
        clearMask |= GL_DEPTH_BUFFER_BIT;
    }

    if (clearMask != 0)
        glClear(clearMask);
}

void RenderBackendGL::executeCommand(const BackendCommandEndRenderPass& /*command*/)
{
    // In GL 2.1, ending a render pass is a no-op.
    // In Vulkan, this would end the VkRenderPass.
}

void RenderBackendGL::executeCommand(const BackendCommandBindDefaultFramebuffer& /*command*/)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderBackendGL::executeCommand(const BackendCommandEndRenderToTexture& /*command*/)
{
    endRenderToTexture();
}

void RenderBackendGL::executeCommand(const BackendCommandSetLineWidth& command)
{
    glLineWidth(command.width);
}

BackendTextureHandle RenderBackendGL::createTexture2D()
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    return BackendTextureHandle(texture);
}

void RenderBackendGL::destroyTexture2D(BackendTextureHandle handle)
{
    if (!handle.isValid())
        return;

    pendingTextureDeletes_.push_back(handle.value());
}

void RenderBackendGL::textureStorage2D(BackendTextureHandle handle, int width, int height, TextureFormat format)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexImage2D(
        GL_TEXTURE_2D, 0, toStorageFormat(format), width, height, 0, toPixelFormat(format),
        toPixelDataType(format), nullptr);
}

void RenderBackendGL::textureSubImage2D(
    BackendTextureHandle handle, int x, int y, int width, int height, TextureFormat format, const void* pixels)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, toPixelFormat(format), GL_UNSIGNED_BYTE, pixels);
}

void RenderBackendGL::textureSetMinMagFilter(
    BackendTextureHandle handle, TextureFilter minFilter, TextureFilter magFilter)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toFilter(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toFilter(magFilter));
}

void RenderBackendGL::textureSetWrap(BackendTextureHandle handle, TextureWrap wrapS, TextureWrap wrapT)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toWrap(wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toWrap(wrapT));
}

void RenderBackendGL::textureGenerateMipmap(BackendTextureHandle handle)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glGenerateMipmap(GL_TEXTURE_2D);
}

} // namespace OpenGL

} // namespace Ren
