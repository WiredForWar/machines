#pragma once

#include "render/PrimitiveTopology.hpp"
#include "render/internal/BackendTypes.hpp"

#include <GL/glew.h>

namespace Ren
{

namespace OpenGL
{
class ScopedEnable
{
public:
    explicit ScopedEnable(GLenum capability)
        : capability_(capability)
    {
        glEnable(capability_);
    }

    ~ScopedEnable() { glDisable(capability_); }

private:
    GLenum capability_{};
};

class ScopedDisable
{
public:
    explicit ScopedDisable(GLenum capability)
        : capability_(capability)
    {
        glDisable(capability_);
    }

    ~ScopedDisable() { glEnable(capability_); }

private:
    GLenum capability_{};
};

GLenum toDrawMode(Ren::PrimitiveTopology topology);
GLenum toStorageFormat(TextureFormat format);
GLenum toPixelFormat(TextureFormat format);
GLenum toFilter(TextureFilter filter);
GLenum toWrap(TextureWrap wrap);

GLbitfield toClearMask(std::uint32_t mask);

} // namespace OpenGL

} // namespace Ren
