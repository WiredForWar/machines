#pragma once

#include "render/render.hpp"

#include <string_view>

class RenIRenderBackend
{
public:
    virtual ~RenIRenderBackend() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual bool isInitialized() const = 0;

    virtual Ren::ProgramId createProgramFromFiles(
        std::string_view vertexShaderPath,
        std::string_view fragmentShaderPath,
        std::string_view vertexShaderDebugName,
        std::string_view fragmentShaderDebugName)
        = 0;
    virtual void releaseProgram(Ren::ProgramId id) = 0;

    virtual void useProgram(Ren::ProgramId id) = 0;

    virtual int uniformLocation(Ren::ProgramId id, std::string_view name) const = 0;
    virtual int attribLocation(Ren::ProgramId id, std::string_view name) const = 0;
};
