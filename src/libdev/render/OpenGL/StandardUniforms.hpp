#pragma once

#include "render/internal/UniformBlocks.hpp"

#include <array>
#include <vector>

#include <GL/glew.h>

namespace Ren
{

namespace OpenGL
{

// Where one program keeps the renderer's uniform blocks, and what was last put
// in them.
//
// A uniform holds its value in the program object, so one already holding what
// is about to be written to it does not need writing to. That is the whole
// reason this remembers anything: the device restates most of a block per draw,
// and almost none of it has changed.
//
// Shared by every GL backend. These are plain glUniform calls against the
// renderer's own structs, and not one of them differs between 2.1, core and ES —
// what differs between those is which blocks a given shader set declares, and
// that shows up as a location of -1 here, which is already the "not in this
// program" answer.
class StandardUniforms
{
public:
    // Ask a freshly linked program where its uniforms are. A block the program
    // does not declare leaves its locations at -1 and is then skipped.
    void resolve(GLuint program);

    // Forget what was sent, without forgetting where it goes. For a new context,
    // in which every uniform is back at zero however this program was built.
    void forgetSentValues();

    void apply(const Gui2DUniforms& uniforms);
    void apply(const StandardFrameUniforms& uniforms);
    void apply(const StandardObjectUniforms& uniforms);
    void apply(const BillboardUniforms& uniforms);
    void apply(const ShadowDepthUniforms& uniforms);
    void apply(const PostProcessUniforms& uniforms);

private:
    struct Locations
    {
        // 2D interface
        GLint screenspace{-1};
        GLint textureSampler{-1};

        // Per frame
        GLint view{-1};
        GLint proj{-1};
        GLint fogColour{-1};
        GLint fogParams{-1};
        GLint fogMode{-1};
        GLint shadowFilterTaps{-1};

        // Per object
        GLint model{-1};
        GLint gpuLighting{-1};
        GLint lightDir{-1};
        GLint lightColor{-1};
        GLint ambientColor{-1};
        GLint matDiffuse{-1};
        GLint matDiffuseA{-1};
        GLint matAmbient{-1};
        GLint matEmissive{-1};
        GLint filter{-1};
        GLint hasVtxMaterials{-1};
        GLint numPointLights{-1};
        GLint pointLightPos{-1};
        GLint pointLightColor{-1};
        GLint pointLightRange{-1};
        GLint pointLightAtten{-1};
        GLint pointLightOmni{-1};
        GLint shadowEnabled{-1};
        GLint shadowMap{-1};
        GLint lightSpaceMatrix{-1};
        GLint shadowMapNear{-1};
        GLint lightSpaceMatrixNear{-1};
        GLint shadowStrength{-1};
        GLint textureSampler2{-1};

        // Billboards
        GLint viewProj{-1};

        // Post-process
        GLint sceneTexture{-1};
        GLint exposure{-1};
    };

    struct SentValues
    {
        std::array<float, 2> screenspace{};
        int textureSampler{};

        std::array<float, 16> view{};
        std::array<float, 16> proj{};
        std::array<float, 3> fogColour{};
        std::array<float, 3> fogParams{};
        int fogMode{};
        int shadowFilterTaps{};

        std::array<float, 16> model{};
        int gpuLighting{};
        std::array<float, 3> lightDir{};
        std::array<float, 3> lightColor{};
        std::array<float, 3> ambientColor{};
        std::array<float, 3> matDiffuse{};
        float matDiffuseA{};
        std::array<float, 3> matAmbient{};
        std::array<float, 3> matEmissive{};
        std::array<float, 3> filter{};
        int hasVtxMaterials{};
        int numPointLights{};
        std::vector<float> pointLightPos{};
        std::vector<float> pointLightColor{};
        std::vector<float> pointLightRange{};
        std::vector<float> pointLightAtten{};
        std::vector<float> pointLightOmni{};
        int shadowEnabled{};
        int shadowMap{};
        std::array<float, 16> lightSpaceMatrix{};
        int shadowMapNear{};
        std::array<float, 16> lightSpaceMatrixNear{};
        float shadowStrength{};
        int textureSampler2{};

        std::array<float, 16> viewProj{};

        int sceneTexture{};
        float exposure{};
    };

    Locations at_{};
    SentValues sent_{};
};

} // namespace OpenGL

} // namespace Ren
