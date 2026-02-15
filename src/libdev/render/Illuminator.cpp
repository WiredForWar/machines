/*
 * I L L U M I N E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "render/internal/Illuminator.hpp"

#include "render/Light.hpp"
#include "render/Mesh.hpp"
#include "render/Camera.hpp"
#include "render/internal/InternalLight.hpp"
#include "render/internal/VertexData.hpp"
#include "render/internal/LightBuffer.hpp"
#include "render/internal/MaterialBody.hpp"
#include "render/internal/GLMath.hpp"
#include "render/internal/InternalMap.hpp"
#include "render/internal/VertexMaterial.hpp"
#include "render/internal/ColourPack.hpp"
#include "render/internal/MaterialManager.hpp"
#include "render/internal/DeviceImpl.hpp"
#include "render/Device.hpp"
#include "render/LightingMode.hpp"
#include "render/RenderVariables.hpp"
#include "render/internal/InternalLight.hpp"
#include "render/internal/VertexData.hpp"
#include "render/internal/LightBuffer.hpp"
#include "render/internal/MaterialBody.hpp"
#include "render/internal/GLMath.hpp"
#include "render/internal/InternalMap.hpp"
#include "render/internal/VertexMaterial.hpp"
#include "render/internal/ColourPack.hpp"
#include "render/internal/MaterialManager.hpp"
#include "render/internal/DeviceImpl.hpp"
#include "render/LightColourTransform.hpp"
#include "render/Device.hpp"

#include "spdlog/spdlog.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

#ifndef _INLINE
#include "render/internal/Illuminator.ipp"
#endif

RenIIlluminator::RenIIlluminator(RenIDeviceImpl* d)
    : devImpl_(d)
    , ambient_(RenColour(0.3))
    , disabled_(false)
    , internallyDisabled_(false)
    , perVertexMats_(false)
    , expanded_(nullptr)
    , expansion_(nullptr)
    , lightingBuffer_(nullptr)
    , globalMaterialXform_(nullptr)
{
    PRE(d);

    lightsOn_.reserve(64);

    // NB: we can't test the invariant here.  The invariant requires that the
    // owning RenDevice is correctly set-up.  This object is created by the
    // RenDevice ctor at which point the device isn't 100% initialised, so the
    // invariant doesn't make any sense.
}

RenIIlluminator::~RenIIlluminator()
{
    // No invariant test.  See note in ctor.
    // All the RenILights are owned by their corresponding RenLight and should
    // *not* be deleted.  The lighting buffer is owned by the derived class.
    delete expansion_;
}

void RenIIlluminator::useLightingBuffer(RenILightingBuffer* l)
{
    PRE(l);
    lightingBuffer_ = l;
}

void RenIIlluminator::startFrame()
{
    // The derived class must call useLightingBuffer before doing any
    // calculations (probably in it's ctor).
    PRE(lightingBuffer_);

    computeGeneric();

    // This assumes that meshes do not change during a render-pass.  All the
    // RenMesh::add* methods should have appropriate pre-conditions.
    lightingBuffer_->checkCapacity(RenMesh::maxVertices());

    // Per-vertex intensities are disabled because they're not required in Machines.
    // if (!expansion_)
    //  expansion_ = new RenIExpandedIntensityMap(RenMesh::maxVertices());
    // ASSERT(expansion_, "No expanded intensity map in illuminator.");

    // expansion_->checkSize(RenMesh::maxVertices());
    // expanded_ = NULL;

    // Let the subclass do it's start frame initialisation (if any).
    doStartFrame();

    ctl_list<RenILight*>::iterator lightIt = lights_.begin();
    while (lightIt != lights_.end())
    {
        RenILight* light = *lightIt;
        ASSERT(light, "Null internal light in illuminator.");

        // TBD: could be removed from the production version.
        light->used(false);

        if (light->on())
            light->startFrame();

        ++lightIt;
    }

    // If the material manager has a global material transform, get it and use
    // it for the next frame.  (The transform is not stored here because there
    // may be more than one illuminator, but there's only one material manager.)
    globalMaterialXform_ = RenIMatManager::instance().globalMaterialTransform();

    POST_INFO(lightingBuffer_->capacity());
    POST_INFO(RenMesh::maxVertices());
    POST(lightingBuffer_->capacity() >= RenMesh::maxVertices());
}

// Template method: the default does nothing.
// virtual
void RenIIlluminator::doStartFrame()
{
}

void RenIIlluminator::computeGeneric()
{
    genericLambertian_ = RenColour::black();

    // Add each light to the generic illumination.
    ctl_list<RenILight*>::const_iterator lightIt = lights_.begin();
    while (lightIt != lights_.end())
    {
        const RenILight* light = *lightIt;
        ASSERT(light, "Null internal light in illuminator.");

        if (light->on())
            light->accumulateGeneric(&genericLambertian_);

        ++lightIt;
    }
}

void RenIIlluminator::addLight(RenLight* light)
{
    if (light)
    {
        RenILight* inLight = light->pILight();
        ASSERT(inLight, "Valid light has null internal light.");
        lights_.push_back(inLight);
        inLight->illuminator(this);

        if (inLight->on())
            lightTurnedOn(inLight);
    }
}

void RenIIlluminator::removeLight(RenLight* light)
{
    if (light)
    {
        RenILight* inLight = light->pILight();
        ASSERT(inLight, "Valid light has null internal light.");

        // Verify that the light is in the list.
        ASSERT_DATA(ctl_list<RenILight*>::iterator it = std::find(lights_.begin(), lights_.end(), inLight));
        ASSERT(it != lights_.end(), "Removed a light which wasn't previously added.");
        lights_.remove(inLight);
        inLight->illuminator(nullptr);

        if (inLight->on())
        {
            ctl_pvector<RenILight>::iterator it = std::find(lightsOn_.begin(), lightsOn_.end(), inLight);
            ASSERT(it != lightsOn_.end(), "On light not found in list.");
            lightsOn_.erase(it);
        }
    }
}

void RenIIlluminator::lightTurnedOn(RenILight* l)
{
    PRE(l);
    PRE(l->on());
    ASSERT(lightsOn_.end() == std::find(lightsOn_.begin(), lightsOn_.end(), l), "Light already in vector.");
    lightsOn_.push_back(l);

    internallyDisabled_ = disabled_;
}

void RenIIlluminator::lightTurnedOff(RenILight* l)
{
    PRE(l);
    PRE(!l->on());
    PRE(lightsOn_.size() > 0);
    ASSERT_DATA(size_t startSize = lightsOn_.size());

    // Clients are likely to coherently turn lights on and off in pairs.  Thus
    // it's likely that the light we are being asked to turn off is at the end
    // of the vector.  Check this first as it's probalby quicker.
    if (lightsOn_.back() == l)
    {
        lightsOn_.pop_back();
    }
    else
    {
        // It's not the last light, so there must be more than 1.
        ASSERT(lightsOn_.size() >= 2, "Light not in vector of turned on lights.");

        // The light isn't at the end, so we need to search thru the list.
        // By the same logic, searching backwards through the list will
        // probably find the light quickly.
        ctl_pvector<RenILight>::iterator it = lightsOn_.end();
        --it;

        while (it != lightsOn_.begin())
        {
            --it;

            if (*it == l)
            {
                lightsOn_.erase(it);
                break;
            }
        }
    }

    if (lightsOn_.size() == 0)
        internallyDisabled_ = true;

    // Verify that the above logic did indeed remove a light.
    ASSERT(lightsOn_.size() == startSize - 1, "Failed to remove light from vector.");
}

void RenIIlluminator::lightVertices(
    const RenIVertexData& in,
    size_t nVertices, // defines a sub-set of in
    const MexAlignedBox3d* pVolume)
{
    // The 1st pre-condition could fail if an STF has more vertices than the
    // largest (non-STF) mesh.  This is unlikely as most STFs have 4 vertices.
    PRE_INFO(in.size());
    PRE_INFO(RenMesh::maxVertices());
    PRE(in.size() <= RenMesh::maxVertices());
    PRE(in.size() <= lightingBuffer_->capacity());
    PRE(nVertices <= in.size());
    PRE_DATA(litData_ = &in);

    glm::mat4 glWorld;
    glWorld = RenDevice::current()->getModelMatrix();
    MexTransform3d mexWorld;
    GLMatrixToMex(glWorld, &mexWorld);

    // Per-vertex fog is disabled for now.  TBD: put it back in.
    // The real version: const bool isFogOn = device_->isFogOn();
    // static const bool isFogOn = FALSE_NON_CONST;
    // if (isFogOn)
    //  computeFogAttenuation(in, mexWorld);

    // Not required now that memcpy is used to initialise lit vertices?
    // vtxBuffer_->setAllSpecular(RenColour::black());
    lightingBuffer_->copyCoords(in);

    // When GPU lighting is active, expand normals into the device scratch buffer
    // so that renderPrimitive/renderIndexed can upload them as a vertex attribute.
    // Also extract light parameters for the shader uniforms.
    const bool gpuLighting = Config::gfxLightingMode.get() != LightingMode::Legacy;
    if (gpuLighting)
    {
        const size_t floatsNeeded = nVertices * 3;
        if (devImpl_->expandedNormals_.size() < floatsNeeded)
            devImpl_->expandedNormals_.resize(floatsNeeded);
        in.expandNormals(devImpl_->expandedNormals_.data(), nVertices);
        devImpl_->expandedNormalsCount_ = nVertices;

        // Expand per-vertex material overrides into flat arrays.
        // Vertices without per-vertex materials get a sentinel (-1) so the
        // shader knows to use the group material uniform instead.
        const RenIVertexMaterials* matMap = in.materialMap();
        devImpl_->hasPerVertexMaterials_ = (matMap != nullptr);
        if (matMap)
        {
            if (devImpl_->expandedVtxDiffuse_.size() < floatsNeeded)
                devImpl_->expandedVtxDiffuse_.resize(floatsNeeded);
            if (devImpl_->expandedVtxAmbient_.size() < floatsNeeded)
                devImpl_->expandedVtxAmbient_.resize(floatsNeeded);
            if (devImpl_->expandedVtxEmissive_.size() < floatsNeeded)
                devImpl_->expandedVtxEmissive_.resize(floatsNeeded);

            // Fill with sentinel (-1) meaning "use group material"
            std::fill_n(devImpl_->expandedVtxDiffuse_.data(), floatsNeeded, -1.0f);
            std::fill_n(devImpl_->expandedVtxAmbient_.data(), floatsNeeded, -1.0f);
            std::fill_n(devImpl_->expandedVtxEmissive_.data(), floatsNeeded, -1.0f);

            // Apply global material transform if present
            const auto* matXform = globalMaterialTransform();
            RenMaterial transformedMat(RenMaterial::NON_SHARABLE);

            for (auto it = matMap->begin(); it != matMap->end(); ++it)
            {
                const Ren::VertexIdx idx = (*it).index();
                if (idx >= nVertices)
                    continue;

                const RenMaterial* pMat = &(*it).material();
                if (matXform)
                {
                    matXform->transform(*pMat, &transformedMat);
                    pMat = &transformedMat;
                }

                const RenColour& d = pMat->diffuse();
                devImpl_->expandedVtxDiffuse_[idx * 3 + 0] = d.r();
                devImpl_->expandedVtxDiffuse_[idx * 3 + 1] = d.g();
                devImpl_->expandedVtxDiffuse_[idx * 3 + 2] = d.b();

                const RenColour& a = pMat->ambient();
                devImpl_->expandedVtxAmbient_[idx * 3 + 0] = a.r();
                devImpl_->expandedVtxAmbient_[idx * 3 + 1] = a.g();
                devImpl_->expandedVtxAmbient_[idx * 3 + 2] = a.b();

                const RenColour& e = pMat->emissive();
                devImpl_->expandedVtxEmissive_[idx * 3 + 0] = e.r();
                devImpl_->expandedVtxEmissive_[idx * 3 + 1] = e.g();
                devImpl_->expandedVtxEmissive_[idx * 3 + 2] = e.b();
            }
        }

        // Sum all directional light contributions into a single direction/color pair.
        // For multiple directional lights, we accumulate colors and use the last direction.
        // Attenuated lights (point + uniform) are collected into GPU arrays.
        const auto* colourXform = RenILight::globalColourTransform();

        devImpl_->gpuLightDir_ = glm::vec3(0.0f, -1.0f, 0.0f);
        devImpl_->gpuLightColor_ = glm::vec3(0.0f);
        int nPt = 0;
        for (const RenILight* light : lightsOn_)
        {
            if (const auto* dirLight = dynamic_cast<const RenIDirectionalLight*>(light))
            {
                const MexVec3& dir = dirLight->direction();
                devImpl_->gpuLightDir_ = glm::vec3(dir.x(), dir.y(), dir.z());
                RenColour col = dirLight->colour();
                if (colourXform)
                    colourXform->transform(col, Ren::DIRECTIONAL, &col);
                devImpl_->gpuLightColor_ += glm::vec3(col.r(), col.g(), col.b());
            }
            else if (const auto* attLight = dynamic_cast<const RenIAttenuatedLight*>(light))
            {
                if (nPt < RenIDeviceImpl::MaxGpuPointLights)
                {
                    const bool isUniform = dynamic_cast<const RenIUniformLight*>(light) != nullptr;
                    const MexPoint3d& pos = attLight->position();
                    devImpl_->gpuPointLightPos_[nPt] = glm::vec3(pos.x(), pos.y(), pos.z());
                    RenColour col = attLight->colour();
                    if (colourXform)
                        colourXform->transform(col, isUniform ? Ren::UNIFORM : Ren::POINT, &col);
                    devImpl_->gpuPointLightColor_[nPt] = glm::vec3(col.r(), col.g(), col.b());
                    devImpl_->gpuPointLightRange_[nPt] = attLight->maxRange();
                    devImpl_->gpuPointLightAtten_[nPt] = glm::vec3(
                        attLight->constantAttenuation(),
                        attLight->linearAttenuation(),
                        attLight->quadraticAttenuation());
                    devImpl_->gpuPointLightOmni_[nPt] = isUniform ? 1.0f : 0.0f;
                    ++nPt;
                }
            }
        }
        devImpl_->gpuNumPointLights_ = nPt;
        devImpl_->gpuAmbientColor_ = glm::vec3(ambient_.r(), ambient_.g(), ambient_.b());
    }
    else
    {
        devImpl_->expandedNormalsCount_ = 0;
        devImpl_->hasPerVertexMaterials_ = false;
    }

    if (!disabled())
        computeLambertian(in, mexWorld, pVolume);

    // If these vertices have an intensity map, then create an expanded version.
    const RenIVertexIntensities* intMap = in.intensityMap();

    // For now, support for per-vertex intensities is disabled because we don't
    // use this feature in Machines and the subsequent lighting code can be more
    // efficient without it.
    ASSERT(!intMap, "Mesh has intensity map.");

    if (intMap != expanded_)
    {
        // This should be allocated by start frame.
        ASSERT(expansion_, "No expanded intensity map in illuminator.");
        expanded_ = intMap;
        expansion_->expand(expanded_);
    }

    // If these vertices have a material map, then apply the material at
    // whichever vertices are affected.
    const RenIVertexMaterials* matMap = in.materialMap();
    perVertexMats_ = (matMap != nullptr);

    if (perVertexMats_)
    {
        // Reset these flags to indicate that each vertex doesn't (yet) have its
        // own material applied to it.
        lightingBuffer_->setAllPerVertexFlags(false);

        // This function will set some of the above flags to true.
        applyVertexMaterials(in);
    }

    POST(lightingBuffer_->sizeMatches(nVertices));
    POST(litData_ == &in);
}

void RenIIlluminator::lightVertices(const RenIVertexData& in, const MexAlignedBox3d* pVolume)
{
    PRE(in.size() <= RenMesh::maxVertices());

    lightVertices(in, in.size(), pVolume);

    POST(lightingBuffer_->sizeMatches(in.size()));
    POST(litData_ == &in);
}

/* TBD: put vertex fog back in; create RenIMaterialApplicator subclasses for
 * fog.  Fog calculation looks like this:
    const RenColour& fogColour = device_->fogColour();
    if (isFogOn)
    {
        // Blend the fog colour with the calculated reflected colour,
        // using the attenuation factor as the blend proportion.
        const float att = *pAtts++;
        const float oneMinusAtt = 1 - att;
        const float r = oneMinusAtt * (lambR * difR + constR) + att * fogColour.r();
        const float g = oneMinusAtt * (lambG * difG + constG) + att * fogColour.g();
        const float b = oneMinusAtt * (lambB * difB + constB) + att * fogColour.b();

        vtx.color = RenICheckedMatApp(r,g,b, alpha_);
    }
*/

