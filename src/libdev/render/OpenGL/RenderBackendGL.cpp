#include "render/OpenGL/RenderBackendGL.hpp"

namespace Ren::OpenGL
{

RenderBackendGL::RenderBackendGL()
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

} // namespace Ren::OpenGL
