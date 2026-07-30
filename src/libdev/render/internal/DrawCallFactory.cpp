#include "render/internal/DrawCallFactory.hpp"

#include "render/Colour.hpp"
#include "render/Material.hpp"
#include "render/internal/VertexData.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace Ren
{

static constexpr int TextureUnit = 0;
static constexpr int ShadowFarTextureUnit = 1;
static constexpr int ShadowNearTextureUnit = 2;

// The point light uniforms are uploaded as one flat run of 3 floats per light,
// so an array of vec3 has to be exactly that with nothing in between.
static_assert(sizeof(glm::vec3) == 3 * sizeof(float), "glm::vec3 must be tightly packed");

StandardObjectUniforms DrawCallFactory::buildStandardObjectUniforms(
    const std::array<float, 16>& model,
    const RenMaterial& mat,
    const GpuLightingState& lighting,
    CommandSink& sink)
{
    StandardObjectUniforms ou;
    ou.model = model;
    ou.gpuLighting = lighting.enabled ? 1 : 0;
    ou.textureSampler = TextureUnit;

    if (lighting.enabled)
    {
        ou.lightDirX = lighting.lightDir.x;
        ou.lightDirY = lighting.lightDir.y;
        ou.lightDirZ = lighting.lightDir.z;
        ou.lightColorR = lighting.lightColor.x;
        ou.lightColorG = lighting.lightColor.y;
        ou.lightColorB = lighting.lightColor.z;
        ou.ambientColorR = lighting.ambientColor.x;
        ou.ambientColorG = lighting.ambientColor.y;
        ou.ambientColorB = lighting.ambientColor.z;

        const RenColour& md = mat.diffuse();
        const RenColour& ma = mat.ambient();
        const RenColour& me = mat.emissive();
        ou.matDiffuseR = md.r(); ou.matDiffuseG = md.g(); ou.matDiffuseB = md.b(); ou.matDiffuseA = md.a();
        ou.matAmbientR = ma.r(); ou.matAmbientG = ma.g(); ou.matAmbientB = ma.b();
        ou.matEmissiveR = me.r(); ou.matEmissiveG = me.g(); ou.matEmissiveB = me.b();

        ou.filterR = lighting.filter.x;
        ou.filterG = lighting.filter.y;
        ou.filterB = lighting.filter.z;
        ou.hasVtxMaterials = lighting.hasPerVertexMaterials ? 1 : 0;

        const int nPt = lighting.numPointLights;
        ou.numPointLights = nPt;
        if (nPt > 0)
        {
            ou.pointLightPos = glm::value_ptr(lighting.pointLightPos[0]);
            ou.pointLightColor = glm::value_ptr(lighting.pointLightColor[0]);
            ou.pointLightRange = lighting.pointLightRange;
            ou.pointLightAtten = glm::value_ptr(lighting.pointLightAtten[0]);
            ou.pointLightOmni = lighting.pointLightOmni;
        }

        ou.shadowEnabled = lighting.shadowEnabled ? 1 : 0;
        if (lighting.shadowEnabled)
        {
            sink.emit(Command::bindTexture2D(lighting.shadowDepthTexture, ShadowFarTextureUnit));
            sink.emit(Command::bindTexture2D(lighting.shadowNearDepthTexture, ShadowNearTextureUnit));
            ou.shadowMapUnit = ShadowFarTextureUnit;
            ou.shadowMapNearUnit = ShadowNearTextureUnit;
            ou.lightSpaceMatrix = lighting.lightSpaceMatrix;
            ou.lightSpaceMatrixNear = lighting.lightSpaceMatrixNear;
            ou.shadowSplitDistance = lighting.shadowSplitDistance;
            ou.shadowStrength = lighting.shadowStrength;
        }
    }

    return ou;
}

void DrawCallFactory::emitStandard3DCommon(
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
    CommandSink& sink)
{
    sink.emit(Command::bindPipeline(handles.pipelineId));

    if (frameUniformsDirty)
    {
        StandardFrameUniforms fu;
        fu.view = frame.view;
        fu.proj = frame.proj;
        fu.fogColourR = frame.fogColourR;
        fu.fogColourG = frame.fogColourG;
        fu.fogColourB = frame.fogColourB;
        fu.fogStartOrX = frame.fogStartOrX;
        fu.fogEndOrY = frame.fogEndOrY;
        fu.fogDensityOrZ = frame.fogDensityOrZ;
        fu.fogMode = frame.fogMode;
        fu.shadowFilterTaps = frame.shadowFilterTaps;
        sink.emit(Command::setStandardFrameUniforms(std::move(fu)));
    }

    auto ou = buildStandardObjectUniforms(model, mat, lighting, sink);
    sink.emit(Command::setStandardObjectUniforms(std::move(ou)));

    // Bind material texture.
    sink.emit(Command::bindTexture2D(materialTexture, TextureUnit));

    // Vertex buffer.
    sink.emit(Command::bufferData(
        BufferTarget::Array,
        handles.vertexBuffer,
        vertices,
        nVertices * sizeof(RenIVertex),
        BufferUsage::StreamDraw));
    sink.emit(Command::bindBuffer(BufferTarget::Array, handles.vertexBuffer));
    sink.emit(Command::enableVertexAttribPointer(
        handles.posAttr, 3, BackendVertexAttribType::Float, false, sizeof(RenIVertex), 0));
    sink.emit(Command::enableVertexAttribPointer(
        handles.uvAttr, 2, BackendVertexAttribType::Float, false, sizeof(RenIVertex),
        sizeof(RenIVertex) - 2 * sizeof(float)));
    sink.emit(Command::enableVertexAttribPointer(
        handles.colAttr, 4, BackendVertexAttribType::UnsignedByte, true, sizeof(RenIVertex),
        3 * sizeof(float) + sizeof(uint32_t)));

    // Normal buffer (GPU lighting).
    if (lighting.enabled && expandedNormals)
    {
        sink.emit(Command::bufferData(
            BufferTarget::Array,
            handles.normalBuffer,
            expandedNormals,
            nVertices * 3 * sizeof(float),
            BufferUsage::StreamDraw));
        sink.emit(Command::bindBuffer(BufferTarget::Array, handles.normalBuffer));
        sink.emit(Command::enableVertexAttribPointer(
            handles.normalAttr, 3, BackendVertexAttribType::Float, false, 3 * sizeof(float), 0));

        // Per-vertex material buffers.
        if (lighting.hasPerVertexMaterials && expandedVtxDiffuse && expandedVtxAmbient && expandedVtxEmissive)
        {
            sink.emit(Command::bufferData(
                BufferTarget::Array,
                handles.vtxDiffuseBuffer,
                expandedVtxDiffuse,
                nVertices * 3 * sizeof(float),
                BufferUsage::StreamDraw));
            sink.emit(Command::bindBuffer(BufferTarget::Array, handles.vtxDiffuseBuffer));
            sink.emit(Command::enableVertexAttribPointer(
                handles.vtxDiffuseAttr, 3, BackendVertexAttribType::Float, false, 3 * sizeof(float), 0));

            sink.emit(Command::bufferData(
                BufferTarget::Array,
                handles.vtxAmbientBuffer,
                expandedVtxAmbient,
                nVertices * 3 * sizeof(float),
                BufferUsage::StreamDraw));
            sink.emit(Command::bindBuffer(BufferTarget::Array, handles.vtxAmbientBuffer));
            sink.emit(Command::enableVertexAttribPointer(
                handles.vtxAmbientAttr, 3, BackendVertexAttribType::Float, false, 3 * sizeof(float), 0));

            sink.emit(Command::bufferData(
                BufferTarget::Array,
                handles.vtxEmissiveBuffer,
                expandedVtxEmissive,
                nVertices * 3 * sizeof(float),
                BufferUsage::StreamDraw));
            sink.emit(Command::bindBuffer(BufferTarget::Array, handles.vtxEmissiveBuffer));
            sink.emit(Command::enableVertexAttribPointer(
                handles.vtxEmissiveAttr, 3, BackendVertexAttribType::Float, false, 3 * sizeof(float), 0));
        }
    }
}

void DrawCallFactory::emitStandard3DDraw(
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
    CommandSink& sink)
{
    emitStandard3DCommon(handles, frame, frameUniformsDirty, model, mat, lighting,
        materialTexture, vertices, nVertices, expandedNormals, expandedVtxDiffuse, expandedVtxAmbient, expandedVtxEmissive, sink);

    sink.emit(Command::draw(topology, 0, nVertices));
}

void DrawCallFactory::emitStandard3DDrawIndexed(
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
    CommandSink& sink)
{
    emitStandard3DCommon(handles, frame, frameUniformsDirty, model, mat, lighting,
        materialTexture, vertices, nVertices, expandedNormals, expandedVtxDiffuse, expandedVtxAmbient, expandedVtxEmissive, sink);

    // Index buffer.
    sink.emit(Command::bufferData(
        BufferTarget::ElementArray,
        handles.elementBuffer,
        indices,
        nIndices * sizeof(unsigned short),
        BufferUsage::StreamDraw));

    sink.emit(Command::drawIndexed(topology, BackendIndexType::UnsignedShort, nIndices));
}

void DrawCallFactory::emitBillboardDrawIndexed(
    const BillboardPipelineHandles& handles,
    const BillboardUniforms& uniforms,
    bool uniformsDirty,
    BackendTextureHandle texture,
    const RenIVertex* vertices,
    std::size_t nVertices,
    const VertexIdx* indices,
    std::size_t nIndices,
    PrimitiveTopology topology,
    CommandSink& sink)
{
    sink.emit(Command::bindPipeline(handles.pipelineId));
    sink.emit(Command::bindTexture2D(texture, 0));

    if (uniformsDirty)
        sink.emit(Command::setBillboardUniforms(uniforms));

    sink.emit(Command::bufferData(
        BufferTarget::Array,
        handles.vertexBuffer,
        vertices,
        nVertices * sizeof(RenIVertex),
        BufferUsage::StreamDraw));
    sink.emit(Command::bindBuffer(BufferTarget::Array, handles.vertexBuffer));
    sink.emit(Command::enableVertexAttribPointer(
        handles.posAttr, 4, BackendVertexAttribType::Float, false, sizeof(RenIVertex), 0));
    sink.emit(Command::enableVertexAttribPointer(
        handles.uvAttr, 2, BackendVertexAttribType::Float, false, sizeof(RenIVertex),
        sizeof(RenIVertex) - 2 * sizeof(float)));
    sink.emit(Command::enableVertexAttribPointer(
        handles.colAttr, 4, BackendVertexAttribType::UnsignedByte, true, sizeof(RenIVertex),
        3 * sizeof(float) + sizeof(uint32_t)));

    sink.emit(Command::bufferData(
        BufferTarget::ElementArray,
        handles.elementBuffer,
        indices,
        nIndices * sizeof(unsigned short),
        BufferUsage::StreamDraw));

    sink.emit(Command::drawIndexed(topology, BackendIndexType::UnsignedShort, nIndices));
}

void DrawCallFactory::emitShadowDepthDrawIndexed(
    const ShadowDepthPipelineHandles& handles,
    const ShadowDepthUniforms& uniforms,
    const RenIVertex* vertices,
    std::size_t nVertices,
    const VertexIdx* indices,
    std::size_t nIndices,
    PrimitiveTopology topology,
    CommandSink& sink)
{
    sink.emit(Command::setShadowDepthUniforms(uniforms));

    sink.emit(Command::bufferData(
        BufferTarget::Array,
        handles.vertexBuffer,
        vertices,
        nVertices * sizeof(RenIVertex),
        BufferUsage::StreamDraw));
    sink.emit(Command::bindBuffer(BufferTarget::Array, handles.vertexBuffer));
    sink.emit(Command::enableVertexAttribPointer(
        handles.posAttr, 3, BackendVertexAttribType::Float, false, sizeof(RenIVertex), 0));

    sink.emit(Command::bufferData(
        BufferTarget::ElementArray,
        handles.elementBuffer,
        indices,
        nIndices * sizeof(unsigned short),
        BufferUsage::StreamDraw));

    sink.emit(Command::drawIndexed(topology, BackendIndexType::UnsignedShort, nIndices));
}

} // namespace Ren
