#pragma once

#include "base/base.hpp"
#include "system/pathname.hpp"
#include "world4d/world4d.hpp"

#include <string>
#include <unordered_map>
#include <vector>

class MachPhysPlanetSurface;
class RenMeshInstance;
class W4dComposite;
class W4dEntity;
class W4dRoot;

// Map from CDF basename (e.g. "adb1.cdf") to full relative path (e.g. "models/administ/boss/level1/adb1.cdf").
using CdfPathMap = std::unordered_map<std::string, std::string>;

// Export all composites from a given root entity tree.
// pathMap provides authoritative CDF paths keyed by basename.
void MachPhysExportFromRoot(
    const W4dRoot& root,
    const SysPathName& outputDir,
    const std::string& prefix,
    const CdfPathMap& pathMap = {});

// Static methods only -- cannot be instantiated.
class MachPhysModelExporter
{
public:
    // Export all loaded exemplar composites from all factories to outputDir.
    // Call after MachPhysPreload::persistentPreload() has completed.
    static void exportAll(const SysPathName& outputDir);

    // Export shared models (wheels, tracks, hover, faceplate) that are not
    // part of any factory root but are loaded by preloadMachines() for mesh caching.
    // Reconstructs .cdf + .x files from mesh data embedded in machine exemplars.
    // roots: factory roots to scan for shared mesh instances.
    static void exportSharedModels(const SysPathName& outputDir, const std::vector<const W4dRoot*>& roots);

    // Export a single composite to outputDir/<name>.cdf + outputDir/<name>_lod<N>.x + .glb
    static void exportComposite(const W4dComposite& composite, const SysPathName& outputDir, const std::string& name);

    // Export terrain tile .lod + .x files from a loaded planet surface.
    // Call after loading from .psb when MACHINES_EXPORT_MODELS is set.
    static void exportTerrainTiles(const MachPhysPlanetSurface& surface, const SysPathName& outputDir);

    // Write a Mesh block for one RenMeshInstance (to .x text stream).
    static void writeMesh(
        std::ostream& os,
        const RenMeshInstance& meshInst,
        const std::string& indent);

    // Write a .glb (binary glTF) file for one LOD level of a composite.
    // Returns the leaf filename written, or "" if LOD is empty.
    static std::string writeGltfFile(
        const W4dComposite& composite,
        const SysPathName& outputDir,
        const std::string& baseName,
        W4dLOD lod);

    // Write a .glb file for a collection of named mesh instances (shared models).
    // meshes: vector of (meshName, RenMeshInstance*) pairs.
    static void writeSharedGltfFile(
        const SysPathName& outputDir,
        const std::string& baseName,
        const std::vector<std::pair<std::string, const RenMeshInstance*>>& meshes);

private:
    MachPhysModelExporter() = delete;

    // Determine max LOD count across composite and all its links
    static W4dLOD maxLODCount(const W4dComposite& composite);

    // Write the .x file for one LOD level. Returns the leaf filename written, or "" if LOD is empty.
    static std::string writeXFile(
        const W4dComposite& composite,
        const SysPathName& outputDir,
        const std::string& baseName,
        W4dLOD lod);

    // Write animation .x files for all composite plans. Returns vector of {planName, xFileName, animSetName, fps}.
    struct AnimInfo
    {
        std::string planName;
        std::string xFileName;
        std::string animSetName;
        double fps{};
    };
    static std::vector<AnimInfo> writeAnimationXFiles(
        const W4dComposite& composite,
        const SysPathName& outputDir,
        const std::string& baseName);

    // Write the .cdf file referencing one or more .x files with DISTANCE lines + animations.
    static void writeCdfFile(
        const W4dComposite& composite,
        const SysPathName& outputDir,
        const std::string& baseName,
        const std::vector<std::string>& xFileNames,
        const std::vector<double>& distances,
        const std::vector<AnimInfo>& animations);
};