const RenIVertex* RenIIlluminator::applyMaterial(const RenMaterial& m, const RenIVertexData& in)
{
    PRE(in.size() == litData_->size());
    PRE(lightingBuffer_->sizeMatches(in.size()));
    return applyMaterial(m, in, in.size());
}

RenI::LitVtxAPtr RenIIlluminator::applyMaterialAndCopy(
    const RenMaterial& m,
    const RenIVertexData& in,
    const ctl_min_memory_vector<Ren::VertexIdx>& indices,
    Ren::VertexIdx nIndicesUsed)
{
    PRE(nIndicesUsed <= in.size());

    const RenIVertex* vtx = applyMaterial(m, in, indices);
    RenIVertex* copy = _NEW_ARRAY(RenIVertex, nIndicesUsed);
    memcpy(copy, vtx, nIndicesUsed * sizeof(RenIVertex));
    return RenI::LitVtxAPtr(copy);
}

RenI::LitVtxAPtr RenIIlluminator::applyMaterialAndCopy(const RenMaterial& m, const RenIVertexData& in, size_t nVertices)
{
    PRE(nVertices <= in.size());

    const RenIVertex* vtx = applyMaterial(m, in, nVertices);
    RenIVertex* copy = _NEW_ARRAY(RenIVertex, nVertices);
    memcpy(copy, vtx, nVertices * sizeof(RenIVertex));
    return RenI::LitVtxAPtr(copy);
}

