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

private:
    bool initialized_{};
};

} // namespace Ren::OpenGL
