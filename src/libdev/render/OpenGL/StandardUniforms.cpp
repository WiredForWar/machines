#include "render/OpenGL/StandardUniforms.hpp"

#include <cstring>

namespace Ren
{

namespace OpenGL
{

namespace
{

// Each reports whether the remembered value had to change, which is to say
// whether a glUniform call is actually needed.

bool changed(int& cached, int value)
{
    if (cached == value)
        return false;

    cached = value;
    return true;
}

bool changed(float& cached, float value)
{
    if (cached == value)
        return false;

    cached = value;
    return true;
}

bool changed(std::array<float, 2>& cached, float x, float y)
{
    if (cached[0] == x && cached[1] == y)
        return false;

    cached = { x, y };
    return true;
}

bool changed(std::array<float, 3>& cached, float x, float y, float z)
{
    if (cached[0] == x && cached[1] == y && cached[2] == z)
        return false;

    cached = { x, y, z };
    return true;
}

bool changed(std::array<float, 16>& cached, const float* value)
{
    if (std::memcmp(cached.data(), value, cached.size() * sizeof(float)) == 0)
        return false;

    std::memcpy(cached.data(), value, cached.size() * sizeof(float));
    return true;
}

bool changed(std::vector<float>& cached, const float* value, std::size_t count)
{
    if (cached.size() == count && std::memcmp(cached.data(), value, count * sizeof(float)) == 0)
        return false;

    cached.assign(value, value + count);
    return true;
}

} // namespace

void StandardUniforms::resolve(GLuint program)
{
    const auto at = [program](const char* name) { return glGetUniformLocation(program, name); };

    at_.screenspace = at("uScreenspace");
    at_.textureSampler = at("uTextureSampler");

    at_.view = at("uV");
    at_.proj = at("uP");
    at_.fogColour = at("uFogColour");
    at_.fogParams = at("uFogParams");
    at_.fogMode = at("uFogMode");
    at_.shadowFilterTaps = at("uShadowFilterTaps");

    at_.model = at("uM");
    at_.gpuLighting = at("uGpuLighting");
    at_.lightDir = at("uLightDir");
    at_.lightColor = at("uLightColor");
    at_.ambientColor = at("uAmbientColor");
    at_.matDiffuse = at("uMatDiffuse");
    at_.matDiffuseA = at("uMatDiffuseA");
    at_.matAmbient = at("uMatAmbient");
    at_.matEmissive = at("uMatEmissive");
    at_.filter = at("uFilter");
    at_.hasVtxMaterials = at("uHasVtxMaterials");
    at_.numPointLights = at("uNumPointLights");
    at_.pointLightPos = at("uPointLightPos");
    at_.pointLightColor = at("uPointLightColor");
    at_.pointLightRange = at("uPointLightRange");
    at_.pointLightAtten = at("uPointLightAtten");
    at_.pointLightOmni = at("uPointLightOmni");
    at_.shadowEnabled = at("uShadowEnabled");
    at_.shadowMap = at("uShadowMap");
    at_.lightSpaceMatrix = at("uLightSpaceMatrix");
    at_.shadowMapNear = at("uShadowMapNear");
    at_.lightSpaceMatrixNear = at("uLightSpaceMatrixNear");
    at_.shadowStrength = at("uShadowStrength");
    at_.textureSampler2 = at("uTextureSampler2");

    at_.viewProj = at("uVP");

    at_.sceneTexture = at("uSceneTexture");
    at_.exposure = at("uExposure");

    forgetSentValues();
}

void StandardUniforms::forgetSentValues()
{
    sent_ = {};
}

void StandardUniforms::apply(const Gui2DUniforms& uniforms)
{
    const Gui2DUniforms& u = uniforms;

    if (at_.screenspace >= 0 && changed(sent_.screenspace, u.screenspaceX, u.screenspaceY))
        glUniform2f(at_.screenspace, u.screenspaceX, u.screenspaceY);
    if (at_.textureSampler >= 0 && changed(sent_.textureSampler, u.textureSampler))
        glUniform1i(at_.textureSampler, u.textureSampler);
}

void StandardUniforms::apply(const StandardFrameUniforms& uniforms)
{
    const StandardFrameUniforms& u = uniforms;

    if (at_.view >= 0 && changed(sent_.view, u.view.data()))
        glUniformMatrix4fv(at_.view, 1, GL_FALSE, u.view.data());
    if (at_.proj >= 0 && changed(sent_.proj, u.proj.data()))
        glUniformMatrix4fv(at_.proj, 1, GL_FALSE, u.proj.data());
    if (at_.fogColour >= 0 && changed(sent_.fogColour, u.fogColourR, u.fogColourG, u.fogColourB))
        glUniform3f(at_.fogColour, u.fogColourR, u.fogColourG, u.fogColourB);
    if (at_.fogParams >= 0 && changed(sent_.fogParams, u.fogStartOrX, u.fogEndOrY, u.fogDensityOrZ))
        glUniform3f(at_.fogParams, u.fogStartOrX, u.fogEndOrY, u.fogDensityOrZ);
    if (at_.fogMode >= 0 && changed(sent_.fogMode, u.fogMode))
        glUniform1i(at_.fogMode, u.fogMode);
    if (at_.shadowFilterTaps >= 0 && changed(sent_.shadowFilterTaps, u.shadowFilterTaps))
        glUniform1i(at_.shadowFilterTaps, u.shadowFilterTaps);
}

void StandardUniforms::apply(const StandardObjectUniforms& uniforms)
{
    const StandardObjectUniforms& u = uniforms;

    if (at_.model >= 0 && changed(sent_.model, u.model.data()))
        glUniformMatrix4fv(at_.model, 1, GL_FALSE, u.model.data());

    if (at_.gpuLighting >= 0 && changed(sent_.gpuLighting, u.gpuLighting))
        glUniform1i(at_.gpuLighting, u.gpuLighting);

    if (u.gpuLighting)
    {
        if (at_.lightDir >= 0 && changed(sent_.lightDir, u.lightDirX, u.lightDirY, u.lightDirZ))
            glUniform3f(at_.lightDir, u.lightDirX, u.lightDirY, u.lightDirZ);
        if (at_.lightColor >= 0 && changed(sent_.lightColor, u.lightColorR, u.lightColorG, u.lightColorB))
            glUniform3f(at_.lightColor, u.lightColorR, u.lightColorG, u.lightColorB);
        if (at_.ambientColor >= 0 && changed(sent_.ambientColor, u.ambientColorR, u.ambientColorG, u.ambientColorB))
            glUniform3f(at_.ambientColor, u.ambientColorR, u.ambientColorG, u.ambientColorB);
        if (at_.matDiffuse >= 0 && changed(sent_.matDiffuse, u.matDiffuseR, u.matDiffuseG, u.matDiffuseB))
            glUniform3f(at_.matDiffuse, u.matDiffuseR, u.matDiffuseG, u.matDiffuseB);
        if (at_.matDiffuseA >= 0 && changed(sent_.matDiffuseA, u.matDiffuseA))
            glUniform1f(at_.matDiffuseA, u.matDiffuseA);
        if (at_.matAmbient >= 0 && changed(sent_.matAmbient, u.matAmbientR, u.matAmbientG, u.matAmbientB))
            glUniform3f(at_.matAmbient, u.matAmbientR, u.matAmbientG, u.matAmbientB);
        if (at_.matEmissive >= 0 && changed(sent_.matEmissive, u.matEmissiveR, u.matEmissiveG, u.matEmissiveB))
            glUniform3f(at_.matEmissive, u.matEmissiveR, u.matEmissiveG, u.matEmissiveB);
        if (at_.filter >= 0 && changed(sent_.filter, u.filterR, u.filterG, u.filterB))
            glUniform3f(at_.filter, u.filterR, u.filterG, u.filterB);
        if (at_.hasVtxMaterials >= 0 && changed(sent_.hasVtxMaterials, u.hasVtxMaterials))
            glUniform1i(at_.hasVtxMaterials, u.hasVtxMaterials);

        if (at_.numPointLights >= 0 && changed(sent_.numPointLights, u.numPointLights))
            glUniform1i(at_.numPointLights, u.numPointLights);

        if (u.numPointLights > 0)
        {
            const std::size_t vec3Count = static_cast<std::size_t>(u.numPointLights) * 3;
            const std::size_t scalarCount = static_cast<std::size_t>(u.numPointLights);

            if (at_.pointLightPos >= 0 && u.pointLightPos != nullptr
                && changed(sent_.pointLightPos, u.pointLightPos, vec3Count))
                glUniform3fv(at_.pointLightPos, u.numPointLights, u.pointLightPos);
            if (at_.pointLightColor >= 0 && u.pointLightColor != nullptr
                && changed(sent_.pointLightColor, u.pointLightColor, vec3Count))
                glUniform3fv(at_.pointLightColor, u.numPointLights, u.pointLightColor);
            if (at_.pointLightRange >= 0 && u.pointLightRange != nullptr
                && changed(sent_.pointLightRange, u.pointLightRange, scalarCount))
                glUniform1fv(at_.pointLightRange, u.numPointLights, u.pointLightRange);
            if (at_.pointLightAtten >= 0 && u.pointLightAtten != nullptr
                && changed(sent_.pointLightAtten, u.pointLightAtten, vec3Count))
                glUniform3fv(at_.pointLightAtten, u.numPointLights, u.pointLightAtten);
            if (at_.pointLightOmni >= 0 && u.pointLightOmni != nullptr
                && changed(sent_.pointLightOmni, u.pointLightOmni, scalarCount))
                glUniform1fv(at_.pointLightOmni, u.numPointLights, u.pointLightOmni);
        }

        if (at_.shadowEnabled >= 0 && changed(sent_.shadowEnabled, u.shadowEnabled))
            glUniform1i(at_.shadowEnabled, u.shadowEnabled);

        if (u.shadowEnabled)
        {
            if (at_.shadowMap >= 0 && changed(sent_.shadowMap, u.shadowMapUnit))
                glUniform1i(at_.shadowMap, u.shadowMapUnit);
            if (at_.lightSpaceMatrix >= 0 && changed(sent_.lightSpaceMatrix, u.lightSpaceMatrix.data()))
                glUniformMatrix4fv(at_.lightSpaceMatrix, 1, GL_FALSE, u.lightSpaceMatrix.data());
            if (at_.shadowMapNear >= 0 && changed(sent_.shadowMapNear, u.shadowMapNearUnit))
                glUniform1i(at_.shadowMapNear, u.shadowMapNearUnit);
            if (at_.lightSpaceMatrixNear >= 0
                && changed(sent_.lightSpaceMatrixNear, u.lightSpaceMatrixNear.data()))
                glUniformMatrix4fv(at_.lightSpaceMatrixNear, 1, GL_FALSE, u.lightSpaceMatrixNear.data());
            if (at_.shadowStrength >= 0 && changed(sent_.shadowStrength, u.shadowStrength))
                glUniform1f(at_.shadowStrength, u.shadowStrength);
        }
    }

    if (at_.textureSampler2 >= 0 && changed(sent_.textureSampler2, u.textureSampler))
        glUniform1i(at_.textureSampler2, u.textureSampler);
}

void StandardUniforms::apply(const BillboardUniforms& uniforms)
{
    const BillboardUniforms& u = uniforms;

    if (at_.viewProj >= 0 && changed(sent_.viewProj, u.viewProj.data()))
        glUniformMatrix4fv(at_.viewProj, 1, GL_FALSE, u.viewProj.data());
    if (at_.textureSampler >= 0 && changed(sent_.textureSampler, u.textureSampler))
        glUniform1i(at_.textureSampler, u.textureSampler);
}

void StandardUniforms::apply(const ShadowDepthUniforms& uniforms)
{
    const ShadowDepthUniforms& u = uniforms;

    if (at_.lightSpaceMatrix >= 0 && changed(sent_.lightSpaceMatrix, u.lightSpaceMatrix.data()))
        glUniformMatrix4fv(at_.lightSpaceMatrix, 1, GL_FALSE, u.lightSpaceMatrix.data());
    if (at_.model >= 0 && changed(sent_.model, u.model.data()))
        glUniformMatrix4fv(at_.model, 1, GL_FALSE, u.model.data());
}

void StandardUniforms::apply(const PostProcessUniforms& uniforms)
{
    const PostProcessUniforms& u = uniforms;

    if (at_.sceneTexture >= 0 && changed(sent_.sceneTexture, u.sceneTextureSampler))
        glUniform1i(at_.sceneTexture, u.sceneTextureSampler);
    if (at_.exposure >= 0 && changed(sent_.exposure, u.exposure))
        glUniform1f(at_.exposure, u.exposure);
}

} // namespace OpenGL

} // namespace Ren
