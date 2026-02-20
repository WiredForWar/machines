#pragma once

#include <memory>

namespace Ren
{

class IRenderBackend;

namespace OpenGL
{

// Public entry point of the OpenGL backend library. RenderBackendGL.hpp is
// private to that library: it pulls in the OpenGL headers, which the generic
// backend factory must not depend on.
std::unique_ptr<IRenderBackend> createGL();

} // namespace OpenGL

} // namespace Ren
