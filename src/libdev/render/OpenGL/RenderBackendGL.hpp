#pragma once

#include "render/internal/IRenderBackend.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <SDL.h>

#include <GL/glew.h>

namespace Ren::OpenGL
{

class RenderBackendGL final : public RenIRenderBackend
{
public:
    RenderBackendGL();

    bool initialize(SDL_Window* window) override;
    void shutdown() override;

    bool isInitialized() const override;

    bool setVSync(bool enabled) override;

    Ren::ProgramId createProgramFromFiles(
        std::string_view vertexShaderPath,
        std::string_view fragmentShaderPath,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName) override;
    void releaseProgram(Ren::ProgramId id) override;

    void useProgram(Ren::ProgramId id) override;

    int uniformLocation(Ren::ProgramId id, std::string_view name) const override;
    int attribLocation(Ren::ProgramId id, std::string_view name) const override;

    Ren::BufferId createBuffer() override;
    void releaseBuffer(Ren::BufferId id) override;

    void bindBuffer(RenBufferTarget target, Ren::BufferId id) override;
    void bufferData(
        RenBufferTarget target,
        Ren::BufferId id,
        std::size_t sizeBytes,
        const void* data,
        RenBufferUsage usage) override;

    Ren::FramebufferId createFramebuffer() override;
    void releaseFramebuffer(Ren::FramebufferId id) override;

    void bindFramebuffer(Ren::FramebufferId id) override;
    void framebufferTexture2D(RenFramebufferAttachment attachment, std::uint32_t textureHandle) override;

    void pushFramebuffer() override;
    void popFramebuffer() override;

private:
    static std::string readTextFile(const std::string& path);

    static GLuint createProgramFromSources(
        const std::string& vertexShaderCode,
        const std::string& fragmentShaderCode,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName);

    GLuint programHandle(Ren::ProgramId id) const;

    GLuint bufferHandle(Ren::BufferId id) const;

    GLuint framebufferHandle(Ren::FramebufferId id) const;

    std::vector<GLuint> programs_{};
    std::vector<GLuint> buffers_{};
    std::vector<GLuint> framebuffers_{};
    std::vector<GLuint> framebufferStack_{};
    bool initialized_{};

    SDL_Window* window_{};
    SDL_GLContext glContext_{};
};

} // namespace Ren::OpenGL
