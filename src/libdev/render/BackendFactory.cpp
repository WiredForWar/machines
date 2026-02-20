#include "render/internal/IRenderBackend.hpp"

#include "spdlog/spdlog.h"

#ifdef MACHINES_HAS_BACKEND_GL21
#include "render/OpenGL/BackendGL.hpp"
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

BackendType IRenderBackend::resolveAutoBackend()
{
    const auto backends = availableBackends();
    if (backends.empty())
    {
        spdlog::critical("No render backends compiled in");
        std::exit(-1);
    }

    return backends.at(0);
}

std::unique_ptr<IRenderBackend> IRenderBackend::create(BackendType type)
{
    if (type == BackendType::Auto)
        type = resolveAutoBackend();

    switch (type)
    {
#ifdef MACHINES_HAS_BACKEND_GL21
    case BackendType::GL21:
        spdlog::info("Creating {} render backend", toString(type));
        return OpenGL::createGL();
#endif
    default:
        spdlog::error("Requested render backend is not compiled in");
        return nullptr;
    }
}

} // namespace Ren
