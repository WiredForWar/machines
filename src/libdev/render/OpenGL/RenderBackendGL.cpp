#include "render/OpenGL/RenderBackendGL.hpp"

#include "spdlog/spdlog.h"

#include <SDL.h>

#include <fstream>

namespace Ren::OpenGL
{

RenderBackendGL::RenderBackendGL()
    : programs_{0,}
    , buffers_{0,}
    , framebuffers_{0,}
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

    const auto getGlStringAsConstChar = [](GLenum name) {
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

    initialized_ = true;
    return true;
}

void RenderBackendGL::shutdown()
{
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

GLuint RenderBackendGL::programHandle(Ren::ProgramId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= programs_.size())
        return 0;

    return programs_[idx];
}

GLuint RenderBackendGL::bufferHandle(Ren::BufferId id) const
{
    if (id == 0)
        return 0;

    const std::size_t idx = static_cast<std::size_t>(id);
    if (idx >= buffers_.size())
        return 0;

    return buffers_[idx];
}

GLuint RenderBackendGL::framebufferHandle(Ren::FramebufferId id) const
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

static bool compileShader(GLuint shaderID, const std::string& code)
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

Ren::ProgramId RenderBackendGL::createProgramFromFiles(
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

    const GLuint program = createProgramFromSources(vertexCode, fragmentCode, vertexShaderDebugName, fragmentShaderDebugName);
    if (program == 0)
        return 0;

    programs_.push_back(program);
    return static_cast<Ren::ProgramId>(programs_.size() - 1);
}

void RenderBackendGL::useProgram(Ren::ProgramId id)
{
    glUseProgram(programHandle(id));
}

int RenderBackendGL::uniformLocation(Ren::ProgramId id, std::string_view name) const
{
    return glGetUniformLocation(programHandle(id), std::string(name).c_str());
}

int RenderBackendGL::attribLocation(Ren::ProgramId id, std::string_view name) const
{
    return glGetAttribLocation(programHandle(id), std::string(name).c_str());
}

void RenderBackendGL::releaseProgram(Ren::ProgramId id)
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

Ren::BufferId RenderBackendGL::createBuffer()
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    if (buffer == 0)
        return 0;

    buffers_.push_back(buffer);
    return static_cast<Ren::BufferId>(buffers_.size() - 1);
}

void RenderBackendGL::bindBuffer(RenBufferTarget target, Ren::BufferId id)
{
    const GLuint buffer = bufferHandle(id);
    const GLenum glTarget = (target == RenBufferTarget::Array) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    glBindBuffer(glTarget, buffer);
}

void RenderBackendGL::bufferData(
    RenBufferTarget target, Ren::BufferId id, std::size_t sizeBytes, const void* data, RenBufferUsage usage)
{
    bindBuffer(target, id);

    const GLenum glTarget = (target == RenBufferTarget::Array) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    const GLenum glUsage = (usage == RenBufferUsage::StreamDraw) ? GL_STREAM_DRAW : GL_STREAM_DRAW;
    glBufferData(glTarget, static_cast<GLsizeiptr>(sizeBytes), data, glUsage);
}

void RenderBackendGL::releaseBuffer(Ren::BufferId id)
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

Ren::FramebufferId RenderBackendGL::createFramebuffer()
{
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    if (framebuffer == 0)
        return 0;

    framebuffers_.push_back(framebuffer);
    return static_cast<Ren::FramebufferId>(framebuffers_.size() - 1);
}

void RenderBackendGL::bindFramebuffer(Ren::FramebufferId id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle(id));
}

void RenderBackendGL::framebufferTexture2D(RenFramebufferAttachment attachment, std::uint32_t textureHandle)
{
    const GLenum glAttachment = (attachment == RenFramebufferAttachment::Color0) ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT0;
    glFramebufferTexture2D(GL_FRAMEBUFFER, glAttachment, GL_TEXTURE_2D, static_cast<GLuint>(textureHandle), 0);
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

void RenderBackendGL::releaseFramebuffer(Ren::FramebufferId id)
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

} // namespace Ren::OpenGL
