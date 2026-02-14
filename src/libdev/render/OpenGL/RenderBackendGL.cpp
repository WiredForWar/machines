#include "render/OpenGL/RenderBackendGL.hpp"

#include "render/internal/surfmgri.hpp"
#include "render/internal/surfbody.hpp"
#include "render/surfmgr.hpp"

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

namespace OpenGL
{

namespace
{

GLenum toStorageFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::RGBA8_UNorm:
        return GL_RGBA8;
    }
    return GL_RGBA8;
}

GLenum toPixelFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::RGBA8_UNorm:
        return GL_RGBA;
    }
    return GL_RGBA;
}

GLenum toFilter(TextureFilter filter)
{
    switch (filter)
    {
    case TextureFilter::Nearest:
        return GL_NEAREST;
    case TextureFilter::Linear:
        return GL_LINEAR;
    case TextureFilter::LinearMipmapLinear:
        return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_NEAREST;
}

GLenum toWrap(TextureWrap wrap)
{
    switch (wrap)
    {
    case TextureWrap::Repeat:
        return GL_REPEAT;
    case TextureWrap::ClampToEdge:
        return GL_CLAMP_TO_EDGE;
    }
    return GL_REPEAT;
}

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
    const GLenum glew_status = glewInit();
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

    initialized_ = true;
    return true;
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
        SDL_GL_DeleteContext(glContext_);
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
    glUseProgram(programHandle(id));
}

int RenderBackendGL::uniformLocation(ProgramId id, std::string_view name) const
{
    return glGetUniformLocation(programHandle(id), std::string(name).c_str());
}

int RenderBackendGL::attribLocation(ProgramId id, std::string_view name) const
{
    return glGetAttribLocation(programHandle(id), std::string(name).c_str());
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

void RenderBackendGL::framebufferTexture2D(FramebufferAttachment attachment, TexId texture)
{
    const GLenum glAttachment
        = (attachment == FramebufferAttachment::Color0) ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT0;

    GLuint textureHandle = 0;
    if (texture != NullTexId)
    {
        RenISurfBody* surfBody = RenSurfaceManager::instance().impl().getSurface(texture);
        if (surfBody && surfBody->nativeTexture2D_.isValid())
        {
            textureHandle = surfBody->nativeTexture2D_.value();
        }
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, glAttachment, GL_TEXTURE_2D, textureHandle, 0);
}

bool RenderBackendGL::beginRenderToTexture(FramebufferId framebuffer, TexId targetTexture)
{
    if (framebuffer == 0)
        return false;

    if (targetTexture == NullTexId)
        return false;

    pushFramebuffer();
    bindFramebuffer(framebuffer);
    framebufferTexture2D(FramebufferAttachment::Color0, targetTexture);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Framebuffer incomplete (status=0x{:X})", static_cast<unsigned int>(status));
        framebufferTexture2D(FramebufferAttachment::Color0, NullTexId);
        popFramebuffer();
        return false;
    }

    return true;
}

void RenderBackendGL::endRenderToTexture()
{
    if (!framebufferStack_.empty())
    {
        framebufferTexture2D(FramebufferAttachment::Color0, NullTexId);
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

void RenderBackendGL::bindTexture2D(TexId id, std::uint32_t unit)
{
    const RenISurfBody* surfBody = RenSurfaceManager::instance().impl().getSurface(id);
    GLuint textureHandle = fallbackTexture2D_;

    if (surfBody && !surfBody->isEmpty() && surfBody->nativeTexture2D_.isValid())
    {
        textureHandle = surfBody->nativeTexture2D_.value();
    }

    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + unit));
    glBindTexture(GL_TEXTURE_2D, textureHandle);
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

void RenderBackendGL::executeCommand(const BackendCommandSetViewport& command)
{
    glViewport(command.x, command.y, command.width, command.height);
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
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexImage2D(
        GL_TEXTURE_2D, 0, toStorageFormat(format), width, height, 0, toPixelFormat(format), GL_UNSIGNED_BYTE, nullptr);
}

void RenderBackendGL::textureSubImage2D(
    BackendTextureHandle handle, int x, int y, int width, int height, TextureFormat format, const void* pixels)
{
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, toPixelFormat(format), GL_UNSIGNED_BYTE, pixels);
}

void RenderBackendGL::textureSetMinMagFilter(
    BackendTextureHandle handle, TextureFilter minFilter, TextureFilter magFilter)
{
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toFilter(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toFilter(magFilter));
}

void RenderBackendGL::textureSetWrap(BackendTextureHandle handle, TextureWrap wrapS, TextureWrap wrapT)
{
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toWrap(wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toWrap(wrapT));
}

void RenderBackendGL::textureGenerateMipmap(BackendTextureHandle handle)
{
    glBindTexture(GL_TEXTURE_2D, handle.value());
    glGenerateMipmap(GL_TEXTURE_2D);
}

} // namespace OpenGL

} // namespace Ren
