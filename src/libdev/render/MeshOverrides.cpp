#include "render/MeshOverrides.hpp"

#include "base/diag.hpp"
#include "render/mesh.hpp"
#include "render/render.hpp"

#include "spdlog/spdlog.h"

#include <filesystem>

// static
RenMeshOverrides& RenMeshOverrides::instance()
{
    static RenMeshOverrides inst;
    return inst;
}

RenMeshOverrides::RenMeshOverrides()
{
}

void RenMeshOverrides::scanDirectory(const std::string& overrideDir)
{
    namespace fs = std::filesystem;

    fs::path root(overrideDir);
    if (!fs::is_directory(root))
        return;

    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(root))
    {
        if (!entry.is_regular_file())
            continue;

        std::string ext = entry.path().extension().string();
        // Convert to lowercase for comparison
        for (char& c : ext)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        // Check if the extension (without leading dot) is supported by any loader
        bool supported = false;
        if (!ext.empty() && ext[0] == '.')
        {
            std::string extNoDot = ext.substr(1);
            for (const auto& se : Ren::supportedMeshExtensions())
            {
                if (se == extNoDot)
                {
                    supported = true;
                    break;
                }
            }
        }
        if (!supported)
            continue;

        // The relative path under the override directory is the key, but always
        // with .x extension -- the original paths stored in models.bin always
        // use .x regardless of the override file format (.glb, .gltf, etc.).
        // E.g. overrideDir="mods/meshes", file="mods/meshes/models/shared/wheel/xwhl48.glb"
        // => key = "models/shared/wheel/xwhl48.x"
        fs::path relative = fs::relative(entry.path(), root);
        fs::path keyPath = relative;
        keyPath.replace_extension(".x");
        std::string key = keyPath.generic_string();

        overrides_[key] = SysPathName(entry.path().generic_string());
    }
}

std::optional<SysPathName> RenMeshOverrides::findOverride(const SysPathName& originalPath) const
{
    // Normalize the original path to forward slashes for matching.
    std::string key = originalPath.pathname();
    for (char& c : key)
    {
        if (c == '\\')
            c = '/';
    }

    auto it = overrides_.find(key);
    if (it != overrides_.end())
        return it->second;

    return std::nullopt;
}

void RenMeshOverrides::markForOverride(RenMesh* mesh, const SysPathName& overridePath)
{
    pending_.push_back({mesh, overridePath});
}

void RenMeshOverrides::applyPendingOverrides()
{
    for (const PendingOverride& po : pending_)
    {
        spdlog::info("RenMeshOverrides: applying deferred override {}", po.overridePath.pathname());
        RENDER_STREAM("RenMeshOverrides: applying deferred override " << po.overridePath << std::endl);
        po.mesh->reloadFromFile(po.overridePath);
    }
    pending_.clear();
}

size_t RenMeshOverrides::count() const
{
    return overrides_.size();
}

void RenMeshOverrides::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}
