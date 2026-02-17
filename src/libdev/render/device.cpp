/*
 * D E V I C E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

// TODO: This god class needs refactoring.
#include "render/device.hpp"
#include "render/TextOptions.hpp"

#include <array>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>

#include <string>
#include "base/diag.hpp"
#include "ctl/vector.hpp"
#include "system/pathname.hpp"

#include "mathex/point3d.hpp"
#include "mathex/point2d.hpp"
#include "mathex/quad3d.hpp"
#include "mathex/coordsys.hpp"
#include "mathex/random.hpp"

#include "render/Font.hpp"
#include "render/colour.hpp"
#include "render/camera.hpp"
#include "render/display.hpp"
#include "render/texmgr.hpp"
#include "render/texture.hpp"
#include "render/surface.hpp"
#include "render/surfmgr.hpp"
#include "render/capable.hpp"
#include "render/stats.hpp"
#include "render/material.hpp"
#include "render/mesh.hpp"
#include "render/drivsel.hpp"

#include "render/internal/DrawCallFactory.hpp"
#include "render/internal/IRenderBackend.hpp"
#include "render/internal/devicei.hpp"
#include "render/internal/matmgr.hpp"
#include "render/internal/internal.hpp"
#include "render/internal/debug.hpp"
#include "render/internal/driver.hpp"
#include "render/internal/glmath.hpp"
#include "render/internal/vpmap.hpp"
#include "render/internal/polysord.hpp"
#include "render/internal/polysorp.hpp"
#include "render/internal/matbody.hpp"
#include "render/internal/matmgr.hpp"
#include "render/internal/illumine.hpp"
#include "render/internal/nonmmx.hpp"
#include "render/internal/displayi.hpp"
#include "render/internal/surfmgri.hpp"
#include "render/internal/capablei.hpp"
#include "render/internal/drivi.hpp"
#include "render/internal/vtxdata.hpp"
#include "render/internal/colpack.hpp"
#include "render/render.hpp"
#include "render/internal/trigroup.hpp"
#include "render/internal/surfbody.hpp"
#include "render/RenderUtils.hpp"
#include "render/RenderVariables.hpp"
#include "system/winapi.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

static std::array<float, 16> toFloatArray(const glm::mat4& m)
{
    std::array<float, 16> result{};
    std::memcpy(result.data(), glm::value_ptr(m), sizeof(float) * 16);
    return result;
}

static Ren::BackendTextureHandle resolveTextureHandle(Ren::TexId id)
{
    if (id == Ren::NullTexId)
        return {};

    const RenISurfBody* surfBody = RenSurfaceManager::instance().impl().getSurfaceBody(id);
    if (surfBody && !surfBody->isEmpty())
        return surfBody->nativeTextureHandle();

    return {};
}

#define CB_RENDEVICE_DEPIMPL_GL()                                                                                      \
    PRE(pImpl_);                                                                                                       \
    CB_DEPIMPL_AUTO(gui2D_);                                                                                           \
    CB_DEPIMPL_AUTO(standard_);                                                                                        \
    CB_DEPIMPL_AUTO(billboard_);                                                                                       \
    CB_DEPIMPL_AUTO(shadowDepth_);                                                                                     \
    CB_DEPIMPL_AUTO(geometryRenderPass_);                                                                              \
    CB_DEPIMPL_AUTO(uiRenderPass_);                                                                                    \
    CB_DEPIMPL_AUTO(shadowRenderPass_);                                                                                \
    CB_DEPIMPL_AUTO(shadowFramebuffer_);                                                                               \
    CB_DEPIMPL_AUTO(shadowDepthTexture_);                                                                              \
    CB_DEPIMPL_AUTO(shadowNearFramebuffer_);                                                                           \
    CB_DEPIMPL_AUTO(shadowNearDepthTexture_);                                                                          \
    CB_DEPIMPL_AUTO(vertexBuffer2D_);                                                                                  \
    CB_DEPIMPL_AUTO(vertexDataBuffer_);                                                                                \
    CB_DEPIMPL_AUTO(normalBuffer_);                                                                                    \
    CB_DEPIMPL_AUTO(vtxDiffuseBuffer_);                                                                                \
    CB_DEPIMPL_AUTO(vtxAmbientBuffer_);                                                                                \
    CB_DEPIMPL_AUTO(vtxEmissiveBuffer_);                                                                               \
    CB_DEPIMPL_AUTO(elementBuffer_);                                                                                   \
    CB_DEPIMPL_AUTO(vertexDataBufferBillboard_);                                                                       \
    CB_DEPIMPL_AUTO(elementBufferBillboard_);                                                                          \
    CB_DEPIMPL_AUTO(offscreenFramebuffer_);                                                                            \
    CB_DEPIMPL_AUTO(postProcess_);                                                                                     \
    CB_DEPIMPL_AUTO(postProcessFBO_);                                                                                  \
    CB_DEPIMPL_AUTO(postProcessColorTexture_);                                                                         \
    CB_DEPIMPL_AUTO(postProcessQuadVBO_);                                                                              \
    CB_DEPIMPL_AUTO(postProcessWidth_);                                                                                \
    CB_DEPIMPL_AUTO(postProcessHeight_);                                                                               \
    CB_DEPIMPL_AUTO(postProcessReady_);

RenDevice::RenDevice(RenDisplay* display)
    : pImpl_(new RenIDeviceImpl(display, this))
    , standardUniformsDirty_(true)
    , billboardUniformsDirty_(true)
{
    PRE(display);

    pImpl_->display_ = display;
    pImpl_->projViewMatrix_ = new glm::mat4;
    pImpl_->coplanarSorter_ = new RenIPriorityPostSorter;
    pImpl_->normalAlphaSorter_ = new RenIDepthPostSorter;
    pImpl_->alphaSorter_ = nullptr;
    pImpl_->doingBackground_ = false;
    pImpl_->caps_ = nullptr;
    pImpl_->stats_ = new RenStats;
    pImpl_->fogOn_ = false;
    pImpl_->fogStart_ = 1;
    pImpl_->fogEnd_ = 500;
    pImpl_->fogDensity_ = 0.05;
    fogParams_ = glm::vec3(pImpl_->fogStart_, pImpl_->fogEnd_, pImpl_->fogDensity_);
    pImpl_->interference_ = 0;
    pImpl_->staticOn_ = false;
    pImpl_->currentCamera_ = nullptr;
    pImpl_->vpMapping_ = nullptr;
    pImpl_->rendering_ = false;
    pImpl_->rendering3D_ = false;
    pImpl_->rendering2D_ = false;
    pImpl_->illuminator_ = nullptr; // viewport must be created 1st, see below
    pImpl_->surfBackBuf_ = nullptr;
    pImpl_->surfFrontBuf_ = nullptr;
    pImpl_->surfacesMayBeLost_ = 0;
    pImpl_->shouldBeginScene_ = true;
    pImpl_->antiAliasingOn_ = true;

}

void RenDevice::renderScreenspace(
    const RenIVertex* vertices,
    const size_t nVertices,
    const RenMaterial& mat,
    Ren::PrimitiveTopology topology,
    const int targetW,
    const int targetH)
{
    renderScreenspace(vertices, nVertices, topology, targetW, targetH, mat.texture().handle());
}

bool RenDevice::initialize()
{
    PRE(Ren::initialised());
    PRE(MexCoordSystem::instance().isSet());

    CB_RENDEVICE_DEPIMPL_GL();
    CB_DEPIMPL_AUTO(backend_);

    // There are two alpha sorters.  Only one is in use at a time.  The switch
    // is made by changing this pointer.
    pImpl_->alphaSorter_ = pImpl_->normalAlphaSorter_;

    fogColour(RenColour::white());

    if (!initializeContext())
    {
        // TBD: What shall i do then ?
        ASSERT(false, "could not create surfaces ");
        return false;
    }

    initializeDisplay();

    // GUI 2D pipeline
    {
        Ren::PipelineDesc desc;
        desc.vertexShader = "2DShading";
        desc.fragmentShader = "2DShading";
        desc.vertexAttributes = {
            { "vertexPosition_screenspace", 2, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex), 0 },
            { "vertexUV", 2, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex), sizeof(RenIVertex) - 2 * sizeof(float) },
            { "vertexColor", 4, Ren::BackendVertexAttribType::UnsignedByte, true, sizeof(RenIVertex), 3 * sizeof(float) + sizeof(uint32_t) },
        };
        desc.uniformNames = { "uScreenspace", "uTextureSampler" };
        gui2D_.id = backend_->createPipeline(desc);
        gui2D_.posAttr = backend_->pipelineAttribLocation(gui2D_.id, "vertexPosition_screenspace");
        gui2D_.uvAttr = backend_->pipelineAttribLocation(gui2D_.id, "vertexUV");
        gui2D_.colAttr = backend_->pipelineAttribLocation(gui2D_.id, "vertexColor");
        gui2D_.screenspaceUniform = backend_->pipelineUniformLocation(gui2D_.id, "uScreenspace");
        gui2D_.texSamplerUniform = backend_->pipelineUniformLocation(gui2D_.id, "uTextureSampler");
    }

    vertexBuffer2D_ = backend_->createBuffer();

    // Standard 3D pipeline
    {
        Ren::PipelineDesc desc;
        desc.vertexShader = "StandardShading";
        desc.fragmentShader = "StandardShading";
        desc.vertexAttributes = {
            { "vertexPosition_modelspace", 3, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex), 0 },
            { "vertexUV", 2, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex), sizeof(RenIVertex) - 2 * sizeof(float) },
            { "vertexColor", 4, Ren::BackendVertexAttribType::UnsignedByte, true, sizeof(RenIVertex), 3 * sizeof(float) + sizeof(uint32_t) },
            { "vertexNormal", 3, Ren::BackendVertexAttribType::Float, false, 3 * sizeof(float), 0 },
            { "vtxDiffuse", 3, Ren::BackendVertexAttribType::Float, false, 3 * sizeof(float), 0 },
            { "vtxAmbient", 3, Ren::BackendVertexAttribType::Float, false, 3 * sizeof(float), 0 },
            { "vtxEmissive", 3, Ren::BackendVertexAttribType::Float, false, 3 * sizeof(float), 0 },
        };
        desc.uniformNames = {
            "uM", "uV", "uP", "uFogColour", "uFogParams", "uTextureSampler2",
            "uGpuLighting", "uLightDir", "uLightColor", "uAmbientColor",
            "uMatDiffuse", "uMatAmbient", "uMatEmissive", "uFilter",
            "uHasVtxMaterials",
            "uNumPointLights",
            "uPointLightPos", "uPointLightColor", "uPointLightRange", "uPointLightAtten", "uPointLightOmni",
            "uShadowMap", "uLightSpaceMatrix", "uShadowEnabled", "uShadowStrength",
            "uShadowMapNear", "uLightSpaceMatrixNear", "uShadowSplitDistance",
        };
        standard_.id = backend_->createPipeline(desc);
        standard_.posAttr = backend_->pipelineAttribLocation(standard_.id, "vertexPosition_modelspace");
        standard_.uvAttr = backend_->pipelineAttribLocation(standard_.id, "vertexUV");
        standard_.colAttr = backend_->pipelineAttribLocation(standard_.id, "vertexColor");
        standard_.normalAttr = backend_->pipelineAttribLocation(standard_.id, "vertexNormal");
        standard_.vtxDiffuseAttr = backend_->pipelineAttribLocation(standard_.id, "vtxDiffuse");
        standard_.vtxAmbientAttr = backend_->pipelineAttribLocation(standard_.id, "vtxAmbient");
        standard_.vtxEmissiveAttr = backend_->pipelineAttribLocation(standard_.id, "vtxEmissive");
        spdlog::info("Standard pipeline attrib locations: pos={} uv={} col={} normal={} vtxDif={} vtxAmb={} vtxEmi={}",
            standard_.posAttr.value(), standard_.uvAttr.value(), standard_.colAttr.value(),
            standard_.normalAttr.value(), standard_.vtxDiffuseAttr.value(), standard_.vtxAmbientAttr.value(),
            standard_.vtxEmissiveAttr.value());
        standard_.modelUniform = backend_->pipelineUniformLocation(standard_.id, "uM");
        standard_.viewUniform = backend_->pipelineUniformLocation(standard_.id, "uV");
        standard_.projUniform = backend_->pipelineUniformLocation(standard_.id, "uP");
        standard_.fogColourUniform = backend_->pipelineUniformLocation(standard_.id, "uFogColour");
        standard_.fogParamsUniform = backend_->pipelineUniformLocation(standard_.id, "uFogParams");
        standard_.texSamplerUniform = backend_->pipelineUniformLocation(standard_.id, "uTextureSampler2");
        standard_.gpuLightingUniform = backend_->pipelineUniformLocation(standard_.id, "uGpuLighting");
        standard_.lightDirUniform = backend_->pipelineUniformLocation(standard_.id, "uLightDir");
        standard_.lightColorUniform = backend_->pipelineUniformLocation(standard_.id, "uLightColor");
        standard_.ambientColorUniform = backend_->pipelineUniformLocation(standard_.id, "uAmbientColor");
        standard_.matDiffuseUniform = backend_->pipelineUniformLocation(standard_.id, "uMatDiffuse");
        standard_.matAmbientUniform = backend_->pipelineUniformLocation(standard_.id, "uMatAmbient");
        standard_.matEmissiveUniform = backend_->pipelineUniformLocation(standard_.id, "uMatEmissive");
        standard_.filterUniform = backend_->pipelineUniformLocation(standard_.id, "uFilter");
        standard_.hasVtxMaterialsUniform = backend_->pipelineUniformLocation(standard_.id, "uHasVtxMaterials");
        standard_.numPointLightsUniform = backend_->pipelineUniformLocation(standard_.id, "uNumPointLights");
        standard_.pointLightPosUniform = backend_->pipelineUniformLocation(standard_.id, "uPointLightPos");
        standard_.pointLightColorUniform = backend_->pipelineUniformLocation(standard_.id, "uPointLightColor");
        standard_.pointLightRangeUniform = backend_->pipelineUniformLocation(standard_.id, "uPointLightRange");
        standard_.pointLightAttenUniform = backend_->pipelineUniformLocation(standard_.id, "uPointLightAtten");
        standard_.pointLightOmniUniform = backend_->pipelineUniformLocation(standard_.id, "uPointLightOmni");
        standard_.shadowMapUniform = backend_->pipelineUniformLocation(standard_.id, "uShadowMap");
        standard_.lightSpaceMatrixUniform = backend_->pipelineUniformLocation(standard_.id, "uLightSpaceMatrix");
        standard_.shadowEnabledUniform = backend_->pipelineUniformLocation(standard_.id, "uShadowEnabled");
        standard_.shadowStrengthUniform = backend_->pipelineUniformLocation(standard_.id, "uShadowStrength");
        standard_.shadowMapNearUniform = backend_->pipelineUniformLocation(standard_.id, "uShadowMapNear");
        standard_.lightSpaceMatrixNearUniform = backend_->pipelineUniformLocation(standard_.id, "uLightSpaceMatrixNear");
        standard_.shadowSplitDistanceUniform = backend_->pipelineUniformLocation(standard_.id, "uShadowSplitDistance");
    }

    vertexDataBuffer_ = backend_->createBuffer();
    normalBuffer_ = backend_->createBuffer();
    vtxDiffuseBuffer_ = backend_->createBuffer();
    vtxAmbientBuffer_ = backend_->createBuffer();
    vtxEmissiveBuffer_ = backend_->createBuffer();
    elementBuffer_ = backend_->createBuffer();

    // Billboard pipeline
    {
        Ren::PipelineDesc desc;
        desc.vertexShader = "BillboardShading";
        desc.fragmentShader = "2DShading";
        desc.vertexAttributes = {
            { "vertexPosition_Billboard", 4, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex), 0 },
            { "vertexUV", 2, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex), sizeof(RenIVertex) - 2 * sizeof(float) },
            { "vertexColor", 4, Ren::BackendVertexAttribType::UnsignedByte, true, sizeof(RenIVertex), 3 * sizeof(float) + sizeof(uint32_t) },
        };
        desc.uniformNames = { "uVP", "uTextureSampler" };
        billboard_.id = backend_->createPipeline(desc);
        billboard_.posAttr = backend_->pipelineAttribLocation(billboard_.id, "vertexPosition_Billboard");
        billboard_.uvAttr = backend_->pipelineAttribLocation(billboard_.id, "vertexUV");
        billboard_.colAttr = backend_->pipelineAttribLocation(billboard_.id, "vertexColor");
        billboard_.viewProjUniform = backend_->pipelineUniformLocation(billboard_.id, "uVP");
        billboard_.texSamplerUniform = backend_->pipelineUniformLocation(billboard_.id, "uTextureSampler");
    }

    // Shadow depth pipeline
    {
        Ren::PipelineDesc desc;
        desc.vertexShader = "ShadowDepth";
        desc.fragmentShader = "ShadowDepth";
        desc.vertexAttributes = {
            { "vertexPosition_modelspace", 3, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex), 0 },
        };
        desc.uniformNames = { "uLightSpaceMatrix", "uM" };
        shadowDepth_.id = backend_->createPipeline(desc);
        shadowDepth_.posAttr = backend_->pipelineAttribLocation(shadowDepth_.id, "vertexPosition_modelspace");
        shadowDepth_.lightSpaceMatrixUniform = backend_->pipelineUniformLocation(shadowDepth_.id, "uLightSpaceMatrix");
        shadowDepth_.modelUniform = backend_->pipelineUniformLocation(shadowDepth_.id, "uM");
    }

    const std::array<Ren::PipelineId, 4> pipelineIDs = { gui2D_.id, standard_.id, billboard_.id, shadowDepth_.id };
    if (std::ranges::any_of(pipelineIDs, [](Ren::PipelineId value) { return value == 0; }))
        return false;

    vertexDataBufferBillboard_ = backend_->createBuffer();
    elementBufferBillboard_ = backend_->createBuffer();

    // Geometry render pass: clear color+depth
    {
        Ren::RenderPassDesc desc;
        desc.colorAttachment.loadOp = Ren::LoadOp::Clear;
        desc.colorAttachment.storeOp = Ren::StoreOp::Store;
        desc.hasDepthAttachment = true;
        desc.depthAttachment.loadOp = Ren::LoadOp::Clear;
        desc.depthAttachment.storeOp = Ren::StoreOp::Store;
        geometryRenderPass_ = backend_->createRenderPass(desc);
    }

    // UI render pass: load existing color, no depth
    {
        Ren::RenderPassDesc desc;
        desc.colorAttachment.loadOp = Ren::LoadOp::Load;
        desc.colorAttachment.storeOp = Ren::StoreOp::Store;
        uiRenderPass_ = backend_->createRenderPass(desc);
    }

    // Shadow render pass: depth-only, clear depth
    {
        Ren::RenderPassDesc desc;
        desc.hasColorAttachment = false;
        desc.hasDepthAttachment = true;
        desc.depthAttachment.loadOp = Ren::LoadOp::Clear;
        desc.depthAttachment.storeOp = Ren::StoreOp::Store;
        shadowRenderPass_ = backend_->createRenderPass(desc);
    }

    // Shadow map resources — two cascades: near (high-res) and far (lower-res).
    {
        const int nearSz = RenIDeviceImpl::ShadowMapSizeNear;
        shadowNearDepthTexture_ = backend_->createTexture2D();
        backend_->textureStorage2D(shadowNearDepthTexture_, nearSz, nearSz, Ren::TextureFormat::Depth16);
        backend_->textureSetMinMagFilter(shadowNearDepthTexture_, Ren::TextureFilter::Nearest, Ren::TextureFilter::Nearest);
        backend_->textureSetWrap(shadowNearDepthTexture_, Ren::TextureWrap::ClampToEdge, Ren::TextureWrap::ClampToEdge);
        shadowNearFramebuffer_ = backend_->createFramebuffer();
        backend_->framebufferAttachDepthTexture(shadowNearFramebuffer_, shadowNearDepthTexture_);

        const int farSz = RenIDeviceImpl::ShadowMapSizeFar;
        shadowDepthTexture_ = backend_->createTexture2D();
        backend_->textureStorage2D(shadowDepthTexture_, farSz, farSz, Ren::TextureFormat::Depth16);
        backend_->textureSetMinMagFilter(shadowDepthTexture_, Ren::TextureFilter::Nearest, Ren::TextureFilter::Nearest);
        backend_->textureSetWrap(shadowDepthTexture_, Ren::TextureWrap::ClampToEdge, Ren::TextureWrap::ClampToEdge);
        shadowFramebuffer_ = backend_->createFramebuffer();
        backend_->framebufferAttachDepthTexture(shadowFramebuffer_, shadowDepthTexture_);
    }

    // Post-process pipeline (tone mapping)
    {
        Ren::PipelineDesc desc;
        desc.vertexShader = "PostProcess";
        desc.fragmentShader = "PostProcess";
        desc.vertexAttributes = {
            { "vertexPosition", 2, Ren::BackendVertexAttribType::Float, false, 4 * sizeof(float), 0 },
            { "vertexUV", 2, Ren::BackendVertexAttribType::Float, false, 4 * sizeof(float), 2 * sizeof(float) },
        };
        desc.uniformNames = { "uSceneTexture", "uExposure" };
        postProcess_.id = backend_->createPipeline(desc);
        if (postProcess_.id != 0)
        {
            postProcess_.posAttr = backend_->pipelineAttribLocation(postProcess_.id, "vertexPosition");
            postProcess_.uvAttr = backend_->pipelineAttribLocation(postProcess_.id, "vertexUV");
            postProcess_.sceneTextureUniform = backend_->pipelineUniformLocation(postProcess_.id, "uSceneTexture");
            postProcess_.exposureUniform = backend_->pipelineUniformLocation(postProcess_.id, "uExposure");
        }
    }

    // Post-process offscreen FBO (RGBA16F color + depth renderbuffer).
    if (postProcess_.id != 0)
    {
        const RenDisplay::Mode mode = pImpl_->display_->currentMode();
        const int w = mode.width();
        const int h = mode.height();

        postProcessColorTexture_ = backend_->createTexture2D();
        postProcessFBO_ = backend_->createFramebuffer();

        // Try RGBA16F first, fall back to RGBA8 if FBO is incomplete.
        Ren::TextureFormat colorFormat = Ren::TextureFormat::RGBA16F;
        backend_->textureStorage2D(postProcessColorTexture_, w, h, colorFormat);
        backend_->textureSetMinMagFilter(
            postProcessColorTexture_, Ren::TextureFilter::Linear, Ren::TextureFilter::Linear);
        backend_->textureSetWrap(
            postProcessColorTexture_, Ren::TextureWrap::ClampToEdge, Ren::TextureWrap::ClampToEdge);
        backend_->framebufferAttachColorTexture(postProcessFBO_, postProcessColorTexture_);
        backend_->framebufferAttachDepthRenderbuffer(postProcessFBO_, w, h);

        if (!backend_->isFramebufferComplete(postProcessFBO_))
        {
            spdlog::warn("Post-process: RGBA16F FBO incomplete, falling back to RGBA8");
            colorFormat = Ren::TextureFormat::RGBA8_UNorm;
            backend_->destroyTexture2D(postProcessColorTexture_);
            postProcessColorTexture_ = backend_->createTexture2D();
            backend_->textureStorage2D(postProcessColorTexture_, w, h, colorFormat);
            backend_->textureSetMinMagFilter(
                postProcessColorTexture_, Ren::TextureFilter::Linear, Ren::TextureFilter::Linear);
            backend_->textureSetWrap(
                postProcessColorTexture_, Ren::TextureWrap::ClampToEdge, Ren::TextureWrap::ClampToEdge);
            backend_->framebufferAttachColorTexture(postProcessFBO_, postProcessColorTexture_);
        }

        const bool fboReady = backend_->isFramebufferComplete(postProcessFBO_);
        const char* fmtName = (colorFormat == Ren::TextureFormat::RGBA16F) ? "RGBA16F" : "RGBA8";

        postProcessQuadVBO_ = backend_->createBuffer();

        // Fullscreen quad: 2 triangles covering [-1,1] with UV [0,1].
        const float quadVertices[] = {
            // pos.x, pos.y, uv.x, uv.y
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
        };
        backend_->bufferData(
            Ren::BufferTarget::Array, postProcessQuadVBO_,
            sizeof(quadVertices), quadVertices, Ren::BufferUsage::StreamDraw);

        postProcessWidth_ = w;
        postProcessHeight_ = h;
        postProcessReady_ = fboReady;

        spdlog::info("Post-process: {}x{} {} FBO, pipeline={}, ready={}",
            w, h, fmtName, postProcess_.id, postProcessReady_);
    }

    // Prepare framebuffer for offscreen rendering
    offscreenFramebuffer_ = backend_->createFramebuffer();

    pImpl_->illuminator_ = new RenINonMMXIlluminator(pImpl_);

    RenStats* pStats = statistics();
    bool statsShown = true;

    if (pStats == nullptr)
    {
        statsShown = false;
    }
    else
    {
        statsShown = pStats->shown();
        pStats->hide();
    }

    if (statsShown)
    {
        pStats->show();
    }

    return true;
}

RenDevice::~RenDevice()
{
    // Note: the pImpl_->alphaSorter_ variable is set to point to either nullptr or to
    // pImpl_->normalAlphaSorter_, depending on what's being rendered.  It should
    // *not* be deleted.
    if (pImpl_->coplanarSorter_)
    {
        delete pImpl_->coplanarSorter_;
    }
    if (pImpl_->normalAlphaSorter_)
    {
        delete pImpl_->normalAlphaSorter_;
    }
    if (pImpl_->caps_)
    {
        delete pImpl_->caps_;
    }
    if (pImpl_->stats_)
    {
        delete pImpl_->stats_;
    }
    if (pImpl_->vpMapping_)
    {
        delete pImpl_->vpMapping_;
    }
    if (pImpl_->projViewMatrix_)
    {
        delete pImpl_->projViewMatrix_;
    }
    if (pImpl_->illuminator_)
    {
        delete pImpl_->illuminator_;
    }
    if (pImpl_->surfBackBuf_)
    {
        delete pImpl_->surfBackBuf_;
    }
    if (pImpl_->surfFrontBuf_)
    {
        delete pImpl_->surfFrontBuf_;
    }
    CB_RENDEVICE_DEPIMPL_GL();
    CB_DEPIMPL_AUTO(backend_);

    backend_->releasePipeline(gui2D_.id);
    backend_->releasePipeline(standard_.id);
    backend_->releasePipeline(billboard_.id);
    backend_->releasePipeline(shadowDepth_.id);

    backend_->releaseRenderPass(geometryRenderPass_);
    backend_->releaseRenderPass(uiRenderPass_);
    backend_->releaseRenderPass(shadowRenderPass_);

    backend_->destroyTexture2D(shadowDepthTexture_);
    backend_->releaseFramebuffer(shadowFramebuffer_);
    backend_->destroyTexture2D(shadowNearDepthTexture_);
    backend_->releaseFramebuffer(shadowNearFramebuffer_);

    backend_->releaseBuffer(vertexBuffer2D_);
    backend_->releaseBuffer(vertexDataBuffer_);
    backend_->releaseBuffer(normalBuffer_);
    backend_->releaseBuffer(vtxDiffuseBuffer_);
    backend_->releaseBuffer(vtxAmbientBuffer_);
    backend_->releaseBuffer(vtxEmissiveBuffer_);
    backend_->releaseBuffer(elementBuffer_);
    backend_->releaseBuffer(vertexDataBufferBillboard_);
    backend_->releaseBuffer(elementBufferBillboard_);
    backend_->releaseFramebuffer(offscreenFramebuffer_);

    backend_->shutdown();

    delete pImpl_;

    pImpl_ = nullptr;

    RenDevice*& currentDeviceRef = RenIDeviceImpl::current();
    if (currentDeviceRef == this)
    {
        currentDeviceRef = nullptr;
        RenIDeviceImpl::currentPimpl() = nullptr;
    }
}

void RenDevice::reset()
{
}

void RenDevice::clearAllSurfaces(const RenColour& colour)
{
    PRE(!rendering3D() && !rendering2D());

    RENDER_STREAM(
        "Inside RenDevice::clearAllSurfaces( RenColour RGBA: " << colour.r() << ", " << colour.g() << ", " << colour.b()
                                                               << ", " << colour.a() << " )" << std::endl);
    pImpl_->background_ = colour;
    clearAllSurfaces();
}

void RenDevice::clearAllSurfaces()
{
    PRE(!rendering3D() && !rendering2D());
    PRE(pImpl_->display_);

    const RenDisplay::Mode mode = pImpl_->display_->currentMode();

    // As in startFrame, if the client hasn't set a viewport, then default
    // to full-screen.
    if (!pImpl_->vpMapping_)
        setViewport(0, 0, mode.width(), mode.height());

    clearAll2D_ = true;

    ASSERT(pImpl_->vpMapping_, "No viewport set; there should be a default.");

    // Having 100% reset the screen, any areas saved under the cursor
    // are now irrelevant.
    pImpl_->display_->displayImpl().discardCursorSaves();
}

void RenDevice::initializeDisplay()
{
    PRE(pImpl_->display_);

    // Check the capabilities of the device, so that we can selectively enable
    // special effects such as alpha textures.
    if (pImpl_->caps_)
    {
        delete pImpl_->caps_;
    }
    pImpl_->caps_ = new RenCapabilities(this, true);
    RENDER_STREAM(*pImpl_->caps_);

    spdlog::info("Initializing OpenGL context...");
    // we have to copy the gamma correction into display in order to implement
    // asserts in gammaCorrection() methods, it's ugly.
    pImpl_->display_->supportsGammaCorrection(pImpl_->caps_->supportsGammaCorrection());

    // We assume that some form of texture transparency is supported.  All
    // reasonable games apps. require transparency.
    ASSERT(
        pImpl_->caps_->supportsColourKey() || pImpl_->caps_->supportsTextureAlpha(),
        runtime_error("No transparency supported by D3D."));

    createViewport();
    RenDisplay::Mode mode = pImpl_->display_->currentMode();
    setViewport(0, 0, mode.width(), mode.height());
    pImpl_->useDevice(this, RenI::FORCE_UPDATE);
    pImpl_->surfacesMayBeLost_ = 10;

    // Clear the both front and back surfaces just in case they are uninitialised.
    clearAllSurfaces(RenColour::black());
}

bool RenDevice::initializeContext()
{
    CB_DEPIMPL_AUTO(backend_);
    CB_DEPIMPL_AUTO(display_);

    if (backend_->isInitialized())
    {
        backend_->shutdown();
    }
    if (!backend_->initialize(display_->window()))
    {
        spdlog::error("Render backend initialization failed");
        return false;
    }

    if (!setVSync(vsyncEnabled_))
    {
        spdlog::warn("Failed to apply VSync preference ({}) during context creation", vsyncEnabled_);
    }

    return true;
}

bool RenDevice::fitToDisplay(RenDisplay* pDisplay)
{
    PRE(pDisplay);
    if (!initializeContext())
        return false;

    initializeDisplay();
    return true;
}

void RenDevice::createViewport()
{
    backgroundColour(RenColour::black());
}

void RenDevice::setViewport(int left, int top, int width, int height)
{
    PRE(left + width <= windowWidth());
    PRE(top + height <= windowHeight());

    const double ratio = (double)width / height;

    // NB: if the viewport is changed, these values *must* also be updated.
    if (pImpl_->vpMapping_)
    {
        delete pImpl_->vpMapping_;
    }

    pImpl_->vpMapping_ = new RenIViewportMapping(width, height, left, top);

    RENDER_STREAM("Set viewport to (" << left << "," << top << ") " << width << "x" << height << "\n");
}

void RenDevice::clearDisplay(int width, int height)
{
    pImpl_->backend_->clearDisplay(width, height);
}

////////////////////////////////// Frame delineation //////////////////////////

bool RenDevice::startFrame()
{
    PRE(!rendering());
    PRE(!pImpl_->immediateCommandBufferActive());

    // When the Window is activated this flag is set indicating that surface
    // memory could have been lost to another process.
    if (pImpl_->surfacesMayBeLost_)
    {
        // Is the back buffer unavailable for rendering?
        if (!pImpl_->display_->checkForLostSurfaces())
        {
            RENDER_STREAM("Primary or z-buffer surface lost.  Can't render." << std::endl);
            return false;
        }

        // They're fixed, so minus one off this flag. After an activate message is recieved
        // we try 10 times to restore lost surfaces. This is because there doesn't appear
        // to be a windows message that we can use that accurately indicates when we can
        // successfully recover any lost surfaces ( WM_ACTIVATE is the most reliable ).
        --pImpl_->surfacesMayBeLost_;
    }

    if (!pImpl_->shouldBeginScene_)
    {
        RENDER_STREAM("RenDevice::startFrame() == false" << std::endl);
        return false;
    }

    // Only do this after we have checked that rendering can proceed.
    // It needs to occur before RenDisplay::startFrame because the display
    // does blitting which asserts that the frame has started.
    pImpl_->rendering_ = true;

    pImpl_->display_->startFrame();

    // When this method is first called, if the client hasn't set a viewport,
    // then default to full-screen.
    if (!pImpl_->vpMapping_)
    {
        setViewport(0, 0, windowWidth(), windowHeight());
    }

    pImpl_->useDevice(this, RenI::LAZY_UPDATE);
    pImpl_->beginFrameCommandBuffer();

    RENDER_STREAM(std::endl << "RenDevice::startFrame() == true" << std::endl << '{' << std::endl);
    RENDER_INDENT(3);

    DEBUG_RESET_FRAME_TIME;

    POST(idleRendering());
    return true;
}

void RenDevice::start2D()
{
    PRE(idleRendering());

    pImpl_->rendering2D_ = true;

    // Set default rendering state which is appropriate for 2D gui stuff.
    // Assume that rendering ops will use 2D transformed, lit co-ords.  Thus
    // don't initialise lighting or any projection/view matrices.
    disableFog();

    pImpl_->alphaSorter_ = nullptr;
    pImpl_->illuminator_->filter(RenColour::white());

    CB_RENDEVICE_DEPIMPL_GL();
    recordCommand(Ren::Command::beginRenderPass(uiRenderPass_));

    const double now = DEBUG_FRAME_TIME;
    RENDER_STREAM("RenDevice::start2D() at " << now << "(ms)\n{\n");
    RENDER_INDENT(3);

    POST(rendering2D());
}

void RenDevice::end2D()
{
    PRE(rendering2D());

    // Needs to go here because smacker animations do not call RenDevice's frame lifecycle methods... -_-
    if (clearAll2D_)
    {
        CB_DEPIMPL_AUTO(background_);
        CB_DEPIMPL_AUTO(backend_);

        using ClearFlag = Ren::BackendClearFlag;
        Ren::BackendCommand command = Ren::Command::clear(background_, backendClearMask(ClearFlag::Colour));
        recordCommand(std::move(command));
        clearAll2D_ = false;
    }

    recordCommand(Ren::Command::endRenderPass());

    pImpl_->rendering2D_ = false;

    RENDER_STREAM("pImpl_->shouldBeginScene_ == " << pImpl_->shouldBeginScene_ << std::endl);
    RENDER_INDENT(-3);

    const double now = DEBUG_FRAME_TIME;
    RENDER_STREAM("RenDevice::end2D() at " << now << "(ms)\n");
    RENDER_STREAM('}' << std::endl);
}

void RenDevice::start3D()
{
    // A camera must have been specified.
    PRE(currentCamera());
    PRE(idleRendering());

    // Clear out any left-over unused material bodies, from eg a persistent load
    RenIMatManager::instance().clearUnusedMatBodyList();

    const double now = DEBUG_FRAME_TIME;
    RENDER_STREAM("RenDevice::start3D() at " << now << "(ms)\n");
    RENDER_INDENT(3);

    pImpl_->rendering3D_ = true;
    pImpl_->shadowMappingEnabled_ = false;

    if (pImpl_->stats_)
    {
        pImpl_->stats_->startFrame();
    }

    RenSurfaceManager::instance().startFrame();

    // Viewpoint and/or camera parameters may have changed.
    updateMatrices();

    POST(rendering3D());
}

void RenDevice::beginGeometryPass(bool clearBack)
{
    PRE(rendering3D());

    // The background colour needs to have the camera's filter applied to it.
    RenColour bgCol = pImpl_->background_;
    bgCol.r(bgCol.r() * pImpl_->currentCamera_->colourFilter().r());
    bgCol.g(bgCol.g() * pImpl_->currentCamera_->colourFilter().g());
    bgCol.b(bgCol.b() * pImpl_->currentCamera_->colourFilter().b());

    ASSERT(pImpl_->vpMapping_, "No viewport set; startFrame should set a default.");

    CB_RENDEVICE_DEPIMPL_GL();

    const bool postProcess = postProcessReady_ && Config::gfxToneMapping.get();
    if (postProcess)
        recordCommand(Ren::Command::beginRenderPass(geometryRenderPass_, bgCol, postProcessFBO_));
    else
        recordCommand(Ren::Command::beginRenderPass(geometryRenderPass_, bgCol));

    pImpl_->illuminator_->filter(pImpl_->currentCamera_->colourFilter());
    pImpl_->illuminator_->startFrame();
    RenMesh::startFrame();

    setFog(pImpl_->fogStart_, pImpl_->fogEnd_, pImpl_->fogDensity_, fogColour());

    if (pImpl_->caps_ && pImpl_->caps_->internal()->supportsZBias())
    {
        const auto* internalCaps = pImpl_->caps_->internal();
        recordCommand(Ren::Command::setPolygonOffsetFill(false));
        recordCommand(Ren::Command::setPolygonOffset(static_cast<float>(internalCaps->minZBias()), 1.0f));
    }

    recordCommand(Ren::Command::setDepthTest(true));
    recordCommand(Ren::Command::setDepthMaskWritable(true));
    recordCommand(Ren::Command::setCullFace(true));

    // All the alpha polygons are drawn as a post-pass in endFrame, so we can
    // turn blending off for the main pass.
    pImpl_->disableAlphaBlending();

    // Enable alpha sorting.
    pImpl_->alphaSorter_ = pImpl_->normalAlphaSorter_;
}

void RenDevice::startBackground(double yon)
{
    PRE(rendering());

    const double now = DEBUG_FRAME_TIME;
    RENDER_STREAM("RenDevice::startBackground() at " << now << "(ms)\n");

    // Disable alpha sorting.  Background alpha is drawn in order.
    pImpl_->alphaSorter_ = nullptr;
    pImpl_->doingBackground_ = true;
    overrideClipping(pImpl_->currentCamera_->hitherClipDistance(), yon);
    disableLighting();
    disableFog();
    CB_DEPIMPL_AUTO(backend_);

    recordCommand(Ren::Command::setDepthMaskWritable(false));
}

inline bool isWhiteChar(char c)
{
    return c == ' ' || c == '\n' || c == '\t';
}

static bool isWhiteString(const std::string& str)
{
    for (int i = 0; i != str.length(); ++i)
        if (!isWhiteChar(str[i]))
            return false;

    return true;
}

void RenDevice::flush3DAlpha()
{
    PRE(rendering());
    PRE(rendering3D());

    const double now = DEBUG_FRAME_TIME;
    RENDER_STREAM("RenDevice::flush3DAlpha() at " << now << "(ms)\n");

    CB_DEPIMPL_AUTO(backend_);

    // Make sure all the Direct3D parameters are set up correctly. This function
    // may be called after 2D update, therefore we can not rely on these parameters
    // being set correcly.
    recordCommand(Ren::Command::setAlphaTestEnabled(0.0f));
    recordCommand(Ren::Command::setDepthTest(true));
    recordCommand(Ren::Command::setDepthMaskWritable(true));
    recordCommand(Ren::Command::setCullFace(true));
    pImpl_->disableAlphaBlending();
    recordCommand(Ren::Command::setMultisample(true));
    recordCommand(Ren::Command::setDepthFunc(Ren::BackendDepthFunc::LessOrEqual));

    if (pImpl_->doingBackground_)
    {
        // The clipping will have been changed for background objects.  It
        // needs to be reset before the other post-sorters are invoked.
        const double hither = pImpl_->currentCamera_->hitherClipDistance();
        const double yon = pImpl_->currentCamera_->yonClipDistance();
        const double h = pImpl_->currentCamera_->projectionPlaneHalfHeight();
        updateProjMatrix(hither, yon, h);

        // Likewise for the lighting, fog and alpha on the coplanar polys.
        enableLighting();
    }

    // Inter-mesh coplanar polygons are drawn with normal settings.  They are
    // drawn here because they need to come at the very end, after anything
    // they might be on top of.  (But before things with alpha.)
    restoreFog();

    pImpl_->coplanarSorter_->render();

    // The coplanar sorter may leave the zBias in an arbitrary state.
    if (pImpl_->caps_ && pImpl_->caps_->internal()->supportsZBias())
    {
        const auto* internalCaps = pImpl_->caps_->internal();
        recordCommand(Ren::Command::setPolygonOffsetFill(false));
        recordCommand(Ren::Command::setPolygonOffset(internalCaps->minZBias(), 1.0f));
    }

    if (pImpl_->caps_->supportsFlatAlpha() || pImpl_->caps_->supportsTextureAlpha())
    {
        // The whole raison-d'etre of the alpha sorter is drawing the polygons
        // without the z-buffer.
        recordCommand(Ren::Command::setDepthMaskWritable(false));
        pImpl_->enableAlphaBlending();

        pImpl_->normalAlphaSorter_->render();
        pImpl_->disableAlphaBlending();
    }

    recordCommand(Ren::Command::setAlphaTestDisabled());
}

void RenDevice::end3D()
{
    PRE(rendering3D());

    const double now1 = DEBUG_FRAME_TIME;
    RENDER_STREAM("  RenDevice::end3D() starts at " << now1 << "(ms)\n");

    flush3DAlpha();

    const double now2 = DEBUG_FRAME_TIME;
    RENDER_STREAM("  RenDevice::end3D() adding 2D effects at " << now2 << "(ms)\n");

    if (pImpl_->interference_ > 0.001)
        addInterference();

    if (pImpl_->staticOn_)
        addStatic();

    // If we fail to end a scene then we should NOT call DirectX
    // BeginScene again until we successfully end a scene.

    RENDER_STREAM("pImpl_->shouldBeginScene_ == " << pImpl_->shouldBeginScene_ << std::endl);
    RENDER_INDENT(-3);

    const double now3 = DEBUG_FRAME_TIME;
    RENDER_STREAM("  RenDevice::end3D() ends at " << now3 << "(ms)\n");
    RENDER_STREAM('}' << std::endl);

    recordCommand(Ren::Command::endRenderPass());

    CB_RENDEVICE_DEPIMPL_GL();

    if (postProcessReady_ && Config::gfxToneMapping.get())
        blitPostProcess();

    pImpl_->rendering3D_ = false;

    POST(idleRendering());
}

void RenDevice::blitPostProcess()
{
    CB_RENDEVICE_DEPIMPL_GL();

    // Resolve the offscreen HDR buffer to the default framebuffer via tone mapping.
    recordCommand(Ren::Command::bindDefaultFramebuffer());
    recordCommand(Ren::Command::setViewport(0, 0, postProcessWidth_, postProcessHeight_));
    recordCommand(Ren::Command::setDepthTest(false));
    recordCommand(Ren::Command::setDepthMaskWritable(false));
    recordCommand(Ren::Command::setBlendStateDisabled());
    recordCommand(Ren::Command::setCullFace(false));
    recordCommand(Ren::Command::setAlphaTestDisabled());

    // Disable all vertex attribs left over from the 3D pipeline before
    // binding the fullscreen-quad VBO which has a different layout.
    recordCommand(Ren::Command::disableVertexAttribPointer(standard_.posAttr));
    recordCommand(Ren::Command::disableVertexAttribPointer(standard_.uvAttr));
    recordCommand(Ren::Command::disableVertexAttribPointer(standard_.colAttr));
    recordCommand(Ren::Command::disableVertexAttribPointer(standard_.normalAttr));
    recordCommand(Ren::Command::disableVertexAttribPointer(standard_.vtxDiffuseAttr));
    recordCommand(Ren::Command::disableVertexAttribPointer(standard_.vtxAmbientAttr));
    recordCommand(Ren::Command::disableVertexAttribPointer(standard_.vtxEmissiveAttr));

    recordCommand(Ren::Command::bindPipeline(postProcess_.id));
    {
        Ren::PostProcessUniforms ppu;
        ppu.sceneTextureSampler = 0;
        ppu.exposure = 1.0f;
        recordCommand(Ren::Command::setPostProcessUniforms(std::move(ppu)));
    }

    recordCommand(Ren::Command::bindTexture2D(
        postProcessColorTexture_, 0, Ren::TextureFilter::Linear, Ren::TextureFilter::Linear));

    recordCommand(Ren::Command::bindBuffer(Ren::BufferTarget::Array, postProcessQuadVBO_));
    recordCommand(Ren::Command::enableVertexAttribPointer(
        postProcess_.posAttr, 2, Ren::BackendVertexAttribType::Float, false,
        4 * sizeof(float), 0));
    recordCommand(Ren::Command::enableVertexAttribPointer(
        postProcess_.uvAttr, 2, Ren::BackendVertexAttribType::Float, false,
        4 * sizeof(float), 2 * sizeof(float)));

    recordCommand(Ren::Command::draw(Ren::PrimitiveTopology::Triangles, 0, 6));

    recordCommand(Ren::Command::disableVertexAttribPointer(postProcess_.posAttr));
    recordCommand(Ren::Command::disableVertexAttribPointer(postProcess_.uvAttr));
}

void RenDevice::commonEndFrame()
{
    const double now1 = DEBUG_FRAME_TIME;
    RENDER_STREAM("  RenDevice::endFrame() starts at " << now1 << "(ms)\n");

    if (pImpl_->stats_)
    {
        pImpl_->stats_->endFrame();
    }

    // Print the statistics (which may be empty) followed by the debug stream.
    // Use the top-most coord of the 3D viewport as the top-most coord for text.
    // (Other areas of the display is not the responsibility of this class.)
    const std::string empty;
    auto const& statsStr = (pImpl_->stats_) ? pImpl_->stats_->statsText() : empty;
    const std::string debugStr = pImpl_->extOut_.dataAsString();

    ASSERT(pImpl_->vpMapping_, "No viewport set; startFrame should set a default.");
    const std::string concat = statsStr + debugStr;

    if (concat.length() > 0 && ! isWhiteString(concat) && pImpl_->shouldBeginScene_)
    {
        RenSurface surf = backSurface();
        auto const& yellowColour { RenColour::yellow() };
        // >trusting incremental rebuilds
        // surf.drawText(pImpl_->debugX_, pImpl_->debugY_, concat, yellowColour, RenSurface::FontSizes::Statistics,
        // RenSurface::AvailableFonts::Terminus);
        const Ren::Font* font = Ren::Font::getFont(RenSurface::getDefaultFontSize());
        ASSERT(font, "Unable to get font");
        surf.filledRectangle(Ren::Rect(pImpl_->debugX_, pImpl_->debugY_, font->horizontalAdvance(concat) + 8, font->height() * 4 + 8), RenColour(0.1, 0.1, 0.1, 0.5));
        surf.drawText(pImpl_->debugX_ + 4, pImpl_->debugY_ + 4, concat, *font, yellowColour);
    }

    pImpl_->extOut_.clear();

    const double now2 = DEBUG_FRAME_TIME;
    RENDER_STREAM("  RenDevice::endFrame() text done at " << now2 << "(ms)\n");
}

void RenDevice::endFrame()
{
    PRE(rendering());

    commonEndFrame();
    RenSurface backBuf = backSurface();
    pImpl_->display_->displayImpl().drawCursor(backBuf);
    pImpl_->destroyFrameCommandBuffer();
    pImpl_->display_->flipBuffers();
    pImpl_->rendering_ = false;

    const double now = DEBUG_FRAME_TIME;
    RENDER_STREAM("  RenDevice::endFrame() buffers flipped at " << now << "(ms)\n");

    RENDER_INDENT(-3);
    RENDER_STREAM("}" << std::endl << std::endl);

    POST(!rendering());
    POST(!rendering3D());
}

///////////////////////// Transformation matrix support (MVP) /////////////////////////

void RenDevice::updateMatrices()
{
    // Recalculate the view matrix based on the camera's current position.
    // TBD: use transform IDs to avoid the calculation when the eyepoint
    // hasn't moved since the last frame.
    updateViewMatrix(view_);

    // If the camera parameters change from frame to frame, we need to update
    // the projection matrix.
    const double hither = pImpl_->currentCamera_->hitherClipDistance();
    const double yon = pImpl_->currentCamera_->yonClipDistance();
    const double h = pImpl_->currentCamera_->projectionPlaneHalfHeight();
    updateProjMatrix(hither, yon, h);

    // Cache the product of the above matrices.
    *pImpl_->projViewMatrix_ = projection_ * view_;

    standardUniformsDirty_ = true;
    billboardUniformsDirty_ = true;
}

// From the Direct3D tunnel sample:
// Creates a matrix which is equivalent to having the camera at a
// specified position. This matrix can be used to convert vertices to
// camera coordinates.

static void computeViewMatrix(
    glm::vec3& rP, // position of the camera
    glm::vec3& rD, // direction of view
    glm::vec3& rN, // up vector
    glm::mat4* lpM // result
)
{
    // Set the rotation part of the matrix and invert it. Vertices must be
    // inverse rotated to achieve the same result of a corresponding
    // camera rotation.
    glm::mat4 tmp;
    tmp[0][3] = tmp[1][3] = tmp[2][3] = tmp[3][0] = tmp[3][1] = tmp[3][2] = (float)0.0;
    tmp[3][3] = (float)1.0;

    GLMatSetRotation(&tmp, &rD, &rN);
    GLMatTranspose(lpM, &tmp);

    // Multiply the rotation matrix by a translation transform.  The
    // translation matrix must be applied first (left of rotation).
    (*lpM)[3][0] = -((*lpM)[0][0] * rP.x + (*lpM)[1][0] * rP.y + (*lpM)[2][0] * rP.z);
    (*lpM)[3][1] = -((*lpM)[0][1] * rP.x + (*lpM)[1][1] * rP.y + (*lpM)[2][1] * rP.z);
    (*lpM)[3][2] = -((*lpM)[0][2] * rP.x + (*lpM)[1][2] * rP.y + (*lpM)[2][2] * rP.z);

    // Negating these elements provides the change from DirectX's left-
    // handed co-ordinate system to a right-handed one.
    if (MexCoordSystem::instance() == MexCoordSystem::RIGHT_HANDED)
    {
        (*lpM)[0][2] = -(*lpM)[0][2];
        (*lpM)[1][2] = -(*lpM)[1][2];
        (*lpM)[2][2] = -(*lpM)[2][2];
        (*lpM)[3][2] = -(*lpM)[3][2];
    }
}

// Use the camera's current position to set the view matrix part.

void RenDevice::updateViewMatrix(glm::mat4& view)
{
    //    PRE(device_);
    PRE(pImpl_->currentCamera_);

    const MexTransform3d& xform = pImpl_->currentCamera_->transform();
    MexVec3 xBasis = xform.xBasis();
    MexVec3 zBasis = xform.zBasis();
    const MexVec3 pos = xform.position();

    xBasis.makeUnitVector();
    zBasis.makeUnitVector();

    // This is how a RenCamera is defined -- looking along the x-axis
    // with the z-axis as its up vector.
    glm::vec3 dir(xBasis.x(), xBasis.y(), xBasis.z());
    glm::vec3 up(zBasis.x(), zBasis.y(), zBasis.z());
    glm::vec3 p(pos.x(), pos.y(), pos.z());

    // The camera's direction needs to be negated for the change from an
    // LH to an RH co-ordinate system.
    if (MexCoordSystem::instance() == MexCoordSystem::RIGHT_HANDED)
    {
        dir.x = -dir.x;
        dir.y = -dir.y;
        dir.z = -dir.z;
    }

    computeViewMatrix(p, dir, up, &view);

    // glm::mat4 view = glm::lookAt(p, dir, up);
    // view = glm::lookAt(p, dir, up);
}

// Taken straight from the M$ samples.

static void
computePerspectiveProjection(glm::mat4* lpd3dMatrix, double dHalfHeight, double dFrontClipping, double dBackClipping)
{
    PRE(lpd3dMatrix);
    PRE(dHalfHeight > 0);
    PRE(dFrontClipping > 0);
    PRE(dFrontClipping < dBackClipping);

    double dTmp1;
    double dTmp2;

    dTmp1 = dHalfHeight / dFrontClipping;
    dTmp2 = dBackClipping / (dBackClipping - dFrontClipping);

    (*lpd3dMatrix)[0][0] = (0.8); //(1.0);
    (*lpd3dMatrix)[0][1] = (0.0);

    (*lpd3dMatrix)[0][2] = (0.0);
    (*lpd3dMatrix)[0][3] = (0.0);
    (*lpd3dMatrix)[1][0] = (0.0);
    (*lpd3dMatrix)[1][1] = (1.0);
    (*lpd3dMatrix)[1][2] = (0.0);
    (*lpd3dMatrix)[1][3] = (0.0);
    (*lpd3dMatrix)[2][0] = (0.0);
    (*lpd3dMatrix)[2][1] = (0.0);
    (*lpd3dMatrix)[2][2] = (dTmp1 * dTmp2);
    (*lpd3dMatrix)[2][3] = (dTmp1);
    (*lpd3dMatrix)[3][0] = (0.0);
    (*lpd3dMatrix)[3][1] = (0.0);

    (*lpd3dMatrix)[3][2] = (-dHalfHeight * dTmp2);
    (*lpd3dMatrix)[3][3] = (0.0);
}

void RenDevice::updateProjMatrix(double hither, double yon, double h)
{
    //    PRE(device_);
    PRE(pImpl_->currentCamera_);
    PRE(h > 0);
    PRE(hither < yon);

    const RenDisplay::Mode mode = pImpl_->display_->currentMode();

    projection_ = glm::perspectiveFovLH<float>(
        pImpl_->currentCamera_->verticalFOVAngle(),
        mode.width(),
        mode.height(),
        hither,
        yon);

    standardUniformsDirty_ = true;
    billboardUniformsDirty_ = true;
}

void RenDevice::overrideClipping(double hither, double yon)
{
    //    PRE(device_);
    PRE(pImpl_->currentCamera_);
    PRE(hither < yon);

    const double h = pImpl_->currentCamera_->projectionPlaneHalfHeight();
    updateProjMatrix(hither, yon, h); // This causes display problems with shadows and so one
}

///////////////////////////////////// Fog support /////////////////////////////
// There are two types of fog in Direct3D, each has a different set of states
// which must be enabled separately.  Table fog is per-pixel depth-based fog
// and is a function of the raster engine.  Alternatively, the lighting module
// can calculate fog on a per-vertex basis, alter the vertex colours and have
// them interpolated across the polygon.  Table is better.

void RenDevice::setFog(float start, float end, float dense, const RenColour& colour)
{
    ASSERT(implies(pImpl_->fogOn_, pImpl_->caps_->supportsFog()), logic_error("Unsupported fog has been switched on."));

    if (pImpl_->caps_->internal()->supportsTableFog())
    {
        if (pImpl_->fogOn_)
        {
            fogParams_ = glm::vec3(start, end, dense);
        }
        else
        {
            fogParams_ = glm::vec3(0, 0, 0);
        }
    }
    else if (pImpl_->caps_->internal()->supportsVertexFog())
    {
        if (pImpl_->fogOn_)
        {
            fogParams_ = glm::vec3(start, end, dense);
        }
        else
        {
            fogParams_ = glm::vec3(0, 0, 0);
        }
    }

    if (pImpl_->fogOn_ && pImpl_->caps_->supportsFog())
    {
        // The fog colour needs to have the camera's filter applied to it.
        RenColour filteredCol = colour;

        if (pImpl_->currentCamera_)
        {
            filteredCol.r(filteredCol.r() * pImpl_->currentCamera_->colourFilter().r());
            filteredCol.g(filteredCol.g() * pImpl_->currentCamera_->colourFilter().g());
            filteredCol.b(filteredCol.b() * pImpl_->currentCamera_->colourFilter().b());
        }

        // How do you set the colour for the light state??
        // It looks like you have to set these parts of the render state
        // for both vertex fog and table fog.

        fogColour_ = glm::vec3(filteredCol.r(), filteredCol.g(), filteredCol.b());
    }

    standardUniformsDirty_ = true;
}

void RenDevice::fogOn(float start, float end, float density)
{
    PRE(start > 0);
    PRE(start < end);
    PRE(density >= 0 && density <= 1);

    pImpl_->fogStart_ = start;
    pImpl_->fogEnd_ = end;
    pImpl_->fogDensity_ = density;

    if (pImpl_->caps_->supportsFog())
    {
        pImpl_->fogOn_ = true;
        setFog(pImpl_->fogStart_, pImpl_->fogEnd_, pImpl_->fogDensity_, fogColour());
    }
}

void RenDevice::fogOff()
{
    pImpl_->fogOn_ = false;
    fogParams_ = glm::vec3(0, 0, 0);
    standardUniformsDirty_ = true;
}

void RenDevice::fogOn()
{
    if (pImpl_->caps_->supportsFog())
    {
        pImpl_->fogOn_ = true;
        fogParams_ = glm::vec3(pImpl_->fogStart_, pImpl_->fogEnd_, pImpl_->fogDensity_);
        standardUniformsDirty_ = true;
    }
}

void RenDevice::disableFog()
{
    if (fogParams_[1] > 0)
    {
        pImpl_->fogEnd_ = fogParams_[1];
    }

    pImpl_->fogDensity_ = fogParams_[2];

    fogOff();
}

void RenDevice::overrideFog(float start, float end, float density)
{
    if (pImpl_->caps_->supportsFog())
        setFog(start, end, density, fogColour());
}

void RenDevice::overrideFog(float start, float end, float density, const RenColour& col)
{
    if (pImpl_->caps_->supportsFog())
        setFog(start, end, density, col);
}

void RenDevice::restoreFog()
{
    pImpl_->fogOn_ = true;

    if (pImpl_->caps_->supportsFog())
    {
        setFog(pImpl_->fogStart_, pImpl_->fogEnd_, pImpl_->fogDensity_, fogColour());
    }
}

bool RenDevice::isFogOn() const
{
    ASSERT(implies(pImpl_->fogOn_, pImpl_->caps_->supportsFog()), logic_error("Unsupported fog has been switched on."));
    return pImpl_->fogOn_;
}

// TBD: these don't account for the overrides.  They probably ought to return
// the values currently in use.  Rewrite?

float RenDevice::fogStart() const
{
    return pImpl_->fogStart_;
}

float RenDevice::fogEnd() const
{
    return pImpl_->fogEnd_;
}

float RenDevice::fogDensity() const
{
    return pImpl_->fogDensity_;
}

// fast modifier of the fog range

void RenDevice::fogEnd(float end)
{
    PRE(pImpl_->fogOn_);

    pImpl_->fogEnd_ = end;
}

////////////////////////////////// Lighting support ///////////////////////////
// Everything is delegated to RenIIlluminator.

const RenColour& RenDevice::ambient() const
{
    return pImpl_->illuminator_->ambient();
}

void RenDevice::ambient(const RenColour& a)
{
    pImpl_->illuminator_->ambient(a);
}

void RenDevice::disableLighting()
{
    pImpl_->illuminator_->disableLighting();
}

void RenDevice::enableLighting()
{
    pImpl_->illuminator_->enableLighting();
}

bool RenDevice::lightingEnabled() const
{
    return pImpl_->illuminator_->lightingEnabled();
}

void RenDevice::addLight(RenLight* l)
{
    pImpl_->illuminator_->addLight(l);
}

void RenDevice::removeLight(RenLight* l)
{
    pImpl_->illuminator_->removeLight(l);
}

void RenDevice::useCamera(RenCamera* cam)
{
    PRE(cam);
    PRE(!rendering3D());
    PRE(cam->hitherClipDistance() < cam->yonClipDistance());

    pImpl_->currentCamera_ = cam;
    pImpl_->illuminator_->filter(pImpl_->currentCamera_->colourFilter());
}

void RenDevice::interferenceOn(double n)
{
    PRE(n >= 0 && n <= 1);
    pImpl_->interference_ = n;
}

void RenDevice::interferenceOff()
{
    pImpl_->interference_ = 0;
}

double RenDevice::interferenceAmount() const
{
    return pImpl_->interference_;
}

bool RenDevice::isInterferenceOn() const
{
    return pImpl_->interference_ > 0.0001;
}

inline float random0to1000()
{
    static MexBasicRandom random = MexBasicRandom::constructSeededFromTime();

    return mexRandomScalar(&random, 0.0, 1000.0);
}

void RenDevice::graduatedNoisePolygon(const Ren::Rect& area, double minAlpha, double maxAlpha)
{
    static RenTexture noiseTex = RenTexManager::instance().createTexture("static.bmp");
    static RenMaterial noiseMat;
    static bool first = true;
    static RenIVertex pts[6];

    if (first)
    {
        first = false;

        noiseMat.texture(noiseTex);
        noiseMat.makeNonSharable();

        for (int i = 0; i != 6; ++i)
        {
            pts[i].z = 0.0;
            pts[i].w = 1.0;
            pts[i].specular = packColourUnChecked(0, 0, 0);
        }
    }

    pts[0].x = pts[2].x = pts[4].x = area.originX;
    pts[1].x = pts[3].x = pts[5].x = area.originX + area.width;

    pts[0].y = pts[1].y = area.originY;
    pts[2].y = pts[3].y = area.originY + area.height / 2;
    pts[4].y = pts[5].y = area.originY + area.height;

    for (int i = 0; i != 6; ++i)
    {
        pts[i].tu = float(random0to1000());
        pts[i].tv = float(random0to1000());
    }

    pts[0].color = pts[1].color = packColour(1, 1, 1, minAlpha);
    pts[2].color = pts[3].color = packColour(1, 1, 1, maxAlpha);
    pts[4].color = pts[5].color = packColour(1, 1, 1, minAlpha);
    impl().setMaterialHandles(noiseMat);

    recordCommand(Ren::Command::setCullFace(false));
    recordCommand(Ren::Command::setDepthTest(false));

    renderScreenspace(pts, 6, noiseMat, Ren::PrimitiveTopology::TriangleStrip, area.width, area.height);
}

void RenDevice::uniformNoisePolygon(const Ren::Rect& area, double maxAlpha)
{
    static RenTexture noiseTex = RenTexManager::instance().createTexture("static.bmp");
    static RenMaterial noiseMat;
    static bool first = true;
    static RenIVertex pts[4];

    if (first)
    {
        first = false;

        noiseMat.texture(noiseTex);
        noiseMat.makeNonSharable();

        for (int i = 0; i != 4; ++i)
        {
            pts[i].z = (0.0);
            pts[i].w = (1.0);
            pts[i].specular = packColourUnChecked(0, 0, 0);
        }
    }
    pts[0].x = pts[2].x = area.originX;
    pts[1].x = pts[3].x = area.originX + area.width;

    pts[0].y = pts[1].y = area.originY;
    pts[2].y = pts[3].y = area.originY + area.height;

    for (int i = 0; i != 4; ++i)
    {
        pts[0].color = pts[1].color = packColour(1, 1, 1, maxAlpha);
        pts[2].color = pts[3].color = packColour(1, 1, 1, maxAlpha);
        pts[i].tu = float(random0to1000());
        pts[i].tv = float(random0to1000());
    }

    impl().setMaterialHandles(noiseMat);

    recordCommand(Ren::Command::setCullFace(false));
    recordCommand(Ren::Command::setDepthTest(false));

    renderScreenspace(pts, 4, noiseMat, Ren::PrimitiveTopology::TriangleStrip, area.width, area.height);
}

void RenDevice::addInterference()
{
    static MexBasicRandom random = MexBasicRandom::constructSeededFromTime();

    // If the noise amount is small, don't do anything on one quarter of
    // occaisions (randomly).
    if (pImpl_->interference_ < 0.2 && mexRandomInt(&random, 0, 3) == 0)
        return;

    // For OpenGL rendering viewport is always starting from 0,0
    //    const int viewportXOffset = pImpl_->vpMapping_->screenLeft();
    //    const int viewportYOffset = pImpl_->vpMapping_->screenTop();
    const int viewportXOffset = 0;
    const int viewportYOffset = 0;

    const int viewportWidth = pImpl_->vpMapping_->screenWidth();
    const int viewportHeight = pImpl_->vpMapping_->screenHeight();

    const int viewportRight = viewportWidth + viewportXOffset;
    const int viewportBottom = viewportHeight + viewportYOffset;

    if (pImpl_->interference_ < 0.8)
    {
        // Use graduated noise.
        const double fractionToCover = std::min(1.0, pImpl_->interference_ + 0.4);
        const double fractionToClear = 1 - fractionToCover;
        const double minAlpha = std::max(0.0, pImpl_->interference_ - 0.6);

        // Draw noise with some 100% clear gaps.
        if (fractionToClear > 0.0)
        {
            double fractionCovered = 0, fractionCleared = 0;
            int lastHeight = viewportYOffset - 0.15 * viewportHeight;

            for (int i = 0; i != 3 && lastHeight < viewportBottom; ++i)
            {
                const double toCoverRemaining = std::max(0.0, fractionToCover - fractionCovered);
                const double toClearRemaining = std::max(0.0, fractionToClear - fractionCleared);
                const double coverFraction = mexRandomScalar(&random, 0.0, toCoverRemaining);
                const double clearFraction = mexRandomScalar(&random, 0.0, toClearRemaining);

                if (toCoverRemaining <= 0)
                    return;

                fractionCovered += coverFraction;
                fractionCleared += clearFraction;
                const int top = lastHeight + (int)(clearFraction * viewportHeight);
                const int bottom = std::min(viewportBottom, top + (int)(coverFraction * viewportHeight));
                lastHeight = bottom;

                Ren::Rect rect1(viewportXOffset, top, viewportWidth, bottom);
                graduatedNoisePolygon(rect1, minAlpha, pImpl_->interference_);
            }
        }
        else // Enirely cover the screen, but vary the alpha.
        {
            const int coveredMinHeight = (int)(viewportHeight * 0.2);
            const int coveredMaxHeight = (int)(viewportHeight * 0.4);

            // The lower alpha value should increase smoothly with noise until
            // at 0.8 it matches the upper value.  Hence we transition
            // seamlessly into the uniform case.
            const double alpha = mexInterpolate(pImpl_->interference_, 0.6, 0.8, minAlpha, pImpl_->interference_);

            int lastHeight = viewportYOffset;
            for (int i = 0; i != 2 && lastHeight < viewportBottom; ++i)
            {
                const int coveredHeight = mexRandomInt(&random, coveredMinHeight, coveredMaxHeight);
                const int top = lastHeight;
                const int bottom = std::min(viewportBottom, top + coveredHeight);
                lastHeight = bottom;

                Ren::Rect rect1(viewportXOffset, top, viewportWidth, bottom);
                graduatedNoisePolygon(rect1, alpha, pImpl_->interference_);
            }

            // Ensure that we go right to the bottom of the screen.
            if (lastHeight < viewportBottom)
            {
                Ren::Rect rect1(viewportXOffset, lastHeight, viewportWidth, viewportBottom);
                graduatedNoisePolygon(rect1, alpha, pImpl_->interference_);
            }
        }
    }
    else // Use uniform noise rather than graduated.
    {
        Ren::Rect rect1(viewportXOffset, viewportYOffset, viewportWidth, viewportHeight);
        uniformNoisePolygon(rect1, pImpl_->interference_);
    }
}

void RenDevice::staticOn()
{
    pImpl_->staticOn_ = true;
}

void RenDevice::staticOff()
{
    pImpl_->staticOn_ = false;
}

bool RenDevice::isStaticOn() const
{
    return pImpl_->staticOn_;
}

void RenDevice::addStatic()
{
    // These aren't static because the viewport can change size in-game.
    // Avoid a 1 pixel boundary at the edges of the viewport.
    const int viewportXOffset = pImpl_->vpMapping_->screenLeft() + 1;
    const int viewportYOffset = pImpl_->vpMapping_->screenTop() + 1;

    const int viewportWidth = pImpl_->vpMapping_->screenWidth() - 2;
    const int viewportHeight = pImpl_->vpMapping_->screenHeight() - 2;

    const int viewportRight = viewportWidth + viewportXOffset;
    const int viewportBottom = viewportHeight + viewportYOffset;

    static const uint32_t numVertices = 300;
    static RenIVertex pts[numVertices];
    static bool first = true;
    static MexBasicRandom random = MexBasicRandom::constructSeededFromTime();

    // The colours, UV coords, depth info and even the alpha values can
    // be set once.  Only the xy coords should vary randomly between frames.
    if (first)
    {
        first = false;

        for (int i = 0; i != numVertices; i += 2)
        {
            const int x1 = (viewportWidth * rand() / RAND_MAX) + viewportXOffset;
            pts[i].x = (x1);

            // get random number from 2 to 13 for line length
            const int lineLength = mexRandomInt(&random, 2, 9);

            const int x2 = x1 + lineLength;

            if (x2 >= viewportRight)
                pts[i + 1].x = viewportRight - 1;
            else
                pts[i + 1].x = x2;

            const double green = mexRandomScalar(&random, 0.6, 1.0);
            const double alpha = mexRandomScalar(&random, 0.3, 0.9);
            pts[i].z = pts[i + 1].z = (0);
            pts[i].w = pts[i + 1].w = (1);
            pts[i].color = pts[i + 1].color = packColour(0, green, 0, alpha);
            pts[i].specular = pts[i + 1].specular = packColourUnChecked(0, 0, 0);
            pts[i].tu = pts[i + 1].tu = (0);
            pts[i].tv = pts[i + 1].tv = (0);
        }
    }

    for (int i = 0; i != numVertices; i += 2)
    {
        // Randomize the y coordinates.
        pts[i].y = float((viewportHeight * rand() / RAND_MAX) + viewportYOffset);

        // All lines are horizontal.
        pts[i + 1].y = pts[i].y;
    }

    disableFog();

    renderScreenspace(pts, numVertices, Ren::PrimitiveTopology::Lines, viewportWidth, viewportHeight);
    restoreFog();
}

const RenColour& RenDevice::backgroundColour() const
{
    return pImpl_->background_;
}

void RenDevice::backgroundColour(const RenColour& b)
{
    pImpl_->background_ = b;
}

Ren::Size RenDevice::windowSize() const
{
    return display()->currentMode().size();
}

int RenDevice::windowWidth() const
{
    // return backSurface().width();
    return display()->currentMode().width();
}

int RenDevice::windowHeight() const
{
    // return backSurface().height();
    return display()->currentMode().height();
}

// static

void RenDevice::useDevice(RenDevice* d)
{
    PRE(d);
    RenIDeviceImpl::useDevice(d, RenI::LAZY_UPDATE);
}

void RenDevice::displayImage(const SysPathName& pathName)
{
    RenSurface backBuf = backSurface();
    const std::string imagePath = pathName.pathname();
    RenTexture texture = RenSurfaceManager::instance().createTexture(imagePath);

    if (texture.isNull())
    {
        spdlog::error("Failed to load display image from {}", imagePath);
        return;
    }

    backBuf.simpleBlit(texture);
    flushCommandBuffer();
    pImpl_->display_->flipBuffers();
}

bool RenDevice::canSee(const MexPoint3d& pt) const
{
    // Originally there was a DX call like viewport_->TransformVertices(1, &txData, D3DTRANSFORM_CLIPPED, &offScreen);
    return true;
}

bool RenDevice::canSee(const MexQuad3d& quad) const
{
    PRE(currentCamera());

    // TODO: same value all the time may cause some problems?
    bool retval = true;

    return retval;
}

std::ostream& operator<<(std::ostream& o, const RenDevice& t)
{

    o << "RenDevice " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "RenDevice " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

bool RenDevice::activate()
{
    // Setting this flag makes startFrame try to restore the surfaces.
    pImpl_->surfacesMayBeLost_ = 10;
    return true;
}

bool RenDevice::setHighestAllowedDisplayMode()
{
    PRE(pImpl_->display_);
    PRE(pImpl_->caps_);

    return pImpl_->display_->setHighestAllowedMode(pImpl_->caps_->maxAvailableDisplayMemoryAfterTextures());
}

MexPoint3d RenDevice::screenToCamera(const MexPoint2d& screenPosition) const
{
    PRE(pImpl_->currentCamera_ != nullptr);

    // Get needed data from the camera
    double projectionDistance = pImpl_->currentCamera_->hitherClipDistance();
    double tanHalfVerticalFOVAngle = pImpl_->currentCamera_->tanHalfVerticalFOVAngle();

    // Get the viewport dimensions
    const Ren::Viewport viewPort = pImpl_->backend_->getViewport();
    double viewportPixelLeft(viewPort.x);
    double viewportPixelTop(viewPort.y);
    double viewportPixelWidth(viewPort.width);
    double viewportPixelHeight(viewPort.height);

    // Compute the viewport dimensions in real world coordinates
    double realViewportHeight = 2.0 * projectionDistance * tanHalfVerticalFOVAngle;
    double realViewportWidth = realViewportHeight * (viewportPixelWidth / viewportPixelHeight);

    // Compute the coordinates in world space in the projection plane of the screen position
    double y = realViewportWidth * ((screenPosition.x() - viewportPixelLeft) / viewportPixelWidth - 0.5);
    double z = realViewportHeight * (0.5 - (screenPosition.y() - viewportPixelTop) / viewportPixelHeight);

    return MexPoint3d(projectionDistance, y, z);
}

RenSurface RenDevice::backSurface()
{
    // Only get the surface once from the manager.  (This method is likely to
    // be called many times.)
    if (!pImpl_->surfBackBuf_)
    {
        pImpl_->surfBackBuf_ = new RenSurface;
        RenISurfaceManagerImpl& surfMan = RenSurfaceManager::instance().impl();
        *pImpl_->surfBackBuf_ = surfMan.createDisplaySurface(this, RenI::BACK);
    }

    return *pImpl_->surfBackBuf_;
}

RenSurface RenDevice::frontSurface()
{
    // Only get the surface once from the manager.  (This method is likely to
    // be called many times.)
    if (!pImpl_->surfFrontBuf_)
    {
        pImpl_->surfFrontBuf_ = new RenSurface;
        RenISurfaceManagerImpl& surfMan = RenSurfaceManager::instance().impl();
        *pImpl_->surfFrontBuf_ = surfMan.createDisplaySurface(this, RenI::FRONT);
    }

    return *pImpl_->surfFrontBuf_;
}

const RenSurface RenDevice::backSurface() const
{
    RenDevice* ncThis = const_cast<RenDevice*>(this);
    return ncThis->backSurface();
}

const RenSurface RenDevice::frontSurface() const
{
    RenDevice* ncThis = const_cast<RenDevice*>(this);
    return ncThis->frontSurface();
}

MexPoint2d RenDevice::cameraToScreen(const MexPoint3d& worldPosition) const
{
    PRE(pImpl_->currentCamera_ != nullptr);

    // Get needed data from the camera
    double projectionDistance = pImpl_->currentCamera_->hitherClipDistance();
    double tanHalfVerticalFOVAngle = pImpl_->currentCamera_->tanHalfVerticalFOVAngle();

    // Get the viewport dimensions
    const Ren::Viewport viewPort = pImpl_->backend_->getViewport();
    double viewportPixelLeft(viewPort.x);
    double viewportPixelTop(viewPort.y);
    double viewportPixelWidth(viewPort.width);
    double viewportPixelHeight(viewPort.height);

    // Compute the viewport dimensions in real world coordinates
    double realViewportHeight = 2.0 * projectionDistance * tanHalfVerticalFOVAngle;
    double realViewportWidth = realViewportHeight * (viewportPixelWidth / viewportPixelHeight);

    // Do the perspective projection of the point into the projection plane.
    // If the point lies in the camera's x=0 plane, frig it a bit.
    double x = worldPosition.x();
    if (fabs(x) < 0.001)
        x = (x >= 0.0 ? 0.001 : -0.001);
    double f = projectionDistance / x;
    double y = worldPosition.y() * f;
    double z = worldPosition.z() * f;

    // Now convert to screen pixel coordinates
    y = viewportPixelWidth * (y / realViewportWidth + 0.5) + viewportPixelLeft;
    z = viewportPixelHeight * (0.5 - z / realViewportHeight) + viewportPixelTop;

    return MexPoint2d(y, z);
}

RenDevice::Filter::Filter(RenDevice* pDevice, const RenColour& col)
    : pDevice_(pDevice)
{
    pDevice_->setFilter(col);
}

RenDevice::Filter::~Filter()
{
    pDevice_->resetFilter();
}

void RenDevice::setFilter(const RenColour& col)
{
    const RenColour& camFilter = pImpl_->currentCamera_->colourFilter();

    RenColour newColour(camFilter.r() * col.r(), camFilter.g() * col.g(), camFilter.b() * col.b());
    pImpl_->illuminator_->filter(newColour);
}

void RenDevice::resetFilter()
{
    pImpl_->illuminator_->filter(pImpl_->currentCamera_->colourFilter());
}

RenCamera* RenDevice::currentCamera() const
{
    return pImpl_->currentCamera_;
}

// static

RenDevice* RenDevice::current()
{
    return RenIDeviceImpl::current();
}

void RenDevice::recordCommand(Ren::BackendCommand command)
{
    PRE(pImpl_);
    PRE(pImpl_->backend_);
    const auto handle = pImpl_->currentCommandBufferHandle();
    PRE(handle.isValid());

    pImpl_->backend_->recordCommand(handle, std::move(command));
}

void RenDevice::recordSetUniform1i(Ren::UniformLocationId location, int value)
{
    if (!location.isValid())
        return;

    recordCommand(Ren::Command::setUniform1i(location, value));
}

void RenDevice::recordSetUniform2f(Ren::UniformLocationId location, float x, float y)
{
    if (!location.isValid())
        return;

    recordCommand(Ren::Command::setUniform2f(location, x, y));
}

void RenDevice::recordSetUniform1fv(Ren::UniformLocationId location, const float* data, int count)
{
    if (!location.isValid() || count <= 0)
        return;

    recordCommand(Ren::Command::setUniform1fv(location, std::vector<float>(data, data + count)));
}

void RenDevice::recordSetUniform3f(Ren::UniformLocationId location, float x, float y, float z)
{
    if (!location.isValid())
        return;

    recordCommand(Ren::Command::setUniform3f(location, x, y, z));
}

void RenDevice::recordSetUniform3fv(Ren::UniformLocationId location, const float* data, int count)
{
    if (!location.isValid() || count <= 0)
        return;

    recordCommand(Ren::Command::setUniform3fv(location, std::vector<float>(data, data + count * 3)));
}

void RenDevice::recordSetUniformMatrix4fv(Ren::UniformLocationId location, const glm::mat4& matrix)
{
    if (!location.isValid())
        return;

    std::array<float, 16> values{};
    std::memcpy(values.data(), glm::value_ptr(matrix), sizeof(float) * values.size());
    recordCommand(Ren::Command::setUniformMatrix4fv(location, values, false));
}

void RenDevice::recordEnableVertexAttribPointer(
    Ren::AttributeLocationId index,
    int size,
    Ren::BackendVertexAttribType type,
    bool normalized,
    std::size_t stride,
    std::size_t offset)
{
    if (!index.isValid())
        return;

    recordCommand(Ren::Command::enableVertexAttribPointer(index, size, type, normalized, stride, offset));
}

void RenDevice::recordDisableVertexAttribPointer(Ren::AttributeLocationId index)
{
    if (!index.isValid())
        return;

    recordCommand(Ren::Command::disableVertexAttribPointer(index));
}

Ren::FrameState RenDevice::buildFrameState() const
{
    Ren::FrameState fs;
    fs.view = toFloatArray(view_);
    fs.proj = toFloatArray(projection_);
    fs.fogColourR = fogColour_.x;
    fs.fogColourG = fogColour_.y;
    fs.fogColourB = fogColour_.z;
    fs.fogStartOrX = fogParams_.x;
    fs.fogEndOrY = fogParams_.y;
    fs.fogDensityOrZ = fogParams_.z;
    return fs;
}

Ren::GpuLightingState RenDevice::buildGpuLightingState(bool gpuLighting) const
{
    Ren::GpuLightingState ls;
    ls.enabled = gpuLighting;
    if (!gpuLighting)
        return ls;

    ls.lightDir = pImpl_->gpuLightDir_;
    ls.lightColor = pImpl_->gpuLightColor_;
    ls.ambientColor = pImpl_->gpuAmbientColor_;
    ls.filter = glm::vec3(
        pImpl_->illuminator()->filter().r(),
        pImpl_->illuminator()->filter().g(),
        pImpl_->illuminator()->filter().b());
    ls.hasPerVertexMaterials = pImpl_->hasPerVertexMaterials_;
    ls.numPointLights = pImpl_->gpuNumPointLights_;
    ls.pointLightPos = pImpl_->gpuPointLightPos_;
    ls.pointLightColor = pImpl_->gpuPointLightColor_;
    ls.pointLightRange = pImpl_->gpuPointLightRange_;
    ls.pointLightAtten = pImpl_->gpuPointLightAtten_;
    ls.pointLightOmni = pImpl_->gpuPointLightOmni_;
    ls.shadowEnabled = pImpl_->shadowMappingEnabled_;
    if (ls.shadowEnabled)
    {
        ls.shadowStrength = pImpl_->shadowStrength_;
        ls.shadowSplitDistance = pImpl_->shadowSplitDistance_;
        ls.lightSpaceMatrix = toFloatArray(pImpl_->lightSpaceMatrix_);
        ls.lightSpaceMatrixNear = toFloatArray(pImpl_->lightSpaceMatrixNear_);
        ls.shadowDepthTexture = pImpl_->shadowDepthTexture_;
        ls.shadowNearDepthTexture = pImpl_->shadowNearDepthTexture_;
    }
    return ls;
}

Ren::StandardPipelineHandles RenDevice::buildStandardHandles() const
{
    CB_RENDEVICE_DEPIMPL_GL();
    Ren::StandardPipelineHandles h;
    h.pipelineId = standard_.id;
    h.posAttr = standard_.posAttr;
    h.uvAttr = standard_.uvAttr;
    h.colAttr = standard_.colAttr;
    h.normalAttr = standard_.normalAttr;
    h.vtxDiffuseAttr = standard_.vtxDiffuseAttr;
    h.vtxAmbientAttr = standard_.vtxAmbientAttr;
    h.vtxEmissiveAttr = standard_.vtxEmissiveAttr;
    h.vertexBuffer = vertexDataBuffer_;
    h.normalBuffer = normalBuffer_;
    h.vtxDiffuseBuffer = vtxDiffuseBuffer_;
    h.vtxAmbientBuffer = vtxAmbientBuffer_;
    h.vtxEmissiveBuffer = vtxEmissiveBuffer_;
    h.elementBuffer = elementBuffer_;
    return h;
}

void RenDevice::beginImmediateCommands()
{
    PRE(pImpl_);
    pImpl_->beginImmediateCommandBuffer();
}

void RenDevice::endImmediateCommands()
{
    PRE(pImpl_);
    PRE(pImpl_->immediateCommandBufferActive());
    pImpl_->endImmediateCommandBuffer();
}

bool RenDevice::immediateCommandsActive() const
{
    PRE(pImpl_);
    return pImpl_->immediateCommandBufferActive();
}

void RenDevice::flushCommandBuffer()
{
    PRE(pImpl_);
    if (pImpl_->frameCommandBufferRecording_)
        pImpl_->flushFrameCommandBuffer();
}

bool RenDevice::rendering() const
{
    return pImpl_->rendering_;
}

bool RenDevice::rendering2D() const
{
    return pImpl_->rendering2D_;
}

bool RenDevice::rendering3D() const
{
    return pImpl_->rendering3D_;
}

bool RenDevice::idleRendering() const
{
    return true; // rendering() and not rendering3D() and not rendering2D();
}

const RenCapabilities& RenDevice::capabilities() const
{
    PRE(pImpl_->caps_); // This is checked by the class invariant.
    return *pImpl_->caps_;
}

RenStats* RenDevice::statistics()
{
    return pImpl_->stats_;
}

const RenStats* RenDevice::statistics() const
{
    return pImpl_->stats_;
}

RenDisplay* RenDevice::display()
{
    return pImpl_->display_;
}

const RenDisplay* RenDevice::display() const
{
    return pImpl_->display_;
}

const RenColour& RenDevice::fogColour() const
{
    return pImpl_->fogColour_;
}

void RenDevice::fogColour(const RenColour& newFogColour)
{
    pImpl_->fogColour_ = newFogColour;
}

std::ostream& RenDevice::out()
{
    PRE(pImpl_);
    return pImpl_->extOut_;
}

RenIDeviceImpl& RenDevice::impl()
{
    PRE(pImpl_);
    return *pImpl_;
}

const RenIDeviceImpl& RenDevice::impl() const
{
    PRE(pImpl_);
    return *pImpl_;
}

Ren::IRenderBackend& RenDevice::backend()
{
    return *impl().backend_;
}

const Ren::IRenderBackend& RenDevice::backend() const
{
    return *impl().backend_;
}

void RenDevice::debugTextCoords(int x, int y)
{
    pImpl_->debugX_ = x;
    pImpl_->debugY_ = y;
}

void RenDevice::debugTextCoords(int* pX, int* pY) const
{
    PRE(pX && pY);
    *pX = pImpl_->debugX_;
    *pY = pImpl_->debugY_;
}

void RenDevice::setVSyncPreference(bool enabled)
{
    CB_RENDEVICE_DEPIMPL_GL();
    CB_DEPIMPL_AUTO(backend_);

    vsyncEnabled_ = enabled;

    if (backend_->isInitialized())
    {
        if (!setVSync(enabled))
        {
            spdlog::warn("Failed to switch VSync to {} after context creation", enabled);
        }
    }
}

bool RenDevice::setVSync(bool enabled)
{
    CB_RENDEVICE_DEPIMPL_GL();
    CB_DEPIMPL_AUTO(backend_);
    PRE(backend_->isInitialized());

    const bool success = backend_->setVSync(enabled);
    if (success)
    {
        vsyncEnabled_ = enabled;
    }
    return success;
}

void RenDevice::antiAliasingOn(bool o)
{
    pImpl_->antiAliasingOn_ = o;
}

bool RenDevice::antiAliasingOn() const
{
    return pImpl_->antiAliasingOn_;
}

void RenDevice::setSmoothScaleEnabled(bool enabled)
{
    pImpl_->smoothScaleEnabled_ = enabled;
}

void RenDevice::setMaterialHandles(const RenMaterial& mat)
{
    pImpl_->setMaterialHandles(mat);
}

void RenDevice::renderToTextureMode(Ren::TexId targetTexture, uint32_t viewPortW, uint32_t viewPortH)
{
    CB_RENDEVICE_DEPIMPL_GL();

    // Bind FBO to texture
    if (targetTexture != Ren::NullTexId)
    {
        recordCommand(Ren::Command::beginRenderToTexture(offscreenFramebuffer_, resolveTextureHandle(targetTexture)));
        Ren::Size viewportSize(viewPortW, viewPortH);
        recordCommand(Ren::Command::setViewport(viewportSize));
    }
    // Bind FBO to screen
    else
    {
        recordCommand(Ren::Command::endRenderToTexture());
        const RenDisplay::Mode& mode = pImpl_->display()->currentMode();
        recordCommand(Ren::Command::setViewport(mode.size()));
    }
}

void RenDevice::enableVertexLayout(
    Ren::AttributeLocationId posAttr,
    int posComponents,
    Ren::AttributeLocationId uvAttr,
    Ren::AttributeLocationId colAttr)
{
    recordEnableVertexAttribPointer(
        posAttr, posComponents, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex), 0);
    recordEnableVertexAttribPointer(
        uvAttr, 2, Ren::BackendVertexAttribType::Float, false, sizeof(RenIVertex),
        sizeof(RenIVertex) - 2 * sizeof(float));
    recordEnableVertexAttribPointer(
        colAttr, 4, Ren::BackendVertexAttribType::UnsignedByte, true, sizeof(RenIVertex),
        3 * sizeof(float) + sizeof(uint32_t));
}

void RenDevice::disableVertexLayout(
    Ren::AttributeLocationId posAttr,
    Ren::AttributeLocationId uvAttr,
    Ren::AttributeLocationId colAttr)
{
    recordDisableVertexAttribPointer(posAttr);
    recordDisableVertexAttribPointer(uvAttr);
    recordDisableVertexAttribPointer(colAttr);
}

void RenDevice::renderScreenspace(
    const RenIVertex* vertices,
    const size_t nVertices,
    Ren::PrimitiveTopology topology,
    const int targetW,
    const int targetH,
    Ren::TexId texture)
{
    CB_RENDEVICE_DEPIMPL_GL();
    CB_DEPIMPL_AUTO(backend_);

    recordCommand(Ren::Command::bindPipeline(gui2D_.id));

    static const int TextureUnit = 0;
    recordCommand(Ren::Command::bindTexture2D(resolveTextureHandle(texture), TextureUnit));

    {
        Ren::Gui2DUniforms u;
        u.screenspaceX = static_cast<float>(targetW);
        u.screenspaceY = static_cast<float>(targetH);
        u.textureSampler = TextureUnit;
        recordCommand(Ren::Command::setGui2DUniforms(std::move(u)));
    }

    recordCommand(Ren::Command::bufferData(
        Ren::BufferTarget::Array,
        vertexBuffer2D_,
        &vertices[0],
        nVertices * sizeof(RenIVertex),
        Ren::BufferUsage::StreamDraw));

    recordCommand(Ren::Command::bindBuffer(Ren::BufferTarget::Array, vertexBuffer2D_));
    enableVertexLayout(gui2D_.posAttr, 2, gui2D_.uvAttr, gui2D_.colAttr);

    recordCommand(Ren::Command::setDepthTest(false));

    using BlendFactor = Ren::BackendBlendFactor;
    recordCommand(Ren::Command::setBlendStateEnabled(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha));
    recordCommand(Ren::Command::draw(topology, 0, nVertices));

    disableVertexLayout(gui2D_.posAttr, gui2D_.uvAttr, gui2D_.colAttr);
}

void RenDevice::renderSurface(
    const RenISurfBody* surf,
    const Ren::Rect& srcArea,
    const Ren::Rect& dstArea,
    const uint32_t targetW,
    const uint32_t targetH,
    const uint32_t colour,
    Ren::BlitMode mode)
{
    CB_RENDEVICE_DEPIMPL_GL();
    CB_DEPIMPL_AUTO(backend_);

    RenIVertex vertices[6];

    glm::vec2 vertex_up_left
        = glm::vec2(dstArea.originX, dstArea.originY + dstArea.height); //( x     , y+srcArea.height );
    glm::vec2 vertex_up_right = glm::vec2(
        dstArea.originX + dstArea.width,
        dstArea.originY + dstArea.height); //( x+srcArea.width, y+srcArea.height );
    glm::vec2 vertex_down_right = glm::vec2(dstArea.originX + dstArea.width, dstArea.originY); //( x+srcArea.width, y );
    glm::vec2 vertex_down_left = glm::vec2(dstArea.originX, dstArea.originY); //( x, y      );

    // TODO handle empty surf
    float uvX = srcArea.originX / (float)surf->width();
    float uvY = srcArea.originY / (float)surf->height();
    float uvW = srcArea.width / (float)surf->width();
    float uvH = srcArea.height / (float)surf->height();
    glm::vec2 uv_up_left = glm::vec2(uvX, uvY + uvH); //( 0.0f, 1.0f );
    glm::vec2 uv_up_right = glm::vec2(uvX + uvW, uvY + uvH); //( 1.0f, 1.0f );
    glm::vec2 uv_down_right = glm::vec2(uvX + uvW, uvY); //( 1.0f, 0.0f );
    glm::vec2 uv_down_left = glm::vec2(uvX, uvY); //( 0.0f, 0.0f );

    // Bind shader
    recordCommand(Ren::Command::bindPipeline(gui2D_.id));

    vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color
        = vertices[5].color = colour;
    if (targetW)
    {
        Ren::Gui2DUniforms guiU;
        guiU.screenspaceX = static_cast<float>(targetW);
        guiU.screenspaceY = -static_cast<float>(targetH);

        vertices[2].x = vertex_up_left[0];
        vertices[2].y = vertex_up_left[1];
        vertices[1].x = vertex_down_left[0];
        vertices[1].y = vertex_down_left[1];
        vertices[0].x = vertex_up_right[0];
        vertices[0].y = vertex_up_right[1];

        vertices[5].x = vertex_down_right[0];
        vertices[5].y = vertex_down_right[1];
        vertices[4].x = vertex_up_right[0];
        vertices[4].y = vertex_up_right[1];
        vertices[3].x = vertex_down_left[0];
        vertices[3].y = vertex_down_left[1];

        vertices[2].tu = uv_up_left[0];
        vertices[2].tv = uv_up_left[1];
        vertices[1].tu = uv_down_left[0];
        vertices[1].tv = uv_down_left[1];
        vertices[0].tu = uv_up_right[0];
        vertices[0].tv = uv_up_right[1];

        vertices[5].tu = uv_down_right[0];
        vertices[5].tv = uv_down_right[1];
        vertices[4].tu = uv_up_right[0];
        vertices[4].tv = uv_up_right[1];
        vertices[3].tu = uv_down_left[0];
        vertices[3].tv = uv_down_left[1];

        guiU.textureSampler = 0;
        recordCommand(Ren::Command::setGui2DUniforms(std::move(guiU)));
    }
    else
    {
        const RenDisplay::Mode& mode = pImpl_->display()->currentMode();
        Ren::Gui2DUniforms guiU;
        guiU.screenspaceX = static_cast<float>(mode.width());
        guiU.screenspaceY = static_cast<float>(mode.height());

        vertices[0].x = vertex_up_left[0];
        vertices[0].y = vertex_up_left[1];
        vertices[1].x = vertex_down_left[0];
        vertices[1].y = vertex_down_left[1];
        vertices[2].x = vertex_up_right[0];
        vertices[2].y = vertex_up_right[1];

        vertices[3].x = vertex_down_right[0];
        vertices[3].y = vertex_down_right[1];
        vertices[4].x = vertex_up_right[0];
        vertices[4].y = vertex_up_right[1];
        vertices[5].x = vertex_down_left[0];
        vertices[5].y = vertex_down_left[1];

        vertices[0].tu = uv_up_left[0];
        vertices[0].tv = uv_up_left[1];
        vertices[1].tu = uv_down_left[0];
        vertices[1].tv = uv_down_left[1];
        vertices[2].tu = uv_up_right[0];
        vertices[2].tv = uv_up_right[1];

        vertices[3].tu = uv_down_right[0];
        vertices[3].tv = uv_down_right[1];
        vertices[4].tu = uv_up_right[0];
        vertices[4].tv = uv_up_right[1];
        vertices[5].tu = uv_down_left[0];
        vertices[5].tv = uv_down_left[1];

        guiU.textureSampler = 0;
        recordCommand(Ren::Command::setGui2DUniforms(std::move(guiU)));
    }

    recordCommand(Ren::Command::bufferData(
        Ren::BufferTarget::Array,
        vertexBuffer2D_,
        &vertices[0],
        6 * sizeof(RenIVertex),
        Ren::BufferUsage::StreamDraw));

    static const int TextureUnit = 0;
    const auto texFilter = pImpl_->smoothScaleEnabled_ ? Ren::TextureFilter::Linear : Ren::TextureFilter::Nearest;
    recordCommand(Ren::Command::bindTexture2D(surf->nativeTextureHandle(), TextureUnit, texFilter, texFilter));

    recordCommand(Ren::Command::bindBuffer(Ren::BufferTarget::Array, vertexBuffer2D_));
    enableVertexLayout(gui2D_.posAttr, 2, gui2D_.uvAttr, gui2D_.colAttr);

    recordCommand(Ren::Command::setDepthTest(false));

    const auto [srcFactor, dstFactor] = blendFactorsForBlitMode(mode);
    recordCommand(Ren::Command::setBlendStateEnabled(srcFactor, dstFactor));
    recordCommand(Ren::Command::draw(Ren::PrimitiveTopology::Triangles, 0, 6));

    disableVertexLayout(gui2D_.posAttr, gui2D_.uvAttr, gui2D_.colAttr);
}

void RenDevice::renderPrimitive(
    const RenIVertex* vertices,
    const size_t nVertices,
    const RenMaterial& mat,
    Ren::PrimitiveTopology topology)
{
    PRE(vertices);
    PRE(nVertices < 5000);

    CB_RENDEVICE_DEPIMPL_GL();

    const bool gpuLighting = pImpl_->expandedNormalsCount_ > 0 && nVertices <= pImpl_->expandedNormalsCount_;

    Ren::DrawCallFactory::Commands cmds;
    Ren::DrawCallFactory::emitStandard3DDraw(
        buildStandardHandles(), buildFrameState(), standardUniformsDirty_,
        toFloatArray(model_), mat, buildGpuLightingState(gpuLighting),
        resolveTextureHandle(mat.texture().handle()),
        vertices, nVertices,
        gpuLighting ? pImpl_->expandedNormals_.data() : nullptr,
        nullptr, nullptr, nullptr,
        topology, &cmds);

    if (standardUniformsDirty_)
        standardUniformsDirty_ = false;

    for (auto& cmd : cmds)
        recordCommand(std::move(cmd));

    if (gpuLighting)
        recordDisableVertexAttribPointer(standard_.normalAttr);

    disableVertexLayout(standard_.posAttr, standard_.uvAttr, standard_.colAttr);
}

void RenDevice::renderIndexed(
    const RenIVertex* vertices,
    const size_t nVertices,
    const Ren::VertexIdx* indices,
    const size_t nIndices,
    const RenMaterial& mat,
    Ren::PrimitiveTopology topology)
{
    PRE(vertices);
    PRE(indices);
    PRE(nVertices < 5000);
    PRE(nIndices < 5000);

    CB_RENDEVICE_DEPIMPL_GL();

    const bool gpuLighting = pImpl_->expandedNormalsCount_ > 0 && nVertices <= pImpl_->expandedNormalsCount_;

    Ren::DrawCallFactory::Commands cmds;
    Ren::DrawCallFactory::emitStandard3DDrawIndexed(
        buildStandardHandles(), buildFrameState(), standardUniformsDirty_,
        toFloatArray(model_), mat, buildGpuLightingState(gpuLighting),
        resolveTextureHandle(mat.texture().handle()),
        vertices, nVertices, indices, nIndices,
        gpuLighting ? pImpl_->expandedNormals_.data() : nullptr,
        gpuLighting && pImpl_->hasPerVertexMaterials_ ? pImpl_->expandedVtxDiffuse_.data() : nullptr,
        gpuLighting && pImpl_->hasPerVertexMaterials_ ? pImpl_->expandedVtxAmbient_.data() : nullptr,
        gpuLighting && pImpl_->hasPerVertexMaterials_ ? pImpl_->expandedVtxEmissive_.data() : nullptr,
        topology, &cmds);

    if (standardUniformsDirty_)
        standardUniformsDirty_ = false;

    for (auto& cmd : cmds)
        recordCommand(std::move(cmd));

    if (gpuLighting)
    {
        recordDisableVertexAttribPointer(standard_.normalAttr);
        if (pImpl_->hasPerVertexMaterials_)
        {
            recordDisableVertexAttribPointer(standard_.vtxDiffuseAttr);
            recordDisableVertexAttribPointer(standard_.vtxAmbientAttr);
            recordDisableVertexAttribPointer(standard_.vtxEmissiveAttr);
        }
    }

    disableVertexLayout(standard_.posAttr, standard_.uvAttr, standard_.colAttr);
}

void RenDevice::renderIndexedScreenspace(
    const RenIVertex* vertices,
    const size_t nVertices,
    const Ren::VertexIdx* indices,
    const size_t nIndices,
    const RenMaterial& mat,
    Ren::PrimitiveTopology topology)
{
    PRE(vertices);
    PRE(indices);
    PRE(nVertices < 5000);
    PRE(nIndices < 5000);

    CB_RENDEVICE_DEPIMPL_GL();

    Ren::BillboardPipelineHandles bh;
    bh.pipelineId = billboard_.id;
    bh.posAttr = billboard_.posAttr;
    bh.uvAttr = billboard_.uvAttr;
    bh.colAttr = billboard_.colAttr;
    bh.vertexBuffer = vertexDataBufferBillboard_;
    bh.elementBuffer = elementBufferBillboard_;

    Ren::BillboardUniforms bu;
    bu.viewProj = toFloatArray(*pImpl_->projViewMatrix_);
    bu.textureSampler = 0;

    Ren::DrawCallFactory::Commands cmds;
    Ren::DrawCallFactory::emitBillboardDrawIndexed(
        bh, bu, billboardUniformsDirty_,
        resolveTextureHandle(mat.texture().handle()),
        vertices, nVertices, indices, nIndices,
        topology, &cmds);

    if (billboardUniformsDirty_)
        billboardUniformsDirty_ = false;

    for (auto& cmd : cmds)
        recordCommand(std::move(cmd));

    disableVertexLayout(billboard_.posAttr, billboard_.uvAttr, billboard_.colAttr);
}

void RenDevice::beginShadowPass(ShadowCascade cascade, const glm::mat4& lightSpaceMatrix)
{
    CB_RENDEVICE_DEPIMPL_GL();
    CB_DEPIMPL_AUTO(backend_);

    if (cascade == ShadowCascade::Near)
        pImpl_->lightSpaceMatrixNear_ = lightSpaceMatrix;
    else
        pImpl_->lightSpaceMatrix_ = lightSpaceMatrix;

    pImpl_->activeShadowLightSpaceMatrix_ = lightSpaceMatrix;
    pImpl_->shadowPassActive_ = true;
    pImpl_->shadowMappingEnabled_ = true;

    const bool isNear = (cascade == ShadowCascade::Near);
    const int sz = isNear ? RenIDeviceImpl::ShadowMapSizeNear : RenIDeviceImpl::ShadowMapSizeFar;
    const auto fb = isNear ? shadowNearFramebuffer_ : shadowFramebuffer_;

    recordCommand(Ren::Command::setViewport(0, 0, sz, sz));
    recordCommand(Ren::Command::beginRenderPass(shadowRenderPass_, fb));
    recordCommand(Ren::Command::setDepthTest(true));
    recordCommand(Ren::Command::setDepthFunc(Ren::BackendDepthFunc::Less));
    recordCommand(Ren::Command::setDepthMaskWritable(true));
    // Disable face culling so single-sided geometry (terrain tiles) is
    // written to the shadow map.  Use polygon-offset bias instead of
    // front-face culling to avoid shadow acne.
    recordCommand(Ren::Command::setCullFace(false));
    recordCommand(Ren::Command::setPolygonOffsetFill(true));
    recordCommand(Ren::Command::setPolygonOffset(0.5f, 1.0f));

    recordCommand(Ren::Command::bindPipeline(shadowDepth_.id));
    {
        Ren::ShadowDepthUniforms sdu;
        sdu.lightSpaceMatrix = toFloatArray(lightSpaceMatrix);
        sdu.model = toFloatArray(glm::mat4(1.0f));
        recordCommand(Ren::Command::setShadowDepthUniforms(std::move(sdu)));
    }
}

void RenDevice::setShadowSplitDistance(float d)
{
    pImpl_->shadowSplitDistance_ = d;
}

void RenDevice::endShadowPass()
{
    CB_RENDEVICE_DEPIMPL_GL();

    pImpl_->shadowPassActive_ = false;

    recordCommand(Ren::Command::setPolygonOffsetFill(false));
    recordCommand(Ren::Command::setCullFace(true));
    recordCommand(Ren::Command::endRenderPass());

    // Restore the default framebuffer and main viewport.
    recordCommand(Ren::Command::bindDefaultFramebuffer());
    const auto sz = windowSize();
    recordCommand(Ren::Command::setViewport(0, 0, sz.width, sz.height));
}

bool RenDevice::isShadowPassActive() const
{
    return pImpl_->shadowPassActive_;
}

bool RenDevice::isShadowMappingEnabled() const
{
    return pImpl_->shadowMappingEnabled_;
}

void RenDevice::shadowStrength(float s)
{
    pImpl_->shadowStrength_ = s;
}

float RenDevice::shadowStrength() const
{
    return pImpl_->shadowStrength_;
}

void RenDevice::renderShadowDepth(
    const RenIVertex* vertices,
    const size_t nVertices,
    const Ren::VertexIdx* indices,
    const size_t nIndices,
    Ren::PrimitiveTopology topology)
{
    PRE(vertices);
    PRE(indices);
    PRE(pImpl_->shadowPassActive_);

    CB_RENDEVICE_DEPIMPL_GL();

    Ren::ShadowDepthPipelineHandles sh;
    sh.pipelineId = shadowDepth_.id;
    sh.posAttr = shadowDepth_.posAttr;
    sh.vertexBuffer = vertexDataBuffer_;
    sh.elementBuffer = elementBuffer_;

    Ren::ShadowDepthUniforms sdu;
    sdu.lightSpaceMatrix = toFloatArray(pImpl_->activeShadowLightSpaceMatrix_);
    sdu.model = toFloatArray(model_);

    Ren::DrawCallFactory::Commands cmds;
    Ren::DrawCallFactory::emitShadowDepthDrawIndexed(
        sh, sdu, vertices, nVertices, indices, nIndices, topology, &cmds);

    for (auto& cmd : cmds)
        recordCommand(std::move(cmd));

    recordDisableVertexAttribPointer(shadowDepth_.posAttr);
}
