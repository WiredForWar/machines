#pragma once

#include "render/internal/IRenderBackend.hpp"

#include <string>
#include <vector>

#include <GL/glew.h>

namespace Ren::OpenGL
{

class RenderBackendGL final : public RenIRenderBackend
{
public:
    RenderBackendGL();

    bool initialize() override;
    void shutdown() override;

    bool isInitialized() const override;

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

private:
    static std::string readTextFile(const std::string& path);

    static GLuint createProgramFromSources(
        const std::string& vertexShaderCode,
        const std::string& fragmentShaderCode,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName);

    GLuint programHandle(Ren::ProgramId id) const;

    GLuint bufferHandle(Ren::BufferId id) const;

    std::vector<GLuint> programs_{};
    std::vector<GLuint> buffers_{};
    bool initialized_{};
};

} // namespace Ren::OpenGL
