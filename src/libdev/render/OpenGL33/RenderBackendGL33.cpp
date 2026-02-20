#include "render/OpenGL33/RenderBackendGL33.hpp"
#include "render/OpenGL/Utils.hpp"

#include "base/prepost.hpp"

#include "spdlog/spdlog.h"

#include <SDL.h>

#include <fstream>
#include <variant>

// Immediate mode is more performant, at least for the legacy renderer.
// Yet we want to have this switch to debug commands buffer implementation.
constexpr bool ImmediateExecution{true};

namespace Ren
{

namespace OpenGL33
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

RenderBackendGL33::RenderBackendGL33()
    : programs_{0,}
    , buffers_{0,}
    , framebuffers_{0,}
    , commandBuffers_{CommandBuffer{},}
{
}

BackendType RenderBackendGL33::backendType() const
{
    return BackendType::GL33;
}

bool RenderBackendGL33::initialize(SDL_Window* window)
{
    if (initialized_)
        return false;

    if (window == nullptr)
        return false;

    window_ = window;

    constexpr int contextMajorVersion = 3;
    constexpr int contextMinorVersion = 3;

    spdlog::info("Requesting GL {}.{} core profile context", contextMajorVersion, contextMinorVersion);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, contextMajorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, contextMinorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    glContext_ = SDL_GL_CreateContext(window_);
    if (glContext_ == nullptr)
    {
        spdlog::error("Failed to create GL 3.3 core context: {}", SDL_GetError());
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
    glewExperimental = GL_TRUE;
    const GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK)
    {
        spdlog::error("Fatal in glewInit: {}", reinterpret_cast<const char*>(glewGetErrorString(glew_status)));
        shutdown();
        return false;
    }

    // glewInit on core profile may generate a spurious GL_INVALID_ENUM — drain it.
    while (glGetError() != GL_NO_ERROR)
    {
    }

    if (!GLEW_VERSION_3_3)
    {
        spdlog::error("GLEW reports that OpenGL 3.3 is not available");
        shutdown();
        return false;
    }

    // Core profile requires a VAO to be bound before any vertex operations.
    glGenVertexArrays(1, &defaultVAO_);
    if (defaultVAO_ == 0)
    {
        spdlog::error("Failed to create default VAO");
        shutdown();
        return false;
    }
    glBindVertexArray(defaultVAO_);

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    stateCache_.reset();

    initialized_ = true;
    return true;
}

void RenderBackendGL33::StateCache::reset()
{
    currentProgram_ = 0;
    enabledAttribs_.reset();

    resetTextureUnits();

    depthTestEnabled_.reset();
    depthMaskWritable_.reset();
    depthFunc_.reset();
    blend_.reset();
    cullFaceEnabled_.reset();
    cullFaceMode_.reset();
    alphaTest_.reset();
    polygonOffset_.reset();
    boundArrayBuffer_ = 0;
    boundElementBuffer_ = 0;
    viewport_.reset();
}

void RenderBackendGL33::StateCache::resetTextureUnits()
{
    textureUnits_ = {};
}

void RenderBackendGL33::shutdown()
{
    flushPendingDeletes();

    if (fallbackTexture2D_ != 0)
    {
        glDeleteTextures(1, &fallbackTexture2D_);
        fallbackTexture2D_ = 0;
    }

    if (defaultVAO_ != 0)
    {
        glDeleteVertexArrays(1, &defaultVAO_);
        defaultVAO_ = 0;
    }

    initialized_ = false;

    if (glContext_ != nullptr)
    {
        SDL_GL_MakeCurrent(nullptr, nullptr);
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }

    window_ = nullptr;
}

bool RenderBackendGL33::isInitialized() const
{
    return initialized_;
}

bool RenderBackendGL33::setVSync(bool enabled)
{
    if (!isInitialized())
    {
        spdlog::warn("Cannot set VSync: backend not initialised yet");
        return false;
    }

    bool success{};

    if (enabled)
    {
        if (SDL_GL_SetSwapInterval(-1) == 0)
        {
            spdlog::info("Adaptive VSync enabled");
            success = true;
        }
        else if (SDL_GL_SetSwapInterval(1) == 0)
        {
            spdlog::info("Standard VSync enabled (adaptive unavailable: {})", SDL_GetError());
            success = true;
        }
    }
    else if (SDL_GL_SetSwapInterval(0) == 0)
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

GLuint RenderBackendGL33::programHandle(ProgramId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= programs_.size())
        return 0;

    return programs_[idx];
}

GLuint RenderBackendGL33::bufferHandle(BufferId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= buffers_.size())
        return 0;

