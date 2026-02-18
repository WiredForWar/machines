/*
 * S C E N E M G R . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "world4d/Scene/SceneManager.hpp"

#include <algorithm>
#include <stdio.h>

#include "base/Diag.hpp"
#include "ctl/List.hpp"

#include "device/Timer.hpp"

#include "mathex/Point3d.hpp"
#include "mathex/Line3d.hpp"
#include "mathex/EulerAngles.hpp"
#include "mathex/Transform3d.hpp"
#include "mathex/Sphere3d.hpp"
#include "mathex/AlignedBox3d.hpp"

#include "render/Device.hpp"
#include "render/Stats.hpp"
#include "render/Colour.hpp"
#include "render/LightingMode.hpp"
#include "render/ShadowQuality.hpp"
#include "render/RenderVariables.hpp"

#include "world4d/Entity/Root.hpp"
#include "world4d/Scene/Camera.hpp"
#include "world4d/Scene/Light.hpp"
#include "world4d/Scene/Environment.hpp"
#include "world4d/Scene/CameraShake.hpp"
#include "world4d/Scene/Domain.hpp"
#include "world4d/Scene/DomainAssignor.hpp"
#include "world4d/Entity/Composite.hpp"
#include "world4d/Internal/Complexity.hpp"
#include "world4d/Scene/Internal/LightImpl.hpp"

#include <glm/gtc/matrix_transform.hpp>

// #define DO_CULL_TRACING
#ifdef DO_CULL_TRACING
#define LIGHT_CULL_STREAM(x) IAIN_STREAM(x)
#define LIGHT_CULL_TRACE(x) x
#define LIGHT_CULL_INDENT(x) IAIN_INDENT(x)
#else
#define LIGHT_CULL_STREAM(x) ;
#define LIGHT_CULL_TRACE(x) ;
#define LIGHT_CULL_INDENT(x) ;
#endif

class W4dSceneManagerImpl
{

public:
    W4dSceneManagerImpl(std::unique_ptr<RenDevice> pDevice, W4dRoot* root);
    ~W4dSceneManagerImpl();

private:
    friend class W4dSceneManager;

    double frameRate();
    void updateComplexityLevel();

    //  W4dRoot* const      root_;
    //  Changed to non-const to allow the root to be updated after construction.
    //  This is necessary for persistence.
    W4dRoot* root_{};
    W4dCamera* currentCamera_{};
    W4dDomainAssignor* domainAssignor_{};

    // Scene background stuff.
    bool clearBg_;
    W4dRoot* bgRoot_{};
    W4dEnvironment* environment_{};

    //  Storage for pushed background details. One day this might
    //  be done using a stack, for the time being we'll pretend it's
    //  a stack that can only have one item on it.
    bool itemsStacked_{};
    bool stackClearBg_;
    W4dRoot* pStackBgRoot_{};
    RenColour stackBackgroundColour_;
    int maxDomainRenderDepth_ = 0;

    // Rendering statistics stuff.
    uint totalEntities_{};
    uint totalDomains_{};

    // There are two lists of lights, one that contains all the lights and
    // another which contains only the dynamic ones.
    using Lights = ctl_pvector<W4dLight>;
    using LightPimpls = ctl_pvector<W4dLightImpl>;
    Lights lights_;
    LightPimpls dynamicLights_;
    uint nGlobalLights_, nLocalLights_, nDynamicLights_;

    std::unique_ptr<RenDevice> const device_;
    W4dCameraShake* pCameraShake_{};

    // set this to true if we want the complexity of the scene
    // to be adjusted so as to guarantee an almost constant frame rate
    W4dComplexity complexity_;

    DevTimer autoAdjustUpdateTimer_;
    double requestedMinFrameRate_;
    double highEnoughFrameRate_;

    static double requestedMinFrameRateInit_;
    static double highEnoughFrameRateInit_;

    bool dynamicLightsEnabled_;
};

// De-pimple everything in the class.
#define CB_SCENEMANAGER_DEPIMPL                                                                                        \
    CB_DEPIMPL(W4dRoot*, root_);                                                                                       \
    CB_DEPIMPL(W4dCamera*, currentCamera_);                                                                            \
    CB_DEPIMPL(W4dDomainAssignor*, domainAssignor_);                                                                   \
    CB_DEPIMPL(bool, clearBg_);                                                                                        \
    CB_DEPIMPL(W4dRoot*, bgRoot_);                                                                                     \
    CB_DEPIMPL(W4dEnvironment*, environment_);                                                                         \
    CB_DEPIMPL(bool, itemsStacked_);                                                                                   \
    CB_DEPIMPL(bool, stackClearBg_);                                                                                   \
    CB_DEPIMPL(W4dRoot*, pStackBgRoot_);                                                                               \
    CB_DEPIMPL(RenColour, stackBackgroundColour_);                                                                     \
    CB_DEPIMPL(uint, totalEntities_);                                                                                  \
    CB_DEPIMPL(uint, totalDomains_);                                                                                   \
    typedef W4dSceneManagerImpl::Lights Lights;                                                                        \
    CB_DEPIMPL(Lights, lights_);                                                                                       \
    CB_DEPIMPL(std::unique_ptr<RenDevice> const, device_);                                                             \
    CB_DEPIMPL(W4dCameraShake*, pCameraShake_);                                                                        \
    CB_DEPIMPL(W4dComplexity, complexity_);                                                                            \
    CB_DEPIMPL(DevTimer, autoAdjustUpdateTimer_);                                                                      \
    CB_DEPIMPL(double, requestedMinFrameRateInit_);                                                                    \
    CB_DEPIMPL(double, highEnoughFrameRateInit_);                                                                      \
    CB_DEPIMPL(double, requestedMinFrameRate_);                                                                        \
    CB_DEPIMPL(double, highEnoughFrameRate_);                                                                          \
    CB_DEPIMPL(bool, dynamicLightsEnabled_);

// static
MATHEX_SCALAR W4dSceneManagerImpl::requestedMinFrameRateInit_ = 20;
MATHEX_SCALAR W4dSceneManagerImpl::highEnoughFrameRateInit_ = 30;

//////////////////////////////////////////////////////////////////////////////
W4dSceneManagerImpl::W4dSceneManagerImpl(std::unique_ptr<RenDevice> pDevice, W4dRoot* root)
    : root_(root)
    , clearBg_(true)
    , device_(std::move(pDevice))
    , requestedMinFrameRate_(requestedMinFrameRateInit_)
    , highEnoughFrameRate_(highEnoughFrameRateInit_)
    , dynamicLightsEnabled_(true)
{
    complexity_.enabled(false);
    lights_.reserve(512);

    maxDomainRenderDepth_ = 36;
}

W4dSceneManagerImpl::~W4dSceneManagerImpl()
{
    delete pCameraShake_;
}

void W4dSceneManagerImpl::updateComplexityLevel()
{
    double framerate = frameRate();

    if (framerate < requestedMinFrameRate_)
    {
        complexity_.decrease();
    }
    else if (framerate > highEnoughFrameRate_)
    {
        complexity_.increase();
    }
}

double W4dSceneManagerImpl::frameRate()
{
    return device_->statistics()->frameRate();
}

//////////////////////////////////////////////////////////////////////////////
W4dSceneManager::W4dSceneManager(std::unique_ptr<RenDevice> pDevice, W4dRoot* root)
    : pImpl_(new W4dSceneManagerImpl(std::move(pDevice), root))
{
    CB_SCENEMANAGER_DEPIMPL;
    PRE(root);
    PRE(!root->hasParent());
    ASSERT(device_, runtime_error("Out of memory"));
}

W4dSceneManager::~W4dSceneManager()
{
    TEST_INVARIANT;
    delete pImpl_;
}

void W4dSceneManager::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

static void countLights(const ctl_pvector<W4dLight>& lights, uint nGlobal, uint nLocal, uint nDynamic, std::ostream& o)
{
    uint globalUsed = 0, localUsed = 0, dynamicUsed = 0;

    for (ctl_pvector<W4dLight>::const_iterator i = lights.begin(); i != lights.end(); ++i)
    {
        const W4dLight* light = *i;

        if (light->wasUsedThisFrame())
        {
            if (light->isGlobal())
                ++globalUsed;
            else if (light->isLocal())
                ++localUsed;
            else if (light->isDynamic())
                ++dynamicUsed;
        }
    }

    o << "Lights (used/total): glob " << globalUsed << "/" << nGlobal;
    o << ", local " << localUsed << "/" << nLocal;
    o << ", dynamic " << dynamicUsed << "/" << nDynamic << "\n";
}

// Render the tree of W4dEntities that the object is managing.
void W4dSceneManager::render()
{
    CB_SCENEMANAGER_DEPIMPL;
    CB_DEPIMPL(uint, nGlobalLights_);
    CB_DEPIMPL(uint, nLocalLights_);
    CB_DEPIMPL(uint, nDynamicLights_);

    // A camera must have been specified.
    PRE(currentCamera());

    LIGHT_CULL_STREAM("\n\n*** Start frame\n\n");

    // update the complexity level
    if (autoAdjustFrameRate())
    {
        if (autoAdjustUpdateTimer_.time() > 0.25)
        {
            autoAdjustUpdateTimer_.time(0);
            pImpl_->updateComplexityLevel();
        }

        if (complexity().highestComplexityReached())
            device_->out() << "Maximale ";
        else if (complexity().lowestComplexityReached())
            device_->out() << "Minimale ";
        else
            device_->out() << "Adjustable ";
        device_->out() << " scene complexity" << std::endl;

        char buffer[1000];
        snprintf(
            buffer,
            sizeof(buffer),
            " target frame rate [%.1f, %.1f]; frame rate %.1f\n",
            requestedMinFrameRate_,
            highEnoughFrameRate_,
            pImpl_->frameRate());
        device_->out() << buffer;
        complexity().usePriority(0);
        device_->out() << " low  priority machines (level, range multiplier): (" << complexity().level() << ", "
                       << complexity().rangeMultiplier() << ")" << std::endl;
        complexity().usePriority(1);
        device_->out() << " high priority machines (level, range multiplier): (" << complexity().level() << ", "
                       << complexity().rangeMultiplier() << ")" << std::endl;
    }

    MexTransform3d unshakenLocalXform = currentCamera_->localTransform();
    if (shakeCamera() && pCameraShake_->shakeIsOver())
        cancelCameraShake();

    if (shakeCamera())
        shakeCurrentCamera();

    if (environment_)
        environment_->update();

    updateLights();
    currentCamera_->updateRenCamera();

    // update the camera far clipping plane to the fog distance
    currentCamera_->adaptToEnvironment(environment());
    //  device_->out() << "Distance far clipping plane: " << currentCamera()->yonClipDistance() << " m" << std::endl;

    device_->start3D();

    // Shadow depth pass: render scene from the light's perspective into the
    // shadow map.  Runs after start3D() (rendering context is ready) but
    // before beginGeometryPass() so the shadow maps are complete before the
    // main geometry pass samples them.
    const bool gpuLighting = Config::gfxLightingMode.get() != LightingMode::Legacy;
    const bool wantShadows = gpuLighting && Config::gfxShadowQuality.get() != ShadowQuality::Static;
    if (wantShadows)
    {
        // Find the first global directional light for the shadow caster.
        W4dDirectionalLight* shadowLight{};
        for (auto* light : lights_)
        {
            if (light->isGlobal() && light->isOn())
            {
                shadowLight = dynamic_cast<W4dDirectionalLight*>(light);
                if (shadowLight)
                    break;
            }
        }

        if (shadowLight)
        {
            const MexVec3& dir = shadowLight->direction();
            const glm::vec3 lightDir(dir.x(), dir.y(), dir.z());

            const MexPoint3d camPos = currentCamera_->globalTransform().position();
            const MexPoint3d frustumCenter = shadowFrustumCenter();
            const glm::vec3 eye(camPos.x(), camPos.y(), camPos.z());
            const glm::vec3 center(frustumCenter.x(), frustumCenter.y(), frustumCenter.z());

            // Dynamic cascade extents based on camera height (zoom level).
            // Low cameras (z ≈ 2..5) get tight cascades; zenith (z ≈ 20..250)
            // gets progressively larger ones.
            const float camZ = static_cast<float>(camPos.z());
            const float heightT = std::clamp((camZ - 2.0f) / (250.0f - 2.0f), 0.0f, 1.0f);
            const float nearExtent = glm::mix(30.0f, 200.0f, heightT);
            const float farExtent = glm::mix(80.0f, 300.0f, heightT);

            const glm::vec3 up = (std::abs(glm::dot(glm::normalize(lightDir), glm::vec3(0, 0, 1))) > 0.99f)
                ? glm::vec3(0, 1, 0)
                : glm::vec3(0, 0, 1);

            // The shader uses view-space distance (length(viewSpace)) to pick
            // the cascade.  Compute the view-space distance from the camera to
            // the edge of the near cascade frustum so the split is consistent.
            const float distToCenter = glm::length(center - eye);
            const float splitViewDist = distToCenter + nearExtent;
            device_->setShadowSplitDistance(splitViewDist);

            // --- Near cascade: high-resolution shadows for close objects ---
            {
                const glm::vec3 nearLightPos = center - lightDir * (nearExtent * 2.0f);
                const glm::mat4 nearView = glm::lookAt(nearLightPos, center, up);
                const glm::mat4 nearProj = glm::ortho(
                    -nearExtent, nearExtent,
                    -nearExtent, nearExtent,
                    1.0f, nearExtent * 5.0f);
                const glm::mat4 nearMatrix = nearProj * nearView;

                device_->beginShadowPass(RenDevice::ShadowCascade::Near, nearMatrix);
                currentCamera_->domainRender(pImpl_->maxDomainRenderDepth_);
                device_->endShadowPass();
            }

            // --- Far cascade: lower-resolution shadows for distant objects ---
            {
                const glm::vec3 farLightPos = center - lightDir * (farExtent * 2.0f);
                const glm::mat4 farView = glm::lookAt(farLightPos, center, up);
                const glm::mat4 farProj = glm::ortho(
                    -farExtent, farExtent,
                    -farExtent, farExtent,
                    1.0f, farExtent * 2.5f);
                const glm::mat4 farMatrix = farProj * farView;

                device_->beginShadowPass(RenDevice::ShadowCascade::Far, farMatrix);
                currentCamera_->domainRender(pImpl_->maxDomainRenderDepth_);
                device_->endShadowPass();
            }
        }
    }

    device_->beginGeometryPass(clearBg_);

    // Attempt a domain render. If the camera is not in a domain, it will use the inOrderRender method.
    currentCamera_->domainRender(pImpl_->maxDomainRenderDepth_);

    totalEntities_ += currentCamera_->entitiesRendered();
    totalDomains_ += currentCamera_->domainsRendered();

    // Render any background items last.
    if (bgRoot_)
    {
        // Ensure that the camera doesn't clip entities at the original far cliping plane
        currentCamera_->isYonClippingEnabled(false);
        device_->startBackground(110000);
        currentCamera_->renderTree(bgRoot_, W4dCamera::IN_ORDER_RENDER);
        currentCamera_->isYonClippingEnabled(true);
    }

    device_->end3D();

    if (shakeCamera())
        currentCamera_->localTransform(unshakenLocalXform);

    if (device_->statistics() && device_->statistics()->shown())
        countLights(lights_, nGlobalLights_, nLocalLights_, nDynamicLights_, device_->out());
}

// Do the per-frame work necessary for each light.
void W4dSceneManager::updateLights()
{
    CB_SCENEMANAGER_DEPIMPL;
    CB_DEPIMPL(uint, nGlobalLights_);
    CB_DEPIMPL(uint, nLocalLights_);
    CB_DEPIMPL(uint, nDynamicLights_);

    // Reset the counts of active lights per frame.
    nGlobalLights_ = nLocalLights_ = nDynamicLights_ = 0;

    for (Lights::iterator i = lights_.begin(); i != lights_.end(); ++i)
    {
        W4dLight* light = *i;

        if (light->isGlobal())
        {
            light->lazyUpdate();
            ++nGlobalLights_;
        }

        // In PerPixel mode, treat LOCAL lights as DYNAMIC so they illuminate
        // everything nearby via domain assignment instead of only their
        // explicitly-assigned entities.
        const bool promoteLocal = Config::gfxLightingMode.get() == LightingMode::PerPixel;

        if (light->isLocal() && !promoteLocal)
        {
            // Legacy path: disable local lights; they will be re-enabled
            // per-entity during the render traversal.
            ++nLocalLights_;
            light->disable();
        }

        // If a client has defined a domain assignor, use it to give every
        // light domains.
        if (light->isDynamic() || (light->isLocal() && promoteLocal))
        {
            // Unfortunately, we must update the dynamic lights so that their
            // positions are correct for domain assignment.
            light->lazyUpdate();

            light->disable();
            ++nDynamicLights_;

            if (dynamicLightsEnabled_)
            {
                if (!domainAssignor_)
                {
                    LIGHT_CULL_STREAM("*Warning: scene manager has no domain assignor.\n");
                }
                else
                {
                    domainAssignor_->assignDomains(root_, light);

                    // Start by unilluminating any previous entities.
                    light->illuminateNone();

                    LIGHT_CULL_STREAM("Light " << (W4dEntity*)light << "\n");

                    // Then add the containing domain and intersecting domains (if any).
                    W4dDomain* containing;
                    if (light->hasContainingDomain(&containing))
                    {
                        LIGHT_CULL_STREAM("  illuminates containing " << (W4dEntity*)containing << "\n");
                        light->illuminate(containing);
                    }

                    const W4dEntity::W4dDomains& intersects = light->intersectingDomains();
                    W4dEntity::W4dDomains::const_iterator it = intersects.begin();
                    while (it != intersects.end())
                    {
                        light->illuminate(*it);
                        LIGHT_CULL_STREAM("  illuminates " << (W4dEntity*)(*it) << "\n");

                        ++it;
                    }

                    LIGHT_CULL_TRACE(if (intersects.size() == 0))
                    LIGHT_CULL_STREAM("  illuminates nothing\n");
                }
            }
        }
    }
}

void W4dSceneManager::setFrameRateForDebug()
{
    CB_SCENEMANAGER_DEPIMPL;

    double veryHigh = 60;
    double veryVeryHigh = 80;

    requestedMinFrameRate_ = veryHigh;
    highEnoughFrameRate_ = veryVeryHigh;
    autoAdjustFrameRate(true);
}

bool W4dSceneManager::autoAdjustFrameRate() const
{
    CB_SCENEMANAGER_DEPIMPL;
    return complexity_.enabled();
}

void W4dSceneManager::autoAdjustFrameRate(bool setautoAdjustFrameRate)
{
    CB_SCENEMANAGER_DEPIMPL;
    complexity_.enabled(setautoAdjustFrameRate);
    if (autoAdjustFrameRate())
        pImpl_->autoAdjustUpdateTimer_.time(0);
    LIONEL_STREAM("W4dSceneManager::autoAdjustFrameRate(): " << autoAdjustFrameRate() << std::endl);
}

const double& W4dSceneManager::requestedMinFrameRate() const
{
    CB_SCENEMANAGER_DEPIMPL;
    return requestedMinFrameRate_;
}

void W4dSceneManager::requestedMinFrameRate(const double& setRequestedMinFrameRate)
{
    CB_SCENEMANAGER_DEPIMPL;

    requestedMinFrameRate_ = setRequestedMinFrameRate;
}

const double& W4dSceneManager::highEnoughFrameRate() const
{
    CB_SCENEMANAGER_DEPIMPL;
    return highEnoughFrameRate_;
}

void W4dSceneManager::highEnoughFrameRate(const double& sethighEnoughFrameRate)
{
    PRE(sethighEnoughFrameRate > requestedMinFrameRate());

    CB_SCENEMANAGER_DEPIMPL;
    highEnoughFrameRate_ = sethighEnoughFrameRate;
}

const W4dComplexity& W4dSceneManager::complexity() const
{
    CB_SCENEMANAGER_DEPIMPL;

    return complexity_;
}

W4dComplexity& W4dSceneManager::complexity()
{
    CB_SCENEMANAGER_DEPIMPL;

    return complexity_;
}

void W4dSceneManager::cullDynamicLights(const W4dEntity* entity)
{
    CB_SCENEMANAGER_DEPIMPL;
    typedef W4dSceneManagerImpl::LightPimpls LightPimpls;
    CB_DEPIMPL(LightPimpls, dynamicLights_);
    CB_DEPIMPL(uint, nDynamicLights_);

    LIGHT_CULL_STREAM("Culling against lights for " << entity << "\n");
    LIGHT_CULL_INDENT(2);

    // This is a real quick test which will obviate even the bounding volume test in
    // some cases (unfortunately not in battles where it's most critical).
    if (nDynamicLights_ == 0)
    {
        LIGHT_CULL_STREAM("Skipping becuase there are zero dynamic lights.\n");
    }
    else
    {
        const MexAlignedBox3d* box = nullptr;
        if (entity->isComposite())
            box = &(entity->asComposite().compositeBoundingVolume());
        else
            box = &(entity->boundingVolume());
        ASSERT(box, "logic error failed to set box pointer");

        const MATHEX_SCALAR entityRadius = box->maxCorner().euclidianDistance(box->minCorner()) / 2;
        const MexPoint3d& entityPosition = entity->globalTransform().position();
        LIGHT_CULL_STREAM("Entity radius=" << entityRadius << "\n");
        LIGHT_CULL_STREAM("Entity pos=" << entityPosition << "\n");

        for (LightPimpls::iterator i = dynamicLights_.begin(); i != dynamicLights_.end(); ++i)
        {
            W4dLightImpl* lightImpl = *i;
            ASSERT(lightImpl, "Found null pimple in dynamic light list.");

            if (lightImpl->isDynamicEnabledOn())
            {
                LIGHT_CULL_STREAM("Considering light " << (W4dEntity*)*i << "\n");
                LIGHT_CULL_INDENT(2);

                if (! lightImpl->hasBoundingSphere())
                {
                    LIGHT_CULL_STREAM("light has no sphere => rejecting\n");
                    lightImpl->dynamicDisable(false);
                }
                else
                {
                    // check if the entity intersects the light
                    const MexSphere3d& sphere = lightImpl->boundingSphere(pImpl_->currentCamera_);
                    const MexPoint3d& lightCenter = sphere.center();

                    const MATHEX_SCALAR centersDist = lightCenter.euclidianDistance(entityPosition);
                    const bool result = centersDist > sphere.radius() + entityRadius;

                    LIGHT_CULL_STREAM("light radius=" << sphere.radius() << "\n");
                    LIGHT_CULL_STREAM("light-entity dist=" << centersDist << "\n");

                    LIGHT_CULL_STREAM("enabled=" << (!result ? "true" : "false") << "\n");

                    lightImpl->dynamicDisable(result);
                }

                LIGHT_CULL_INDENT(-2);
            }
        }
    }

    LIGHT_CULL_INDENT(-2);
}

W4dCamera* W4dSceneManager::currentCamera() const
{
    return pImpl_->currentCamera_;
}

void W4dSceneManager::useCamera(W4dCamera* cam)
{
    CB_SCENEMANAGER_DEPIMPL;

    PRE(cam);
    PRE(cam->findRoot() == root_);
    PRE(cam->hitherClipDistance() < cam->yonClipDistance());

    currentCamera_ = cam;
    device_->useCamera(cam->pRenCamera());

    // modify the camera transform key so as to prevent lazy evaluation to occur at the first
    // render pass
    MexTransform3d trans(currentCamera_->globalTransform());
    // do nothing
    trans.translate(MexPoint3d());
    currentCamera_->globalTransform(trans);
}

void W4dSceneManager::useLevelOfDetail(bool enableLod)
{
    complexity().enabledLOD(enableLod);
}

void W4dSceneManager::setMaxDomainDepth(int maxDepth)
{
    pImpl_->maxDomainRenderDepth_ = maxDepth;
}

void W4dSceneManager::ambient(const RenColour& amb)
{
    pImpl_->device_->ambient(amb);
}

const RenColour& W4dSceneManager::ambient() const
{
    return pImpl_->device_->ambient();
}

void W4dSceneManager::environment(W4dEnvironment* e)
{
    pImpl_->environment_ = e;
}

W4dEnvironment* W4dSceneManager::environment()
{
    return pImpl_->environment_;
}

void W4dSceneManager::clearBackground(bool f)
{
    pImpl_->clearBg_ = f;
}

bool W4dSceneManager::clearBackground() const
{
    return pImpl_->clearBg_;
}

void W4dSceneManager::backgroundColour(const RenColour& bg)
{
    pImpl_->device_->backgroundColour(bg);
}

const RenColour& W4dSceneManager::backgroundColour() const
{
    return pImpl_->device_->backgroundColour();
}

void W4dSceneManager::useBackground(W4dRoot* r)
{
    pImpl_->bgRoot_ = r;
}

void W4dSceneManager::useRoot(W4dRoot* r)
{
    pImpl_->root_ = r;
}

void W4dSceneManager::showStats(double updateInterval)
{
    RenStats* stats = pImpl_->device_->statistics();

    if (stats)
    {
        stats->show();
        stats->displayInterval(updateInterval);
    }
}

void W4dSceneManager::hideStats()
{
    RenStats* stats = pImpl_->device_->statistics();

    if (stats)
        stats->hide();
}

void W4dSceneManager::addLight(W4dLight* pLight)
{
    CB_SCENEMANAGER_DEPIMPL;
    typedef W4dSceneManagerImpl::LightPimpls LightPimpls;
    CB_DEPIMPL(LightPimpls, dynamicLights_);
    PRE(pLight);

    // Not all lights are attached to the root of the actual rendered world.
    // Some are attached to exemplar roots. This light will be added to the
    // current scene manager only iff it shares the same root.  A bit crufty.
    // The scene manager-root concept needs a redesign.
    W4dRoot* lightsRoot = pLight->findRoot();
    if (root_ == lightsRoot || (bgRoot_ && bgRoot_ == lightsRoot))
    {
        lights_.push_back(pLight);
        device_->addLight(pLight->pRenLight());

        // Maintain a second list containing only the dynamic lights (or rather their
        // pimples).  This assumes that a light won't change it's pimple during its life.
        if (pLight->isDynamic())
            dynamicLights_.push_back(&(pLight->impl()));
    }
}

void W4dSceneManager::removeLight(W4dLight* pLight)
{
    CB_SCENEMANAGER_DEPIMPL;
    typedef W4dSceneManagerImpl::LightPimpls LightPimpls;
    CB_DEPIMPL(LightPimpls, dynamicLights_);
    PRE(pLight);

    // If it's a dynamic light, remove it from the dynamic-only list.
    if (pLight->isDynamic())
    {
        LightPimpls::iterator found = find(dynamicLights_.begin(), dynamicLights_.end(), &(pLight->impl()));
        if (found != dynamicLights_.end())
            dynamicLights_.erase(found);
    }

    Lights::iterator found = find(lights_.begin(), lights_.end(), pLight);
    if (found != lights_.end())
    {
        lights_.erase(found);
        device_->removeLight(pLight->pRenLight());
    }
}

void W4dSceneManager::turnOffAllLocalLights()
{
    CB_SCENEMANAGER_DEPIMPL;

    for (Lights::iterator i = lights_.begin(); i != lights_.end(); ++i)
    {
        W4dLight* light = *i;

        if (light->isLocal())
            light->disable();
    }
}

void W4dSceneManager::updateDisplay()
{
}

std::ostream& W4dSceneManager::out()
{
    return pImpl_->device_->out();
}

RenDevice* W4dSceneManager::pDevice()
{
    return pImpl_->device_.get();
}

// TBD: this is total hack for Milestone IX.  It ought to be a member
// variable of W4dSceneManager, but I wish to avoid a header file change.
static W4dEnvironment* w4dPushedEnv = nullptr;

void W4dSceneManager::pushBackgroundData()
{
    CB_SCENEMANAGER_DEPIMPL;
    PRE(! itemsStacked_);

    stackClearBg_ = clearBackground();
    pStackBgRoot_ = bgRoot_;
    stackBackgroundColour_ = backgroundColour();

    // Total hack for Milestone IX.
    environment_->disable();
    ::w4dPushedEnv = environment_;
    environment_ = nullptr;
    ambient(0.0);

    itemsStacked_ = true;

    POST(itemsStacked_);
}

void W4dSceneManager::popBackgroundData()
{
    CB_SCENEMANAGER_DEPIMPL;
    PRE(itemsStacked_);

    clearBackground(stackClearBg_);
    useBackground(pStackBgRoot_);
    backgroundColour(stackBackgroundColour_);

    // Total hack for Milestone IX.
    environment_ = ::w4dPushedEnv;
    environment_->enable();

    itemsStacked_ = false;

    POST(! itemsStacked_);
}

W4dRoot& W4dSceneManager::root() const
{
    PRE(pImpl_->root_ != nullptr);
    return *(pImpl_->root_);
}

void W4dSceneManager::clearAllLights()
{
    CB_SCENEMANAGER_DEPIMPL;

    while (lights_.size() != 0)
        removeLight(lights_.back());
}

bool W4dSceneManager::shakeCamera() const
{
    return pImpl_->pCameraShake_ != nullptr;
}

W4dCameraShake& W4dSceneManager::cameraShakeForEdit()
{
    CB_SCENEMANAGER_DEPIMPL;

    if (pCameraShake_ == nullptr)
        pCameraShake_ = new W4dCameraShake();

    return *pCameraShake_;
}

void W4dSceneManager::shakeCurrentCamera()
{
    CB_SCENEMANAGER_DEPIMPL;

    MexLine3d cameraToEpicCentre(currentCamera_->globalTransform().position(), pCameraShake_->epicCentre());
    MATHEX_SCALAR length = cameraToEpicCentre.length();

    const MATHEX_SCALAR epicRadius = 200.0;
    MATHEX_SCALAR normedLength = length / epicRadius;
    MATHEX_SCALAR damp = 1.0 - 2.0 * normedLength + normedLength * normedLength;
    if (length > epicRadius)
        damp = 0.0;

    MexTransform3d unshakenTransform = currentCamera_->localTransform();

    MexTransform3d shakeTransform;
    pCameraShake_->offsetTransform(&shakeTransform);

    MexEulerAngles angles;
    shakeTransform.rotation(&angles);

    MexRadians azi = angles.azimuth();
    MexRadians ele = angles.elevation();
    MexRadians rol = angles.roll();

    angles.azimuth(damp * azi);
    angles.elevation(damp * ele);
    angles.roll(damp * rol);

    unshakenTransform.rotate(angles);

    currentCamera_->localTransform(unshakenTransform);
}

void W4dSceneManager::cancelCameraShake()
{
    CB_SCENEMANAGER_DEPIMPL;

    delete pCameraShake_;
    pCameraShake_ = nullptr;

    POST(!shakeCamera());
}

void W4dSceneManager::domainAssignor(W4dDomainAssignor* d)
{
    pImpl_->domainAssignor_ = d;
}

W4dDomainAssignor* W4dSceneManager::domainAssignor() const
{
    return pImpl_->domainAssignor_;
}

bool W4dSceneManager::dynamicLightsEnabled() const
{
    return pImpl_->dynamicLightsEnabled_;
}

void W4dSceneManager::dynamicLightsEnabled(bool enabled)
{
    pImpl_->dynamicLightsEnabled_ = enabled;
}

MexPoint3d W4dSceneManager::shadowFrustumCenter() const
{
    CB_DEPIMPL_AUTO(currentCamera_);

    const MexTransform3d& camXform = currentCamera_->globalTransform();
    const MexPoint3d camPos = camXform.position();
    const MexVec3 camFwd = camXform.xBasis();
    const MexVec3 camUpBasis = camXform.yBasis();
    const MexVec3 camRightBasis = camXform.zBasis();

    const glm::vec3 eye(camPos.x(), camPos.y(), camPos.z());
    const glm::vec3 fwd(camFwd.x(), camFwd.y(), camFwd.z());
    const glm::vec3 camUp(camUpBasis.x(), camUpBasis.y(), camUpBasis.z());
    const glm::vec3 camRight(camRightBasis.x(), camRightBasis.y(), camRightBasis.z());

    const double verticalFov = currentCamera_->verticalFOVAngle();
    const double aspect = currentCamera_->horizontalFOVAngle() / verticalFov;
    const float tanHalfVFov = static_cast<float>(std::tan(verticalFov * 0.5));
    const float tanHalfHFov = tanHalfVFov * static_cast<float>(aspect);

    const float camZ = static_cast<float>(camPos.z());
    const float maxGroundDist = camZ + 200.0f;

    // Sample: center, right edge, top edge, bottom edge.
    const std::array<glm::vec2, 4> sampleOffsets{{
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {0.0f, 1.0f},
        {0.0f, -1.0f},
    }};

    float minGroundDist = std::numeric_limits<float>::max();
    glm::vec3 groundHit = eye;
    for (const auto& uv : sampleOffsets)
    {
        const glm::vec3 rayDir = glm::normalize(
            fwd + camRight * (uv.x * tanHalfHFov) + camUp * (uv.y * tanHalfVFov));

        if (rayDir.z >= -0.01f)
            continue;

        const float tHit = -eye.z / rayDir.z;
        if (tHit <= 0.0f)
            continue;

        const float tClamped = std::min(tHit, maxGroundDist);
        const glm::vec3 hit = eye + rayDir * tClamped;
        const float dist = glm::length(hit - eye);
        if (dist < minGroundDist)
        {
            minGroundDist = dist;
            groundHit = hit;
        }
    }

    // Fallback: if no frustum ray hits the ground (looking straight up),
    // place the center slightly ahead on the ground.
    glm::vec3 center;
    if (minGroundDist < std::numeric_limits<float>::max())
    {
        center = groundHit;
    }
    else
    {
        const glm::vec3 fwdHoriz = glm::normalize(glm::vec3(fwd.x, fwd.y, 0.0f));
        center = glm::vec3(eye.x, eye.y, 0.0f) + fwdHoriz * std::min(30.0f, maxGroundDist);
    }

    return MexPoint3d(center.x, center.y, center.z);
}

/* End SCENEMGR.CPP *************************************************/
