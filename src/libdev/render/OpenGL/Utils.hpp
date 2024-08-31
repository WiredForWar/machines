#pragma once

#include "render/PrimitiveTopology.hpp"

#include <GL/glew.h>

namespace Ren
{

namespace OpenGL
{
    constexpr GLenum getDrawMode(Ren::PrimitiveTopology topology)
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
} // namespace OpenGL

} // namespace Ren
