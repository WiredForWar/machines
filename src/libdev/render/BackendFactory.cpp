#include "render/internal/IRenderBackend.hpp"

#include "spdlog/spdlog.h"

#ifdef MACHINES_HAS_BACKEND_GL21
#include "render/OpenGL/RenderBackendGL.hpp"
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

std::unique_ptr<IRenderBackend> IRenderBackend::create(BackendType type)
{
    if (type == BackendType::Auto)
    {
        const auto backends = availableBackends();
        if (backends.empty())
        {
            spdlog::error("No render backends compiled in");
            return nullptr;
        }

        type = backends.at(0);
    }

    switch (type)
    {
#ifdef MACHINES_HAS_BACKEND_GL21
    case BackendType::GL21:
        spdlog::info("Creating {} render backend", toString(type));
        return std::make_unique<OpenGL::RenderBackendGL>();
#endif
    default:
        spdlog::error("Requested render backend is not compiled in");
        return nullptr;
    }
}

} // namespace Ren
