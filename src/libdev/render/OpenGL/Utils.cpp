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

GLenum toBlendFactor(BackendBlendFactor factor)
{
    switch (factor)
    {
    case BackendBlendFactor::Zero:
        return GL_ZERO;
    case BackendBlendFactor::One:
        return GL_ONE;
    case BackendBlendFactor::SrcColor:
        return GL_SRC_COLOR;
    case BackendBlendFactor::OneMinusSrcColor:
        return GL_ONE_MINUS_SRC_COLOR;
    case BackendBlendFactor::DstColor:
        return GL_DST_COLOR;
    case BackendBlendFactor::OneMinusDstColor:
        return GL_ONE_MINUS_DST_COLOR;
    case BackendBlendFactor::SrcAlpha:
        return GL_SRC_ALPHA;
    case BackendBlendFactor::OneMinusSrcAlpha:
        return GL_ONE_MINUS_SRC_ALPHA;
    case BackendBlendFactor::DstAlpha:
        return GL_DST_ALPHA;
    case BackendBlendFactor::OneMinusDstAlpha:
        return GL_ONE_MINUS_DST_ALPHA;
    }
    return GL_ONE;
}

GLenum toDepthFunc(BackendDepthFunc func)
{
    switch (func)
    {
    case BackendDepthFunc::Never:
        return GL_NEVER;
    case BackendDepthFunc::Less:
        return GL_LESS;
    case BackendDepthFunc::Equal:
        return GL_EQUAL;
    case BackendDepthFunc::LessOrEqual:
        return GL_LEQUAL;
    case BackendDepthFunc::Greater:
        return GL_GREATER;
    case BackendDepthFunc::NotEqual:
        return GL_NOTEQUAL;
    case BackendDepthFunc::GreaterOrEqual:
        return GL_GEQUAL;
    case BackendDepthFunc::Always:
        return GL_ALWAYS;
    }
    return GL_LEQUAL;
}

GLenum toVertexAttribType(BackendVertexAttribType type)
{
    switch (type)
    {
    case BackendVertexAttribType::Float:
        return GL_FLOAT;
    case BackendVertexAttribType::UnsignedByte:
        return GL_UNSIGNED_BYTE;
    }
    return GL_FLOAT;
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
