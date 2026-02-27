#pragma once

#include <memory>

namespace Ren
{

class IRenderBackend;
class IRenderSurface;

namespace OpenGL
{

// Public entry point of the GL 2.1 backend library. RenderBackendGL21.hpp is
// private to that library: it pulls in the OpenGL headers, which the generic
// backend factory must not depend on.

// Whether this backend can present on the given surface. Cheap: it only checks
// that the host can provide a GL context, not that the driver is good enough --
// that is only known once initialize() has one to query.
bool canUseGL21(IRenderSurface* surface);

std::unique_ptr<IRenderBackend> createGL21();

} // namespace OpenGL

} // namespace Ren
