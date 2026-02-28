#pragma once

#include "base/base.hpp"
#include "system/pathname.hpp"

#include <unordered_map>
#include <optional>
#include <string>
#include <vector>

class RenMesh;

// Registry of mesh overrides.  Maps original mesh file paths (as stored in
// RenMesh::pathName()) to replacement .x files found in override directories.
//
// Usage:
//   RenMeshOverrides::instance().scanDirectory("mods/meshes");
//   // ... later, after models.bin / .psb deserialization ...
//   // perRead(PerIstream&, RenMesh&) checks the registry automatically.
//
// The override directory must mirror the original path structure.  For example,
// to override "models/shared/wheel/xwhl48.x", place the replacement file at
// "mods/meshes/models/shared/wheel/xwhl48.glb" (or .x, .gltf, .agt).
// The key is always normalised to .x regardless of the override file format.
class RenMeshOverrides
{
public:
    static RenMeshOverrides& instance();

    // Recursively scan a directory for mesh files and register them as overrides.
    // The directory must mirror the original model path structure.
    // E.g. if overrideDir is "mods/meshes" and it contains
    // "models/shared/wheel/xwhl48.x", then any mesh whose pathName() is
    // "models/shared/wheel/xwhl48.x" will be replaced.
    void scanDirectory(const std::string& overrideDir);

    // Look up an override for the given original path.
    // Returns the override file path if one is registered, or std::nullopt.
    std::optional<SysPathName> findOverride(const SysPathName& originalPath) const;

    // Schedule a mesh for deferred override.  Called during perRead when
    // it is unsafe to destroy the deserialized geometry immediately.
    void markForOverride(RenMesh* mesh, const SysPathName& overridePath);

    // Apply all pending overrides.  Call after the persistence stream is
    // fully read (e.g. after persistentPreload returns).
    void applyPendingOverrides();

    // Number of registered overrides.
    size_t count() const;

    void CLASS_INVARIANT;

private:
    RenMeshOverrides();

    // Key: original relative path (e.g. "models/shared/wheel/xwhl48.x")
    // Value: absolute or relative path to the override file
    std::unordered_map<std::string, SysPathName> overrides_;

    struct PendingOverride
    {
        RenMesh* mesh{};
        SysPathName overridePath;
    };
    std::vector<PendingOverride> pending_;
};
