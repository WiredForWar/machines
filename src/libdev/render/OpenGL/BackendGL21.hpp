#pragma once

#include <memory>

namespace Ren
{

class IRenderBackend;

namespace OpenGL
{

// Public entry point of the GL 2.1 backend library. RenderBackendGL21.hpp is
// private to that library: it pulls in the OpenGL headers, which the generic
// backend factory must not depend on.
std::unique_ptr<IRenderBackend> createGL21();

} // namespace OpenGL

} // namespace Ren
