#include "render/OpenGL/Utils.hpp"

namespace Ren
{

namespace OpenGL
{

GLenum toDrawMode(Ren::PrimitiveTopology topology)
{
    switch (topology)
    {
    case PrimitiveTopology::Points:
        return GL_POINTS;
    case PrimitiveTopology::Lines:
        return GL_LINES;
    case PrimitiveTopology::LineStrip:
        return GL_LINE_STRIP;
    case PrimitiveTopology::Triangles:
        return GL_TRIANGLES;
    case PrimitiveTopology::TriangleStrip:
        return GL_TRIANGLE_STRIP;
    case PrimitiveTopology::TriangleFan:
        return GL_TRIANGLE_FAN;
    }

    // UNREACHABLE
    return GL_NONE;
}

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

GLenum toIndexType(BackendIndexType type)
{
    switch (type)
    {
    case BackendIndexType::UnsignedByte:
        return GL_UNSIGNED_BYTE;
    case BackendIndexType::UnsignedShort:
        return GL_UNSIGNED_SHORT;
    case BackendIndexType::UnsignedInt:
        return GL_UNSIGNED_INT;
    }
    return GL_UNSIGNED_SHORT;
}

GLbitfield toClearMask(std::uint32_t mask)
{
    GLbitfield glMask = 0;
    if (mask & backendClearMask(BackendClearFlag::Colour))
    {
        glMask |= GL_COLOR_BUFFER_BIT;
    }
    if (mask & backendClearMask(BackendClearFlag::Depth))
    {
        glMask |= GL_DEPTH_BUFFER_BIT;
    }
    if (mask & backendClearMask(BackendClearFlag::Stencil))
    {
        glMask |= GL_STENCIL_BUFFER_BIT;
    }
    return glMask;
}

} // namespace OpenGL

} // namespace Ren