    return buffers_[idx];
}

GLuint RenderBackendGL33::framebufferHandle(FramebufferId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= framebuffers_.size())
        return 0;

    return framebuffers_[idx];
}

std::string RenderBackendGL33::readTextFile(const std::string& path)
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

GLuint RenderBackendGL33::createProgramFromSources(
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

ProgramId RenderBackendGL33::createProgramFromFiles(
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

void RenderBackendGL33::useProgram(ProgramId id)
{
    const GLuint handle = programHandle(id);
    if (handle == stateCache_.currentProgram_)
        return;
    stateCache_.currentProgram_ = handle;
    glUseProgram(handle);
}

UniformLocationId RenderBackendGL33::uniformLocation(ProgramId id, std::string_view name) const
{
    return UniformLocationId(glGetUniformLocation(programHandle(id), std::string(name).c_str()));
}

AttributeLocationId RenderBackendGL33::attribLocation(ProgramId id, std::string_view name) const
{
    return AttributeLocationId(glGetAttribLocation(programHandle(id), std::string(name).c_str()));
}

void RenderBackendGL33::releaseProgram(ProgramId id)
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

PipelineId RenderBackendGL33::createPipeline(const PipelineDesc& desc)
{
    // Resolve logical shader names to GL 3.3 shader directory
    static constexpr const char* shadersDir = "data/shaders/330/";
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

void RenderBackendGL33::releasePipeline(PipelineId id)
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

UniformLocationId RenderBackendGL33::pipelineUniformLocation(PipelineId id, std::string_view name) const
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

AttributeLocationId RenderBackendGL33::pipelineAttribLocation(PipelineId id, std::string_view name) const
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

RenderPassId RenderBackendGL33::createRenderPass(const RenderPassDesc& desc)
{
    RenderPass pass;
    pass.alive = true;
    pass.desc = desc;

    renderPasses_.push_back(std::move(pass));
    return static_cast<RenderPassId>(renderPasses_.size());
}

void RenderBackendGL33::releaseRenderPass(RenderPassId id)
{
    if (id == 0 || id > renderPasses_.size())
        return;

    renderPasses_[id - 1].alive = false;
}

BufferId RenderBackendGL33::createBuffer()
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    if (buffer == 0)
        return 0;

    buffers_.push_back(buffer);
    return static_cast<BufferId>(buffers_.size() - 1);
}

void RenderBackendGL33::bindBuffer(BufferTarget target, BufferId id)
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

void RenderBackendGL33::bufferData(
    BufferTarget target, BufferId id, std::size_t sizeBytes, const void* data, BufferUsage usage)
{
    bindBuffer(target, id);

    const GLenum glTarget = (target == BufferTarget::Array) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    const GLenum glUsage = (usage == BufferUsage::StreamDraw) ? GL_STREAM_DRAW : GL_STREAM_DRAW;
    glBufferData(glTarget, static_cast<GLsizeiptr>(sizeBytes), data, glUsage);
}

void RenderBackendGL33::releaseBuffer(BufferId id)
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

FramebufferId RenderBackendGL33::createFramebuffer()
{
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    if (framebuffer == 0)
        return 0;

    framebuffers_.push_back(framebuffer);
    return static_cast<FramebufferId>(framebuffers_.size() - 1);
}

void RenderBackendGL33::bindFramebuffer(FramebufferId id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle(id));
}

void RenderBackendGL33::framebufferAttachDepthTexture(FramebufferId fbo, BackendTextureHandle depthTexture)
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

void RenderBackendGL33::framebufferAttachColorTexture(FramebufferId fbo, BackendTextureHandle colorTexture)
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

void RenderBackendGL33::framebufferAttachDepthRenderbuffer(FramebufferId fbo, int width, int height)
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

bool RenderBackendGL33::isFramebufferComplete(FramebufferId fbo)
{
    const GLuint fboHandle = framebufferHandle(fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return status == GL_FRAMEBUFFER_COMPLETE;
}

void RenderBackendGL33::endRenderToTexture()
{
    if (!framebufferStack_.empty())
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    }
    currentFboColorAttachment_ = 0;
    popFramebuffer();
}

void RenderBackendGL33::pushFramebuffer()
{
    GLint current = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current);
    framebufferStack_.push_back(static_cast<GLuint>(current));
}

