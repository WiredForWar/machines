#include "render/OpenGL/RenderBackendGL.hpp"

#include "spdlog/spdlog.h"

#include <fstream>

namespace Ren::OpenGL
{

RenderBackendGL::RenderBackendGL()
    : programs_{0,}
{
}

bool RenderBackendGL::initialize()
{
    if (initialized_)
        return false;

    initialized_ = true;
    return true;
}

void RenderBackendGL::shutdown()
{
    initialized_ = false;
}

bool RenderBackendGL::isInitialized() const
{
    return initialized_;
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

} // namespace Ren::OpenGL
