#pragma once

#include <cstddef>
#include <vector>

namespace RenI {

// Snapshot of per-mesh GPU lighting arrays for delayed rendering groups.
// Per-frame light parameters (direction, colour, point lights) are NOT
// included because they remain valid throughout the frame.
struct GpuMeshLightingSnapshot
{
    std::vector<float> normals{};
    size_t normalsCount{};
    std::vector<float> vtxDiffuse{};
    std::vector<float> vtxAmbient{};
    std::vector<float> vtxEmissive{};
    bool hasPerVertexMaterials{};
};

} // namespace RenI
