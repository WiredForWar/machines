#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace Ren
{

// Uniform block for the 2D/GUI pipeline.
struct Gui2DUniforms
{
    float screenspaceX{};
    float screenspaceY{};
    int textureSampler{};
};

// Per-frame uniforms for the standard 3D pipeline (set once per frame or when camera/fog changes).
struct StandardFrameUniforms
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

// Per-object uniforms for the standard 3D pipeline (set per draw call).
struct StandardObjectUniforms
{
    std::array<float, 16> model{};
    int gpuLighting{};
    // Directional light
    float lightDirX{};
    float lightDirY{};
    float lightDirZ{};
    float lightColorR{};
    float lightColorG{};
    float lightColorB{};
    float ambientColorR{};
    float ambientColorG{};
    float ambientColorB{};
    // Material
    float matDiffuseR{};
    float matDiffuseG{};
    float matDiffuseB{};
    float matAmbientR{};
    float matAmbientG{};
    float matAmbientB{};
    float matEmissiveR{};
    float matEmissiveG{};
    float matEmissiveB{};
    // Filter
    float filterR{};
    float filterG{};
    float filterB{};
    int hasVtxMaterials{};
    // Point lights
    int numPointLights{};
    // Flat arrays: 3 floats per light, up to 16 lights.
    std::vector<float> pointLightPos{};
    std::vector<float> pointLightColor{};
    std::vector<float> pointLightRange{};
    std::vector<float> pointLightAtten{};
    std::vector<float> pointLightOmni{};
    // Shadows
    int shadowEnabled{};
    float shadowStrength{};
    float shadowSplitDistance{};
    std::array<float, 16> lightSpaceMatrix{};
    std::array<float, 16> lightSpaceMatrixNear{};
    int textureSampler{};
    int shadowMapUnit{};
    int shadowMapNearUnit{};
};

// Uniform block for the billboard pipeline.
struct BillboardUniforms
{
    std::array<float, 16> viewProj{};
    int textureSampler{};
};

// Uniform block for the shadow depth pipeline.
struct ShadowDepthUniforms
{
    std::array<float, 16> lightSpaceMatrix{};
    std::array<float, 16> model{};
};

// Uniform block for the post-process pipeline.
struct PostProcessUniforms
{
    int sceneTextureSampler{};
    float exposure{};
};

} // namespace Ren
