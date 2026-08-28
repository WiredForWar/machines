/*
 * C A M E R A . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "ctl/List.hpp"
#include "world4d/Scene/Camera.hpp"

#include "render/Camera.hpp"
#include "render/Device.hpp"

#include <algorithm>
#include <queue>
#include <unordered_set>

#include "world4d/Scene/Domain.hpp"
#include "world4d/Scene/Portal.hpp"
#include "world4d/Entity/Root.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Scene/LocalLightList.hpp"
#include "world4d/Scene/CameraVolume.hpp"
#include "world4d/Scene/ShadowVolume.hpp"
#include "world4d/Scene/Environment.hpp"
#include "world4d/Internal/Complexity.hpp"
#include "world4d/Entity/Internal/EntityImpl.hpp"

// For debugging the culling.  This generates loads of output, so it's not
// normally compiled into the code.
// #define DO_CULL_TRACING
#ifdef DO_CULL_TRACING
#define CULL_TRACE(x) x
#define CULL_STREAM(x) IAIN_STREAM(x)
#define CULL_INDENT(x) IAIN_INDENT(x)
#else
#define CULL_TRACE(x) ;
#define CULL_STREAM(x) ;
#define CULL_INDENT(x) ;
#endif

PER_DEFINE_PERSISTENT(W4dCamera);

W4dCamera::W4dCamera(W4dSceneManager* mgr, W4dEntity* pParent, const W4dTransform3d& localXform)
    : W4dEntity(pParent, localXform, NOT_SOLID)
    , camera_(new RenCamera)
    , manager_(mgr)
    , pVolume_(new W4dCameraVolume(*this))
    , lastPassId_(0)
    , passId_(0)
    , savedYonClipDistance_(500)
{
    PRE(mgr);
    PRE(pParent);
    ASSERT(camera_, runtime_error("Out of memory."));
}

W4dCamera::~W4dCamera()
{
    delete camera_;
    delete pVolume_;
}

// Render the tree of W4dEntities that this camera is attached to,
// starting from the root of the tree.
void W4dCamera::inOrderRender()
{
    W4dRoot* root = findRoot();
    ASSERT(!root->hasParent(), logic_error());

    // Update the camera clipping volume
    pVolume_->update();

    passId_ = W4dManager::instance().generateRenderPassId();

    renderTree(root, IN_ORDER_RENDER);
}

// Render the given node and any tree below it.
void W4dCamera::renderTree(W4dEntity* node, TraversalType traversalType)
{
    PRE(node);
    CULL_STREAM("renderTree for " << node << "\n");
    CULL_INDENT(2);

    RenDevice* device = RenDevice::current();
    W4dEntityImpl& nodeImpl = node->entityImpl();

    // Skip static shadow meshes when real-time shadow mapping is active.
    if (nodeImpl.isShadow() && device->isShadowMappingEnabled())
        return;

    if ((nodeImpl.doNotLight() || nodeImpl.noShadowCast()) && device->isShadowPassActive())
        return;

    // Has the node been rendered by the given camera already on the current
    // render pass?  If so, don't repeatedly render this entity.
    if (nodeImpl.passId() == renderPassId())
    {
        CULL_STREAM("Processed already according to PassID\n");
    }
    else
    {
        // Mark this entity as rendered during this pass.
        nodeImpl.passId(renderPassId());
        CULL_TRACE(++entitiesRendered_);

        // Check it is visible
        if (node->visible())
        {
            // If this object is flagged as totally unlit, disable the render
            // lighting.  This logic should ensure that we don't disable the
            // lighting and, more particularly, re-enable it if this node is a
            // child of a node which already disabled the lighting.
            bool lightingDisabledHere = false;
            if (nodeImpl.doNotLight() && device->lightingEnabled())
            {
                CULL_STREAM("Disabled lighting for " << node << "\n");
                device->disableLighting();
                lightingDisabledHere = true;
            }
            else if (nodeImpl.hasLightList())
            {
                // If we have a light list turn it on
                CULL_STREAM("lights on for " << node << "\n");
                nodeImpl.lightListForEdit().turnOnAll();
            }

            W4dEntity::RenderType renderType = RENDER_CHILDREN;

            // TBD: make this robust, it's a *REAL HACK*.  What we really want to
            // test is: will W4dEntity::render do anything?  With the current
            // virtual implementations of render, the test below should amount to
            // the same thing.  This isn't guaranteed.
            bool nodeIsComposite = nodeImpl.isComposite();
            if (nodeIsComposite || nodeImpl.hasMesh())
            {
                // Check to see if the bounding volume of the entity intersects
                // whatever this pass is culling against.
                if (isInCullVolume(*node))
                {
                    // There's no need to call this method if the lighting system is
                    // turned off.  We don't check the doNotLight flag because the
                    // lighting could have been turned off by a parent of the node.
                    if (device->lightingEnabled())
                        manager_->cullDynamicLights(node);

                    renderType = node->render(*this, manager_->complexity());
                }
            }

            // render the node's children if required
            if (renderType == W4dEntity::RENDER_CHILDREN)
            {
                const W4dEntity::W4dEntities& kids = nodeImpl.children();
                if (nodeIsComposite)
                {
                    // render the composite children based without recomputing the intersection relationships
                    for (W4dEntity::W4dEntities::const_iterator it = kids.begin(); it != kids.end(); ++it)
                    {
                        if (traversalType == IN_ORDER_RENDER || (!(*it)->isDomain()))
                            renderSubTree(*it);
                    }
                }
                else
                {
                    for (W4dEntity::W4dEntities::const_iterator it = kids.begin(); it != kids.end(); ++it)
                    {
                        if (traversalType == IN_ORDER_RENDER || (!(*it)->isDomain()))
                            renderTree(*it, traversalType);
                    }
                }
            }

            if (lightingDisabledHere)
            {
                // Re-enable the lighting system.
                ASSERT(!device->lightingEnabled(), "Expected lighting to be off.");
                device->enableLighting();
            }
            else if (nodeImpl.hasLightList())
            {
                // turn off any lights
                CULL_STREAM("lights off for " << node << "\n");
                nodeImpl.lightListForEdit().turnOffAll();
            }
        }
    }

    CULL_INDENT(-2);
}
// Render the given node and any tree below it. without recomputing the intersecting volume
// and calling cullDynamicLights
// shoud be called to render the children and the heldObjects of a composite object

void W4dCamera::renderSubTree(W4dEntity* node)
{
    PRE(node);
    CULL_STREAM("renderSubTree for " << node << "\n");
    CULL_INDENT(2);

    RenDevice* device = RenDevice::current();
    W4dEntityImpl& nodeImpl = node->entityImpl();

    // Skip static shadow meshes when real-time shadow mapping is active.
    if (nodeImpl.isShadow() && device->isShadowMappingEnabled())
        return;

    if ((nodeImpl.doNotLight() || nodeImpl.noShadowCast()) && device->isShadowPassActive())
        return;

    // Has the node been rendered by the given camera already on the current
    // render pass?  If so, don't repeatedly render this entity.
    if (nodeImpl.passId() == renderPassId())
    {
        CULL_STREAM("Processed already according to PassID\n");
    }
    else
    {
        // Mark this entity as rendered during this pass.
        nodeImpl.passId(renderPassId());
        CULL_TRACE(++entitiesRendered_);

        // Check it is visible
        if (node->visible())
        {
            // If this object is flagged as totally unlit, disable the render
            // lighting.  This logic should ensure that we don't disable the
            // lighting and, more particularly, re-enable it if this node is a
            // child of a node which already disabled the lighting.
            bool lightingDisabledHere = false;
            if (nodeImpl.doNotLight() && device->lightingEnabled())
            {
                CULL_STREAM("Disabled lighting for " << node << "\n");
                device->disableLighting();
                lightingDisabledHere = true;
            }
            else if (nodeImpl.hasLightList())
            {
                // If we have a light list turn it on
                CULL_STREAM("lights on for " << node << "\n");
                nodeImpl.lightListForEdit().turnOnAll();
            }

            // Check to see if the bounding volume of the entity intersects the camera's bv
            W4dEntity::RenderType renderType = RENDER_CHILDREN;

            // TBD: make this robust, it's a *REAL HACK*.  What we really want to
            // test is: will W4dEntity::render draw anything which needs lights?
            // With the current virtual implementations of render, the test below
            // should amount to the same thing.  This isn't guaranteed.
            if (nodeImpl.isComposite() || nodeImpl.hasMesh())
                renderType = node->render(*this, manager_->complexity());

            if (renderType == W4dEntity::RENDER_CHILDREN)
            {
                const W4dEntity::W4dEntities& kids = nodeImpl.children();
                for (W4dEntity::W4dEntities::const_iterator it = kids.begin(); it != kids.end(); ++it)
                {
                    if (!(*it)->isDomain())
                        renderSubTree(*it);
                }
            }

            if (lightingDisabledHere)
            {
                // Re-enable the lighting system.
                ASSERT(!device->lightingEnabled(), "Expected lighting to be off.");
                device->enableLighting();
            }
            else if (nodeImpl.hasLightList())
            {
                // turn off any lights
                CULL_STREAM("lights off for " << node << "\n");
                nodeImpl.lightListForEdit().turnOffAll();
            }
        }
    }

    CULL_INDENT(-2);
}

// Render the tree that this camera is attached to using domain and
// portal culling.
void W4dCamera::domainRender(const int maxDepth)
{
    PRE(maxDepth > 0);

    cullVisibleSet(maxDepth);
    renderVisibleSet();
}

void W4dCamera::cullVisibleSet(const int maxDepth)
{
    PRE(maxDepth > 0);

    visibleDomains_.erase(visibleDomains_.begin(), visibleDomains_.end());
    visibleSetIsUnculled_ = false;

    // Check the camera is in a domain
    W4dDomain* myDomain;
    if (!hasContainingDomain(&myDomain))
    {
        // Camera not in a domain, so the whole tree is drawn without culling.
        visibleSetIsUnculled_ = true;
        return;
    }

    CULL_TRACE(entitiesRendered_ = 0);
    CULL_TRACE(domainsRendered_ = 0);

    // Update the camera clipping volume
    if (lastRenderTransformKey_ != globalTransform().key())
        pVolume_->update();

    cullDomains(myDomain, maxDepth);
}

void W4dCamera::renderVisibleSet()
{
    if (visibleSetIsUnculled_)
    {
        inOrderRender();
        return;
    }

    // A fresh pass id each time, so that the per-entity "already drawn this
    // pass" marks do not stop a second drawing of the same visible set.
    passId_ = W4dManager::instance().generateRenderPassId();

    // Counted per drawing, as they were when culling and drawing were one.
    CULL_TRACE(entitiesRendered_ = 0);
    CULL_TRACE(domainsRendered_ = 0);

    for (W4dDomain* in : visibleDomains_)
    {
        CULL_TRACE(++domainsRendered_);

        // Render the domain's subtree.
        // NB: also tags the domain with this camera's render pass Id.
        renderTree(in, DOMAIN_RENDER);

        // The above renderTree call will turn on the domain's light list then
        // turn it off again.  We need the list to be on for any intersecting
        // entities or for recursive domains.
        if (in->hasLightList())
            in->lightListForEdit().turnOnAll();

        // Render any entities which (might) intersect the current domain.
        const W4dEntities& entities = in->intersectingEntities();
        for (W4dEntities::const_iterator it = entities.begin(); it != entities.end(); ++it)
        {
            if (*it)
                renderTree(*it, DOMAIN_RENDER);
        }

        if (in->hasLightList())
            in->lightListForEdit().turnOffAll();
    }

    // For test purposes only: render any non-domain children of the root.
    W4dRoot* root = findRoot();
    ASSERT(!root->hasParent(), logic_error());

    const W4dEntity::W4dEntities& kids = root->children();
    for (W4dEntity::W4dEntities::const_iterator it = kids.begin(); it != kids.end(); ++it)
    {
        W4dEntity* entity = *it;
        if (!entity->isDomain())
            renderTree(entity, DOMAIN_RENDER);
    }

    lastRenderTransformKey_ = globalTransform().key();
    lastPassId_ = passId_;
}

void W4dCamera::cullVolume(const W4dShadowVolume* volume)
{
    cullVolume_ = volume;
}

const W4dShadowVolume* W4dCamera::cullVolume() const
{
    return cullVolume_;
}

bool W4dCamera::isInCullVolume(const W4dEntity& entity) const
{
    return cullVolume_ ? cullVolume_->intersects(entity) : pVolume_->intersects(entity);
}

bool W4dCamera::isInCullVolume(const MexQuad3d& quad) const
{
    return cullVolume_ ? cullVolume_->canSee(quad) : canSee(quad);
}

void W4dCamera::cullDomains(W4dDomain* startDomain, int maxDepth)
{
    PRE(startDomain);

    // BFS traversal: process domains breadth-first so that rooms closest
    // (in graph distance) to the camera are recorded first.  This prevents
    // the depth budget from being wasted going deep into one corridor chain
    // before visiting nearby visible rooms.

    struct BfsEntry
    {
        W4dDomain* domain{};
        int depth{};
    };

    std::queue<BfsEntry> bfsQueue;
    bfsQueue.push({startDomain, 0});

    // The fused version marked domains through the render pass id, which it got
    // for free from drawing them. Culling on its own has to track this itself.
    std::unordered_set<const W4dDomain*> queued;
    queued.insert(startDomain);

    // With proper 6-plane canSee culling (behind + far + 4 sides) and BFS,
    // the frustum test is the real traversal limiter.  The budget is just a
    // safety net against degenerate portal graphs.  Use a generous cap so
    // that indoor maps with many small domains are fully rendered.
    const int safetyCap = std::max(maxDepth * 4, 512);
    int domainsProcessed = 0;

    while (!bfsQueue.empty())
    {
        const auto [in, depth] = bfsQueue.front();
        bfsQueue.pop();

        // Safety limit to prevent runaway traversal.
        if (domainsProcessed > safetyCap)
        {
            CULL_STREAM("reached safety cap (" << safetyCap << ")\n");
            continue;
        }
        ++domainsProcessed;

        CULL_STREAM("traversing into " << (W4dEntity*)in << " depth=" << depth << "\n");
        CULL_INDENT(2);

        visibleDomains_.push_back(in);

        // Enqueue neighbouring domains (BFS expansion).
        const W4dDomain::W4dPortals& portals = in->portals();
        for (W4dDomain::W4dPortals::const_iterator it = portals.begin(); it != portals.end(); ++it)
        {
            const W4dPortal* portal = *it;

            // Don't traverse through closed portals.
            if (portal && portal->isOpenFrom(in))
            {
                W4dDomain* nextDomain = portal->otherDomain(in);
                CULL_STREAM("Considering " << nextDomain->name() << ": ");

                // Don't traverse if we have already reached the domain.
                if (queued.find(nextDomain) == queued.end())
                {
                    CULL_STREAM("through " << portal->globalAperture() << " ");
                    if (isInCullVolume(portal->globalAperture()))
                    {
                        CULL_STREAM("visible - enqueued\n");
                        queued.insert(nextDomain);
                        bfsQueue.push({nextDomain, depth + 1});
                    }
                    else
                    {
                        CULL_STREAM("not visible\n");
                    }
                }
                else
                {
                    CULL_STREAM("reached already\n");
                }
            }
        }

        CULL_STREAM("leaving " << (W4dEntity*)in << " depth=" << depth << "\n\n");
        CULL_INDENT(-2);
    }
}

void W4dCamera::adaptToEnvironment(W4dEnvironment* env)
{
    MATHEX_SCALAR newYonClipDistance;
    if (env && env->fogOn())
    {
        newYonClipDistance = 1.5 * env->fogVisibility();
        if (newYonClipDistance > savedYonClipDistance())
            newYonClipDistance = savedYonClipDistance();
    }
    else
    {
        // reset to default
        newYonClipDistance = savedYonClipDistance();
    }
    // use RenCamera::yonClipCamera instead of its local implementation
    // the saved value of the clip distance is not overriden
    camera_->yonClipDistance(newYonClipDistance);
}

void W4dCamera::savedYonClipDistance(double dist)
{
    savedYonClipDistance_ = dist;
}

double W4dCamera::savedYonClipDistance() const
{
    return savedYonClipDistance_;
}

void W4dCamera::hitherClipDistance(double dist)
{
    PRE(dist > 0);

    camera_->hitherClipDistance(dist);
}

double W4dCamera::hitherClipDistance() const
{
    return camera_->hitherClipDistance();
}

void W4dCamera::yonClipDistance(double dist)
{
    PRE(dist > 0);

    // save locally the clip distance to make sure that adaptToEnvironment() has sthg
    // to fall on in case the fog distance goes crazy
    savedYonClipDistance(dist);
    camera_->yonClipDistance(dist);
}

double W4dCamera::yonClipDistance() const
{
    return camera_->yonClipDistance();
}

// Only the vertical angle can be set.  The horizontal angle can be read.
void W4dCamera::setVerticalFOVAngle(double angle)
{
    PRE(angle > 0);
    camera_->setVerticalFOVAngle(angle);
}

double W4dCamera::verticalFOVAngle() const
{
    return camera_->verticalFOVAngle();
}

// The camera will be set-up such that the horizontal angle will
// be w/h*FOVAngle(), where w and h are the dimensions of the
// viewport/window.
double W4dCamera::horizontalFOVAngle() const
{
    // Get the aspect ratio of the viewport via the scenemanager's device
    RenDevice* pDevice = manager_->pDevice();
    double ratio = double(pDevice->windowWidth()) / double(pDevice->windowHeight());

    return camera_->horizontalFOVAngle(ratio);
}

bool W4dCamera::canSee(const MexPoint3d& pt) const
{
    return manager_->pDevice()->canSee(pt);
}

bool W4dCamera::canSee(const MexQuad3d& quad) const
{
    return pVolume_->canSee(quad);
}

uint W4dCamera::entitiesRendered() const
{
    return entitiesRendered_;
}

uint W4dCamera::domainsRendered() const
{
    return domainsRendered_;
}

void W4dCamera::updateRenCamera()
{
    camera_->transform(globalTransform());
}

W4dSceneManager* W4dCamera::manager() const
{
    return manager_;
}

RenCamera* W4dCamera::pRenCamera() const
{
    return camera_;
}

const MexTransform3dKey& W4dCamera::lastRenderTransformKey() const
{
    return lastRenderTransformKey_;
}

// virtual
void W4dCamera::doDebugOutput(std::ostream& ostr)
// Write out the details of 'this' for debugging purposes.
{
    ostr << "Start W4dCamera " << static_cast<const void*>(this) << std::endl;
    ostr << "End   W4dCamera " << static_cast<const void*>(this) << std::endl;
}

// virtual
bool W4dCamera::intersectsLine(const MexLine3d&, MATHEX_SCALAR*, Accuracy) const
{
    return false;
}

void perWrite(PerOstream& ostr, const W4dCamera& camera)
{
    const W4dEntity& base = camera;

    ostr << base;
}

void perRead(PerIstream& istr, W4dCamera& camera)
{
    W4dEntity& base = camera;

    istr >> base;
}

W4dCamera::W4dCamera(PerConstructor con)
    : W4dEntity(con)
    , pVolume_(new W4dCameraVolume(*this))
    , lastPassId_(0)
{
}

void W4dCamera::isYonClippingEnabled(bool isIt)
{
    pVolume_->isYonClippingEnabled(isIt);
}

bool W4dCamera::isYonClippingEnabled() const
{
    return pVolume_->isYonClippingEnabled();
}