void RenderBackendGL33::popFramebuffer()
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

Viewport RenderBackendGL33::getViewport() const
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    return {vp[0], vp[1], vp[2], vp[3]};
}

void RenderBackendGL33::clearDisplay(int width, int height)
{
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RenderBackendGL33::readPixelsFloat(int x, int y, int width, int height, float* rgba)
{
    glReadPixels(x, y, width, height, GL_RGBA, GL_FLOAT, rgba);
}

void RenderBackendGL33::readPixelsUByte(int x, int y, int width, int height, unsigned char* rgba)
{
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

void RenderBackendGL33::releaseFramebuffer(FramebufferId id)
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


BackendCommandBufferHandle RenderBackendGL33::createCommandBuffer()
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

void RenderBackendGL33::destroyCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr)
        return;

    buffer->alive = false;
    buffer->recording = false;
    buffer->commands.clear();
}

void RenderBackendGL33::beginCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr)
        return;

    buffer->recording = true;
    buffer->commands.clear();
}

void RenderBackendGL33::recordCommand(BackendCommandBufferHandle handle, BackendCommand&& command)
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

void RenderBackendGL33::endCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr || !buffer->recording)
        return;

    buffer->recording = false;
}

void RenderBackendGL33::submitCommandBuffer(BackendCommandBufferHandle handle)
{
    CommandBuffer* buffer = commandBufferFromHandle(handle);
    if (buffer == nullptr || buffer->recording)
        return;

    for (const BackendCommand& command : buffer->commands)
    {
        std::visit([this](const auto& cmd) { executeCommand(cmd); }, command);
    }

    buffer->commands.clear();

    if (activeCommandBufferCount() == 0)
        flushPendingDeletes();
}

void RenderBackendGL33::flushPendingDeletes()
{
    PRE(activeCommandBufferCount() == 0);

    if (!pendingTextureDeletes_.empty())
    {
        glDeleteTextures(
            static_cast<GLsizei>(pendingTextureDeletes_.size()),
            pendingTextureDeletes_.data());
        pendingTextureDeletes_.clear();

        stateCache_.textureUnits_ = {};
    }
}

std::size_t RenderBackendGL33::activeCommandBufferCount() const
{
    std::size_t count{};
    for (const auto& buffer : commandBuffers_)
    {
        if (buffer.alive && buffer.recording)
            ++count;
    }
    return count;
}

RenderBackendGL33::CommandBuffer* RenderBackendGL33::commandBufferFromHandle(BackendCommandBufferHandle handle)
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

const RenderBackendGL33::CommandBuffer* RenderBackendGL33::commandBufferFromHandle(BackendCommandBufferHandle handle) const
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

void RenderBackendGL33::executeCommand(const BackendCommand& command)
{
    std::visit([this](const auto& cmd) { executeCommand(cmd); }, command);
}

