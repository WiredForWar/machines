#pragma once

#include "render/internal/BackendCommands.hpp"
#include "render/internal/BackendTypes.hpp"
#include "render/internal/UniformBlocks.hpp"
#include "render/PrimitiveTopology.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

class RenIVertex;
class RenMaterial;
class RenColour;

namespace Ren
{

// Lightweight snapshot of per-frame state needed to build standard 3D uniforms.
struct FrameState
{
    std::array<float, 16> view{};
    std::array<float, 16> proj{};
    float fogColourR{};
    float fogColourG{};
    float fogColourB{};
    float fogStartOrX{};
    float fogEndOrY{};
    float fogDensityOrZ{};
};

// Lightweight snapshot of per-object GPU lighting state.
struct GpuLightingState
{
    bool enabled{};
    glm::vec3 lightDir{};
    glm::vec3 lightColor{};
    glm::vec3 ambientColor{};
    glm::vec3 filter{};
    bool hasPerVertexMaterials{};

    int numPointLights{};
    const glm::vec3* pointLightPos{};
    const glm::vec3* pointLightColor{};
    const float* pointLightRange{};
    const glm::vec3* pointLightAtten{};
    const float* pointLightOmni{};

    bool shadowEnabled{};
    float shadowStrength{};
    float shadowSplitDistance{};
    std::array<float, 16> lightSpaceMatrix{};
    std::array<float, 16> lightSpaceMatrixNear{};
    BackendTextureHandle shadowDepthTexture{};
    BackendTextureHandle shadowNearDepthTexture{};
};

// Pipeline handles and buffer IDs needed to emit standard 3D draw commands.
struct StandardPipelineHandles
{
    PipelineId pipelineId{};
    AttributeLocationId posAttr{};
    AttributeLocationId uvAttr{};
    AttributeLocationId colAttr{};
    AttributeLocationId normalAttr{};
    AttributeLocationId vtxDiffuseAttr{};
    AttributeLocationId vtxAmbientAttr{};
    AttributeLocationId vtxEmissiveAttr{};
    BufferId vertexBuffer{};
    BufferId normalBuffer{};
    BufferId vtxDiffuseBuffer{};
    BufferId vtxAmbientBuffer{};
    BufferId vtxEmissiveBuffer{};
    BufferId elementBuffer{};
};

// Pipeline handles and buffer IDs for the billboard pipeline.
struct BillboardPipelineHandles
{
    PipelineId pipelineId{};
    AttributeLocationId posAttr{};
    AttributeLocationId uvAttr{};
    AttributeLocationId colAttr{};
    BufferId vertexBuffer{};
    BufferId elementBuffer{};
};

// Pipeline handles and buffer IDs for the shadow depth pipeline.
struct ShadowDepthPipelineHandles
{
    PipelineId pipelineId{};
    AttributeLocationId posAttr{};
    BufferId vertexBuffer{};
    BufferId elementBuffer{};
};

// Builds BackendCommand sequences for the various draw-call types.
// Decouples command generation from RenDevice, eliminating duplication
// between renderPrimitive and renderIndexed.
class DrawCallFactory
{
public:
    using Commands = std::vector<BackendCommand>;

    // Build the StandardObjectUniforms block from material + GPU lighting state.
    static StandardObjectUniforms buildStandardObjectUniforms(
        const std::array<float, 16>& model,
        const RenMaterial& mat,
        const GpuLightingState& lighting,
        Commands* out);

    // Emit commands for a standard 3D non-indexed draw.
    static void emitStandard3DDraw(
        const StandardPipelineHandles& handles,
        const FrameState& frame,
        bool frameUniformsDirty,
        const std::array<float, 16>& model,
        const RenMaterial& mat,
        const GpuLightingState& lighting,
        BackendTextureHandle materialTexture,
        const RenIVertex* vertices,
        std::size_t nVertices,
        const float* expandedNormals,
        const float* expandedVtxDiffuse,
        const float* expandedVtxAmbient,
        const float* expandedVtxEmissive,
        PrimitiveTopology topology,
        Commands* out);

    // Emit commands for a standard 3D indexed draw.
    static void emitStandard3DDrawIndexed(
        const StandardPipelineHandles& handles,
        const FrameState& frame,
        bool frameUniformsDirty,
        const std::array<float, 16>& model,
        const RenMaterial& mat,
        const GpuLightingState& lighting,
        BackendTextureHandle materialTexture,
        const RenIVertex* vertices,
        std::size_t nVertices,
        const VertexIdx* indices,
        std::size_t nIndices,
        const float* expandedNormals,
        const float* expandedVtxDiffuse,
        const float* expandedVtxAmbient,
        const float* expandedVtxEmissive,
        PrimitiveTopology topology,
        Commands* out);

    // Emit commands for a billboard indexed draw.
    static void emitBillboardDrawIndexed(
        const BillboardPipelineHandles& handles,
        const BillboardUniforms& uniforms,
        bool uniformsDirty,
        BackendTextureHandle texture,
        const RenIVertex* vertices,
        std::size_t nVertices,
        const VertexIdx* indices,
        std::size_t nIndices,
        PrimitiveTopology topology,
        Commands* out);

    // Emit commands for a shadow depth indexed draw.
    static void emitShadowDepthDrawIndexed(
        const ShadowDepthPipelineHandles& handles,
        const ShadowDepthUniforms& uniforms,
        const RenIVertex* vertices,
        std::size_t nVertices,
        const VertexIdx* indices,
        std::size_t nIndices,
        PrimitiveTopology topology,
        Commands* out);

private:
    // Shared helper: emit pipeline bind, frame uniforms, object uniforms,
    // texture bind, vertex buffer upload + layout, and optional normal/material buffers.
    static void emitStandard3DCommon(
        const StandardPipelineHandles& handles,
        const FrameState& frame,
        bool frameUniformsDirty,
        const std::array<float, 16>& model,
        const RenMaterial& mat,
        const GpuLightingState& lighting,
        BackendTextureHandle materialTexture,
        const RenIVertex* vertices,
        std::size_t nVertices,
        const float* expandedNormals,
        const float* expandedVtxDiffuse,
        const float* expandedVtxAmbient,
        const float* expandedVtxEmissive,
        Commands* out);
};

} // namespace Ren