static void resizeIndices(ctl_min_memory_vector<Ren::VertexIdx>& v, size_t newSize)
{
    if (newSize > v.size())
    {
        v.reserve(newSize);

        // Each element of the vector is it's own index.
        for (size_t i = v.size(); i != newSize; ++i)
            v.push_back(static_cast<Ren::VertexIdx>(i));
    }
    else if (newSize < v.size())
    {
        v.erase(v.begin() + newSize, v.end());
    }

    POST(v.size() == newSize);
}

// Construct a vector of indices and call the virtual method which applys the
// material indirectly via the indices.  This can be done slightly more
// efficiently without the indirection.  However, a) it is only called for spin-
// to-face polys, i.e., infrequently; b) only 3 or 4 vertices are involved;
// c) it simplifies the code that does the low-level work; and d) it means we
// can considerably reduce the amount of low-level code.

const RenIVertex* RenIIlluminator::applyMaterial(const RenMaterial& mat, const RenIVertexData& in, size_t nVertices)
{
    static ctl_min_memory_vector<Ren::VertexIdx> indices;

    // This function is used by spin-to-face polys.  The resize might be horribly
    // inefficient; however, spin-to-face polys always have 4 (occaisionally 3)
    // vertices, so the resize should not be expensive.
    resizeIndices(indices, nVertices);

    applyMaterial(mat, in, indices);
    return lightingBuffer_->vertices();
}