void RenderBackendGL33::executeCommand(const BackendCommandClear& command)
{
    const GLbitfield mask = OpenGL::toClearMask(command.mask);
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

void RenderBackendGL33::executeCommand(const BackendCommandSetViewport& command)
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

void RenderBackendGL33::executeCommand(const BackendCommandSetMultisample& /*command*/)
{
    // GL 3.3 core profile: GL_MULTISAMPLE is always enabled if the framebuffer
    // has multisample samples. glEnable/glDisable(GL_MULTISAMPLE) is a no-op
    // in core profile — just ignore the command.
}

void RenderBackendGL33::executeCommand(const BackendCommandDraw& command)
{
    const GLenum mode = OpenGL::toDrawMode(command.topology);
    glDrawArrays(mode, command.first, command.count);
}

void RenderBackendGL33::executeCommand(const BackendCommandDrawIndexed& command)
{
    const GLenum mode = OpenGL::toDrawMode(command.topology);
    const GLenum indexType = OpenGL::toIndexType(command.indexType);
    glDrawElements(mode, command.count, indexType, reinterpret_cast<const void*>(command.indexBufferOffset));
}

void RenderBackendGL33::executeCommand(const BackendCommandSetBlendState& command)
{
    const GLenum src = command.enabled ? OpenGL::toBlendFactor(command.srcFactor) : 0;
    const GLenum dst = command.enabled ? OpenGL::toBlendFactor(command.dstFactor) : 0;
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

void RenderBackendGL33::executeCommand(const BackendCommandSetCullFace& command)
{
    if (stateCache_.cullFaceEnabled_ == command.enabled)
        return;
    stateCache_.cullFaceEnabled_ = command.enabled;
    if (command.enabled)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetCullFaceMode& command)
{
    const GLenum mode = command.mode == BackendCullFaceMode::Front ? GL_FRONT : GL_BACK;
    if (stateCache_.cullFaceMode_ == mode)
        return;
    stateCache_.cullFaceMode_ = mode;
    glCullFace(mode);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetPolygonOffsetFill& command)
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

void RenderBackendGL33::executeCommand(const BackendCommandSetPolygonOffset& command)
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

void RenderBackendGL33::executeCommand(const BackendCommandSetAlphaTest& command)
{
    // GL 3.3 core has no glAlphaFunc/GL_ALPHA_TEST. We track the state here
    // and the 330 fragment shaders implement alpha test via `discard`.
    // The alpha test uniform is set as part of the uniform block commands
    // (Gui2DUniforms, etc.) — see the uniform block execute methods below.
    if (stateCache_.alphaTest_.has_value())
    {
        const auto& a = *stateCache_.alphaTest_;
        if (a.enabled == command.enabled && (!command.enabled || a.reference == command.reference))
            return;
    }
    stateCache_.alphaTest_ = {command.enabled, command.reference};

    // Set the alpha test uniform on the currently bound program.
    const GLint locEnabled = glGetUniformLocation(stateCache_.currentProgram_, "uAlphaTestEnabled");
    const GLint locRef = glGetUniformLocation(stateCache_.currentProgram_, "uAlphaTestReference");
    if (locEnabled >= 0)
        glUniform1i(locEnabled, command.enabled ? 1 : 0);
    if (locRef >= 0)
        glUniform1f(locRef, command.reference);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetDepthMask& command)
{
    if (stateCache_.depthMaskWritable_ == command.writable)
        return;
    stateCache_.depthMaskWritable_ = command.writable;
    glDepthMask(command.writable ? GL_TRUE : GL_FALSE);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetDepthFunc& command)
{
    const GLenum func = OpenGL::toDepthFunc(command.function);
    if (stateCache_.depthFunc_ == func)
        return;
    stateCache_.depthFunc_ = func;
    glDepthFunc(func);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetDepthTest& command)
{
    if (stateCache_.depthTestEnabled_ == command.enabled)
        return;
    stateCache_.depthTestEnabled_ = command.enabled;
    if (command.enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetUniform1i& command)
{
    if (!command.location.isValid())
        return;

    glUniform1i(command.location.value(), command.value);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetUniform1f& command)
{
    if (!command.location.isValid())
        return;

    glUniform1f(command.location.value(), command.value);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetUniform2f& command)
{
    if (!command.location.isValid())
        return;

    glUniform2f(command.location.value(), command.x, command.y);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetUniform1fv& command)
{
    if (!command.location.isValid())
        return;

    glUniform1fv(command.location.value(), static_cast<GLsizei>(command.values.size()), command.values.data());
}

void RenderBackendGL33::executeCommand(const BackendCommandSetUniform3f& command)
{
    if (!command.location.isValid())
        return;

    glUniform3f(command.location.value(), command.x, command.y, command.z);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetUniform3fv& command)
{
    if (!command.location.isValid())
        return;

    glUniform3fv(command.location.value(), static_cast<GLsizei>(command.values.size() / 3), command.values.data());
}

void RenderBackendGL33::executeCommand(const BackendCommandSetUniformMatrix4fv& command)
{
    if (!command.location.isValid())
        return;

    glUniformMatrix4fv(command.location.value(), 1, command.transpose ? GL_TRUE : GL_FALSE, command.values.data());
}

void RenderBackendGL33::executeCommand(const BackendCommandSetVertexAttribPointer& command)
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
            OpenGL::toVertexAttribType(command.type),
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

void RenderBackendGL33::executeCommand(const BackendCommandSetProgram& command)
{
    useProgram(command.programId);
}

void RenderBackendGL33::executeCommand(const BackendCommandBindPipeline& command)
{
    const PipelineId id = command.pipelineId;
    if (id == 0 || id > pipelines_.size())
        return;

    const Pipeline& pipeline = pipelines_[id - 1];
    if (!pipeline.alive)
        return;

    useProgram(pipeline.programId);
}

void RenderBackendGL33::executeCommand(const BackendCommandBindTexture2D& command)
{
    const GLuint textureHandle = command.textureHandle.isValid() ? command.textureHandle.value() : fallbackTexture2D_;
    const GLenum minF = OpenGL::toFilter(command.minFilter);
    const GLenum magF = OpenGL::toFilter(command.magFilter);
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

void RenderBackendGL33::executeCommand(const BackendCommandBufferData& command)
{
    bufferData(command.target, command.bufferId, command.data.size(), command.data.data(), command.usage);
}

void RenderBackendGL33::executeCommand(const BackendCommandBindBuffer& command)
{
    bindBuffer(command.target, command.bufferId);
}

void RenderBackendGL33::executeCommand(const BackendCommandBeginRenderToTexture& command)
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

void RenderBackendGL33::executeCommand(const BackendCommandBeginRenderPass& command)
{
    const RenderPassId id = command.renderPassId;
    if (id == 0 || id > renderPasses_.size())
        return;

    const RenderPass& pass = renderPasses_[id - 1];
    if (!pass.alive)
        return;

    if (command.framebufferId != 0)
        bindFramebuffer(command.framebufferId);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
        stateCache_.depthMaskWritable_ = true;
        glDepthMask(GL_TRUE);
        clearMask |= GL_DEPTH_BUFFER_BIT;
    }

    if (clearMask != 0)
        glClear(clearMask);
}

void RenderBackendGL33::executeCommand(const BackendCommandEndRenderPass& /*command*/)
{
}

void RenderBackendGL33::executeCommand(const BackendCommandBindDefaultFramebuffer& /*command*/)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderBackendGL33::executeCommand(const BackendCommandBindFramebuffer& command)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle(command.framebufferId));
}

void RenderBackendGL33::executeCommand(const BackendCommandEndRenderToTexture& /*command*/)
{
    endRenderToTexture();
}

void RenderBackendGL33::executeCommand(const BackendCommandSetLineWidth& command)
{
    glLineWidth(command.width);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetGui2DUniforms& command)
{
    const auto& u = command.uniforms;
    const GLint locScreenspace = glGetUniformLocation(stateCache_.currentProgram_, "uScreenspace");
    const GLint locSampler = glGetUniformLocation(stateCache_.currentProgram_, "uTextureSampler");
    if (locScreenspace >= 0)
        glUniform2f(locScreenspace, u.screenspaceX, u.screenspaceY);
    if (locSampler >= 0)
        glUniform1i(locSampler, u.textureSampler);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetStandardFrameUniforms& command)
{
    const auto& u = command.uniforms;
    const GLuint prog = stateCache_.currentProgram_;
    const GLint locV = glGetUniformLocation(prog, "uV");
    const GLint locP = glGetUniformLocation(prog, "uP");
    const GLint locFogColour = glGetUniformLocation(prog, "uFogColour");
    const GLint locFogParams = glGetUniformLocation(prog, "uFogParams");
    if (locV >= 0)
        glUniformMatrix4fv(locV, 1, GL_FALSE, u.view.data());
    if (locP >= 0)
        glUniformMatrix4fv(locP, 1, GL_FALSE, u.proj.data());
    if (locFogColour >= 0)
        glUniform3f(locFogColour, u.fogColourR, u.fogColourG, u.fogColourB);
    if (locFogParams >= 0)
        glUniform3f(locFogParams, u.fogStartOrX, u.fogEndOrY, u.fogDensityOrZ);
    const GLint locFogMode = glGetUniformLocation(prog, "uFogMode");
    if (locFogMode >= 0)
        glUniform1i(locFogMode, u.fogMode);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetStandardObjectUniforms& command)
{
    const auto& u = command.uniforms;
    const GLuint prog = stateCache_.currentProgram_;

    const GLint locM = glGetUniformLocation(prog, "uM");
    if (locM >= 0)
        glUniformMatrix4fv(locM, 1, GL_FALSE, u.model.data());

    const GLint locGpu = glGetUniformLocation(prog, "uGpuLighting");
    if (locGpu >= 0)
        glUniform1i(locGpu, u.gpuLighting);

    if (u.gpuLighting)
    {
        const GLint locLD = glGetUniformLocation(prog, "uLightDir");
        const GLint locLC = glGetUniformLocation(prog, "uLightColor");
        const GLint locAC = glGetUniformLocation(prog, "uAmbientColor");
        const GLint locMD = glGetUniformLocation(prog, "uMatDiffuse");
        const GLint locMDA = glGetUniformLocation(prog, "uMatDiffuseA");
        const GLint locMA = glGetUniformLocation(prog, "uMatAmbient");
        const GLint locME = glGetUniformLocation(prog, "uMatEmissive");
        const GLint locF = glGetUniformLocation(prog, "uFilter");
        const GLint locHV = glGetUniformLocation(prog, "uHasVtxMaterials");
        if (locLD >= 0) glUniform3f(locLD, u.lightDirX, u.lightDirY, u.lightDirZ);
        if (locLC >= 0) glUniform3f(locLC, u.lightColorR, u.lightColorG, u.lightColorB);
        if (locAC >= 0) glUniform3f(locAC, u.ambientColorR, u.ambientColorG, u.ambientColorB);
        if (locMD >= 0) glUniform3f(locMD, u.matDiffuseR, u.matDiffuseG, u.matDiffuseB);
        if (locMDA >= 0) glUniform1f(locMDA, u.matDiffuseA);
        if (locMA >= 0) glUniform3f(locMA, u.matAmbientR, u.matAmbientG, u.matAmbientB);
        if (locME >= 0) glUniform3f(locME, u.matEmissiveR, u.matEmissiveG, u.matEmissiveB);
        if (locF >= 0) glUniform3f(locF, u.filterR, u.filterG, u.filterB);
        if (locHV >= 0) glUniform1i(locHV, u.hasVtxMaterials);

        const GLint locNPL = glGetUniformLocation(prog, "uNumPointLights");
        if (locNPL >= 0) glUniform1i(locNPL, u.numPointLights);
        if (u.numPointLights > 0)
        {
            const GLint locPP = glGetUniformLocation(prog, "uPointLightPos");
            const GLint locPC = glGetUniformLocation(prog, "uPointLightColor");
            const GLint locPR = glGetUniformLocation(prog, "uPointLightRange");
            const GLint locPA = glGetUniformLocation(prog, "uPointLightAtten");
            const GLint locPO = glGetUniformLocation(prog, "uPointLightOmni");
            if (locPP >= 0 && !u.pointLightPos.empty())
                glUniform3fv(locPP, u.numPointLights, u.pointLightPos.data());
            if (locPC >= 0 && !u.pointLightColor.empty())
                glUniform3fv(locPC, u.numPointLights, u.pointLightColor.data());
            if (locPR >= 0 && !u.pointLightRange.empty())
                glUniform1fv(locPR, u.numPointLights, u.pointLightRange.data());
            if (locPA >= 0 && !u.pointLightAtten.empty())
                glUniform3fv(locPA, u.numPointLights, u.pointLightAtten.data());
            if (locPO >= 0 && !u.pointLightOmni.empty())
                glUniform1fv(locPO, u.numPointLights, u.pointLightOmni.data());
        }

        const GLint locSE = glGetUniformLocation(prog, "uShadowEnabled");
        if (locSE >= 0) glUniform1i(locSE, u.shadowEnabled);
        if (u.shadowEnabled)
        {
            const GLint locSM = glGetUniformLocation(prog, "uShadowMap");
            const GLint locLSM = glGetUniformLocation(prog, "uLightSpaceMatrix");
            const GLint locSMN = glGetUniformLocation(prog, "uShadowMapNear");
            const GLint locLSMN = glGetUniformLocation(prog, "uLightSpaceMatrixNear");
            const GLint locSD = glGetUniformLocation(prog, "uShadowSplitDistance");
            const GLint locSS = glGetUniformLocation(prog, "uShadowStrength");
            if (locSM >= 0) glUniform1i(locSM, u.shadowMapUnit);
            if (locLSM >= 0) glUniformMatrix4fv(locLSM, 1, GL_FALSE, u.lightSpaceMatrix.data());
            if (locSMN >= 0) glUniform1i(locSMN, u.shadowMapNearUnit);
            if (locLSMN >= 0) glUniformMatrix4fv(locLSMN, 1, GL_FALSE, u.lightSpaceMatrixNear.data());
            if (locSD >= 0) glUniform1f(locSD, u.shadowSplitDistance);
            if (locSS >= 0) glUniform1f(locSS, u.shadowStrength);
        }
    }

    const GLint locTS = glGetUniformLocation(prog, "uTextureSampler2");
    if (locTS >= 0)
        glUniform1i(locTS, u.textureSampler);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetBillboardUniforms& command)
{
    const auto& u = command.uniforms;
    const GLuint prog = stateCache_.currentProgram_;
    const GLint locVP = glGetUniformLocation(prog, "uVP");
    const GLint locSampler = glGetUniformLocation(prog, "uTextureSampler");
    if (locVP >= 0)
        glUniformMatrix4fv(locVP, 1, GL_FALSE, u.viewProj.data());
    if (locSampler >= 0)
        glUniform1i(locSampler, u.textureSampler);
}

void RenderBackendGL33::executeCommand(const BackendCommandSetShadowDepthUniforms& command)
{
    const auto& u = command.uniforms;
    const GLuint prog = stateCache_.currentProgram_;
    const GLint locLSM = glGetUniformLocation(prog, "uLightSpaceMatrix");
    const GLint locM = glGetUniformLocation(prog, "uM");
    if (locLSM >= 0)
        glUniformMatrix4fv(locLSM, 1, GL_FALSE, u.lightSpaceMatrix.data());
    if (locM >= 0)
        glUniformMatrix4fv(locM, 1, GL_FALSE, u.model.data());
}

void RenderBackendGL33::executeCommand(const BackendCommandSetPostProcessUniforms& command)
{
    const auto& u = command.uniforms;
    const GLuint prog = stateCache_.currentProgram_;
    const GLint locScene = glGetUniformLocation(prog, "uSceneTexture");
    const GLint locExposure = glGetUniformLocation(prog, "uExposure");
    if (locScene >= 0)
        glUniform1i(locScene, u.sceneTextureSampler);
    if (locExposure >= 0)
        glUniform1f(locExposure, u.exposure);
}

BackendTextureHandle RenderBackendGL33::createTexture2D()
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    return BackendTextureHandle(texture);
}

