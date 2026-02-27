#include "render/internal/IRenderBackend.hpp"

#include "spdlog/spdlog.h"

#ifdef MACHINES_HAS_BACKEND_GL21
#include "render/OpenGL/BackendGL21.hpp"
#endif

namespace Ren
{

std::vector<BackendType> IRenderBackend::availableBackends()
{
    return {
#ifdef MACHINES_HAS_BACKEND_GL21
        BackendType::GL21,
#endif
    };
}

std::vector<BackendType> IRenderBackend::supportedBackends(IRenderSurface* surface)
{
    if (surface == nullptr)
        return availableBackends();

    // Each backend decides for itself what it needs from the surface; the
    // factory only knows how to ask.
    std::vector<BackendType> result;
#ifdef MACHINES_HAS_BACKEND_GL21
    if (OpenGL::canUseGL21(surface))
        result.push_back(BackendType::GL21);
#endif

    return result;
}

BackendType IRenderBackend::resolveAutoBackend(IRenderSurface* surface)
{
    // In preference order, so the first the surface supports is the best one.
    const auto backends = supportedBackends(surface);
    if (backends.empty())
    {
        spdlog::critical("No render backend can present on this surface");
        std::exit(-1);
    }

    return backends.at(0);
}

std::unique_ptr<IRenderBackend> IRenderBackend::create(IRenderSurface* surface, BackendType type)
{
    if (type == BackendType::Auto)
        type = resolveAutoBackend(surface);

    switch (type)
    {
#ifdef MACHINES_HAS_BACKEND_GL21
    case BackendType::GL21:
        if (surface != nullptr && !OpenGL::canUseGL21(surface))
        {
            spdlog::error("Requested {} render backend cannot present on this surface", toString(type));
            return nullptr;
        }

        spdlog::info("Creating {} render backend", toString(type));
        return OpenGL::createGL21();
#endif
    default:
        spdlog::error("Requested render backend is not compiled in");
        return nullptr;
    }
}

} // namespace Ren