void RenIIlluminator::applyMaterialGenericLighting(const RenMaterial& mat, RenIVertex* vertices, size_t nVertices)
{
    PRE(vertices);

    RenColour result = genericLambertian_;
    result *= mat.diffuse();
    RenColour amb = ambient_;
    amb *= mat.ambient();
    result += amb;
    result += mat.emissive();

    // Important: apply the filter to the result.
    result *= filter_;

    // Assemble the result into D3D format and modify all the input vertices.
    // The above calculations may have done all sorts of things to the alpha
    // value, depending on the emissive and ambient alpha values.  The one we
    // should use is the diffuse alpha (unmodified).
    const uint glResult = packColourChecked(result.r(), result.g(), result.b(), mat.diffuse().packedAlpha());
    static const uint packedBlack = packColourUnChecked(0, 0, 0, 0);

    for (int i = 0; i != nVertices; ++i)
    {
        {
            vertices[i].color = glResult;
        }
        vertices[i].specular = packedBlack;
    }
}

void RenIIlluminator::CLASS_INVARIANT
{
    INVARIANT(devImpl_);
    INVARIANT(
        implies(RenIDeviceImpl::currentPimpl()->rendering(), lightingBuffer_->capacity() >= RenMesh::maxVertices()));
}

std::ostream& operator<<(std::ostream& o, const RenIIlluminator& t)
{
    // TBD: if/when you can print a RenILight, make this a bit more verbose.
    o << "Scene illuminator with " << t.lights_.size() << " lights\n";
    return o;
}

/* End ILLUMINE.CPP *************************************************/