void RenderBackendGL33::destroyTexture2D(BackendTextureHandle handle)
{
    if (!handle.isValid())
        return;

    pendingTextureDeletes_.push_back(handle.value());
}

void RenderBackendGL33::textureStorage2D(BackendTextureHandle handle, int width, int height, TextureFormat format)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexImage2D(
        GL_TEXTURE_2D, 0, OpenGL::toStorageFormat(format), width, height, 0, OpenGL::toPixelFormat(format),
        OpenGL::toPixelDataType(format), nullptr);
}

void RenderBackendGL33::textureSubImage2D(
    BackendTextureHandle handle, int x, int y, int width, int height, TextureFormat format, const void* pixels)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, OpenGL::toPixelFormat(format), GL_UNSIGNED_BYTE, pixels);
}

void RenderBackendGL33::textureSetMinMagFilter(
    BackendTextureHandle handle, TextureFilter minFilter, TextureFilter magFilter)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGL::toFilter(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGL::toFilter(magFilter));
}

void RenderBackendGL33::textureSetWrap(BackendTextureHandle handle, TextureWrap wrapS, TextureWrap wrapT)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL::toWrap(wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL::toWrap(wrapT));
}

void RenderBackendGL33::textureGenerateMipmap(BackendTextureHandle handle)
{
    stateCache_.resetTextureUnits();
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glGenerateMipmap(GL_TEXTURE_2D);
}

} // namespace OpenGL33

} // namespace Ren
