/*
 * E N T Y I M P L . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "ctl/List.hpp"

#include "mathex/Line3d.hpp"
#include "mathex/Vec2.hpp"

#include "render/MeshInstance.hpp"
#include "render/Mesh.hpp"
#include "render/Scale.hpp"
#include "render/Colour.hpp"
#include "render/Device.hpp"

#include "world4d/Entity/Root.hpp"
#include "world4d/Scene/Domain.hpp"
#include "world4d/Subject/Subject.hpp"
#include "world4d/Scene/Camera.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Entity/EntityScale.hpp"
#include "world4d/Scene/LocalLightList.hpp"
#include "world4d/Entity/Composite.hpp"
#include "world4d/Entity/Link.hpp"
#include "world4d/Scene/GarbageCollector.hpp"
#include "world4d/Materials/Internal/AnimationData.hpp"
#include "world4d/Materials/Internal/CycleTextureData.hpp"
#include "world4d/Materials/Internal/MultiTexture.hpp"
#include "world4d/Materials/Internal/UVData.hpp"
#include "world4d/Plans/ColourPulseData.hpp"
#include "world4d/Materials/Internal/MultiColour.hpp"
#include "world4d/Scene/Internal/LightData.hpp"
#include "world4d/Sound/SoundManager.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Entity/Entity.hpp"
#include "world4d/Scene/Internal/NoLight.hpp"
#include "world4d/Entity/Internal/EntityImpl.hpp"

#ifndef _INLINE
#include "world4d/Entity/Internal/EntityImpl.ipp"
#endif

W4dEntityImpl::W4dEntityImpl(W4dEntity* pParent, const W4dTransform3d& newLocalTransform, W4dEntity::Solidity solid)
    : localTransform_(newLocalTransform)
    , pParent_(pParent)
    , meshes_(new Meshes)
    , timeGlobalTransformLastUpdated_(-1.0)
    , solid_(solid)
    , ownedByParent_(true)
    , visible_(true)
    , isStationary_(true)
    , notScaled_(true)
{
    PRE(pParent != nullptr);
    LOG_CONSTRUCTION;

    meshes_->reserve(4);

    TEST_INVARIANT;
}

W4dEntityImpl::W4dEntityImpl(const W4dEntityImpl& copyMe, W4dEntity* pParent, const W4dTransform3d& newLocalTransform)
    : localTransform_(newLocalTransform)
    , pParent_(pParent)
    , meshes_(new Meshes)
    , passId_(copyMe.passId_)
    , timeGlobalTransformLastUpdated_(-1.0)
    , solid_(copyMe.solid_)
    , isDomain_(copyMe.isDomain_)
    , ownedByParent_(copyMe.ownedByParent_)
    , visible_(copyMe.visible_)
    , isStationary_(copyMe.isStationary_)
    , notScaled_(true)
    , isComposite_(copyMe.isComposite_)
    , name_(copyMe.name_)
    , doNotLight_(copyMe.doNotLight_)
    , clientData_(copyMe.clientData_)
{
    if (copyMe.pFilterColour_ != nullptr)
        pFilterColour_ = new RenColour(*copyMe.pFilterColour_);

    if (copyMe.pAnimationDataPtrs_ != nullptr)
        pAnimationDataPtrs_ = new AnimationDataPtrs(*copyMe.pAnimationDataPtrs_);

    if (copyMe.pAnimationLightDataPtrs_ != nullptr)
        pAnimationLightDataPtrs_ = new AnimationDataPtrs(*copyMe.pAnimationLightDataPtrs_);

    TEST_INVARIANT;
}

W4dEntityImpl::W4dEntityImpl()
    : meshes_(new Meshes)
    , solid_(W4dEntity::NOT_SOLID)
    , timeGlobalTransformLastUpdated_(-1.0)
    , ownedByParent_(true)
    , visible_(true)
    , isStationary_(true)
    , notScaled_(true)
{
    TEST_INVARIANT;
}

W4dEntityImpl::W4dEntityImpl(
    W4dEntity* pParent,
    const W4dTransform3d& newLocalTransform,
    W4dEntity::Solidity solid,
    W4dEntity::NotAChild)
    : pParent_(pParent)
    , meshes_(new Meshes)
    , solid_(solid)
    , timeGlobalTransformLastUpdated_(-1.0)
    , ownedByParent_(false)
    , // NB: different
    visible_(true)
    , isStationary_(true)
    , notScaled_(true)
    , localTransform_(newLocalTransform)
{
    TEST_INVARIANT;
}

W4dEntityImpl::~W4dEntityImpl()
{
    ASSERT_INFO(nLinkedSounds_);
    ASSERT(nLinkedSounds_ == 0, "W4dEntityImpl dtor still has non-zero sound count");

    delete pEntityScale_;
    delete pPlan_;
    delete pPlanUpdateTimes_;

    delete childList_;
    delete intersectingDomains_;
    delete boundingVolume_;

    delete pLocalLightList_;
    delete pFilterColour_;
    delete pAnimationDataPtrs_;
    delete pAnimationLightDataPtrs_;

    for (Meshes::iterator i = meshes_->begin(); i != meshes_->end(); ++i)
    {
        delete (*i).mesh;
    }
    delete meshes_;
}

void W4dEntityImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

void W4dEntityImpl::name(const std::string& name)
{
    name_ = name;
}

const std::string& W4dEntityImpl::name() const
{
    return name_;
}

void W4dEntityImpl::updateHasMeshFlag()
{
    hasMesh_ = meshes_->size() > 0 && (*meshes_)[0].mesh;
}

const W4dEntity::W4dEntities& W4dEntityImpl::children() const
{
    // This is used to export an empty child list for entities with no child list allocated
    static W4dEntity::W4dEntities dummyChildList;

    return (childList_ == nullptr ? dummyChildList : *childList_);
}

void W4dEntityImpl::addAnimationData(W4dAnimationData* pAnimData)
{
    if (pAnimationDataPtrs_ == nullptr)
    {
        pAnimationDataPtrs_ = new AnimationDataPtrs;
        pAnimationDataPtrs_->reserve(2);
    }

    pAnimationDataPtrs_->push_back(AnimationDataPtr(pAnimData));
}

void W4dEntityImpl::addLightAnimationData(W4dLightData* pAnimData)
{
    if (pAnimationLightDataPtrs_ == nullptr)
    {
        pAnimationLightDataPtrs_ = new AnimationDataPtrs;
        pAnimationLightDataPtrs_->reserve(2);
    }

    pAnimationLightDataPtrs_->push_back(AnimationDataPtr(pAnimData));
}

void W4dEntityImpl::applyLightAnimations(W4dEntity* pEntity, const PhysAbsoluteTime& startTime) const
{
    if (pAnimationLightDataPtrs_ != nullptr)
    {
        size_t nAnimations = pAnimationLightDataPtrs_->size();
        for (size_t i = 0; i < nAnimations; ++i)
        {
            (*pAnimationLightDataPtrs_)[i]->apply(pEntity, startTime);
        }
    }
}

void W4dEntityImpl::applyNonLightAnimations(W4dEntity* pEntity, const PhysAbsoluteTime& startTime) const
{
    PRE(pEntity->hasMesh());

    if (pAnimationDataPtrs_ != nullptr)
    {
        size_t nAnimations = pAnimationDataPtrs_->size();
        for (size_t i = 0; i < nAnimations; ++i)
        {
            (*pAnimationDataPtrs_)[i]->apply(pEntity, startTime);
        }
    }
}

void W4dEntityImpl::applyAllAnimations(W4dEntity* pEntity, const PhysAbsoluteTime& startTime) const
{
    applyLightAnimations(pEntity, startTime);
    applyNonLightAnimations(pEntity, startTime);
}

PER_DEFINE_PERSISTENT(W4dEntityImpl);

void perWrite(PerOstream& ostr, const W4dEntityImpl& t)
{
    ostr << t.localTransform_;
    ostr << t.globalTransform_;
    ostr << t.pParent_;
    ostr << t.childList_;
    ostr << t.meshes_;

    ostr << t.intersectingDomains_;
    ostr << t.pHolder_;
    ostr << t.pEntityScale_;
    ostr << t.pPlan_;
    ostr << t.pPlanUpdateTimes_;
    ostr << t.pSubject_;
    ostr << t.passId_;
    ostr << t.checkId_;
    ostr << t.id_;
    ostr << t.boundingVolume_;

    int iTemp = (t.solid_ == W4dEntity::SOLID) ? 1 : 0;
    PER_WRITE_RAW_OBJECT(ostr, iTemp);

    bool bTemp;
    bTemp = t.isDomain_;
    PER_WRITE_RAW_OBJECT(ostr, bTemp);
    bTemp = t.ownedByParent_;
    PER_WRITE_RAW_OBJECT(ostr, bTemp);
    bTemp = t.visible_;
    PER_WRITE_RAW_OBJECT(ostr, bTemp);
    bTemp = t.isStationary_;
    PER_WRITE_RAW_OBJECT(ostr, bTemp);
    bTemp = t.notScaled_;
    PER_WRITE_RAW_OBJECT(ostr, bTemp);
    bTemp = t.isComposite_;
    PER_WRITE_RAW_OBJECT(ostr, bTemp);
    bTemp = t.isGarbage_;
    PER_WRITE_RAW_OBJECT(ostr, bTemp);
    bTemp = t.hasMesh_;
    PER_WRITE_RAW_OBJECT(ostr, bTemp);

    ostr << t.pLocalLightList_;

    writeAllocatedStringFromPointer(ostr, &t.name_);
    ostr << t.pAnimationDataPtrs_;
    ostr << t.pAnimationLightDataPtrs_;
    ostr << t.pFilterColour_;
    ostr << t.doNotLight_;
    ostr << t.clientData_;
}

void perRead(PerIstream& istr, W4dEntityImpl& t)
{
    istr >> t.localTransform_;

    istr >> t.globalTransform_;
    istr >> t.pParent_;
    istr >> t.childList_;
    istr >> t.meshes_;

    istr >> t.intersectingDomains_;
    istr >> t.pHolder_;
    istr >> t.pEntityScale_;
    istr >> t.pPlan_;
    istr >> t.pPlanUpdateTimes_;
    istr >> t.pSubject_;
    istr >> t.passId_;
    istr >> t.checkId_;
    istr >> t.id_;
    istr >> t.boundingVolume_;

    int iTemp;
    PER_READ_RAW_OBJECT(istr, iTemp);
    t.solid_ = (iTemp == 1) ? W4dEntity::SOLID : W4dEntity::NOT_SOLID;

    bool bTemp;
    PER_READ_RAW_OBJECT(istr, bTemp);
    t.isDomain_ = bTemp;
    PER_READ_RAW_OBJECT(istr, bTemp);
    t.ownedByParent_ = bTemp;
    PER_READ_RAW_OBJECT(istr, bTemp);
    t.visible_ = bTemp;
    PER_READ_RAW_OBJECT(istr, bTemp);
    t.isStationary_ = bTemp;
    PER_READ_RAW_OBJECT(istr, bTemp);
    t.notScaled_ = bTemp;
    PER_READ_RAW_OBJECT(istr, bTemp);
    t.isComposite_ = bTemp;
    PER_READ_RAW_OBJECT(istr, bTemp);
    t.isGarbage_ = bTemp;
    PER_READ_RAW_OBJECT(istr, bTemp);
    t.hasMesh_ = bTemp;

    istr >> t.pLocalLightList_;

    //  This call must be made here to make sure that the entity plan
    //  update times are properly set up

    // t.name used to be a heap-allocated string. Use special read function
    // to load PerDataType::PER_OBJECT_POINTER into a stack-allocated object
    readAllocatedStringFromPointer(istr, &t.name_);

    istr >> t.pAnimationDataPtrs_;
    istr >> t.pAnimationLightDataPtrs_;
    istr >> t.pFilterColour_;
    istr >> t.doNotLight_;
    istr >> t.clientData_;
}

void perWrite(PerOstream& ostr, const W4dEntityImpl::MeshData& data)
{
    ostr << data.mesh;
    ostr << data.distance; // This is distance for mesh details
}

void perRead(PerIstream& istr, W4dEntityImpl::MeshData& data)
{
    istr >> data.mesh;
    istr >> data.distance;

    // char val;
    // PER_READ_RAW_OBJECT( istr, val );

    // PER_READ_RAW_OBJECT( istr, val );
    // PER_READ_RAW_OBJECT( istr, val );
}

void perWrite(PerOstream& ostr, const W4dEntityImpl::PlanUpdateTimes& times)
{
    ostr << times.transformTime;
    ostr << times.visibilityTime;
    ostr << times.scaleTime;
    ostr << times.uvTime;
    ostr << times.materialTime;
}

void perRead(PerIstream& istr, W4dEntityImpl::PlanUpdateTimes& times)
{
    istr >> times.transformTime;
    istr >> times.visibilityTime;
    istr >> times.scaleTime;
    istr >> times.uvTime;
    istr >> times.materialTime;
}

void W4dEntityImpl::filterColour(const RenColour& colour)
{
    if (! filterColourIsSet())
        pFilterColour_ = new RenColour(colour);
    else
        *pFilterColour_ = colour;

    POST(filterColourIsSet());
}

const RenColour& W4dEntityImpl::filterColour() const
{
    PRE(filterColourIsSet());
    return *pFilterColour_;
}

bool W4dEntityImpl::filterColourIsSet() const
{
    return pFilterColour_ != nullptr;
}

// static
const std::string& W4dEntityImpl::dummyName()
{
    static std::string emptyString;
    return emptyString;
}

W4dLocalLightList& W4dEntityImpl::lightListForEdit()
{
    if (pLocalLightList_ == nullptr)
        pLocalLightList_ = new W4dLocalLightList();

    return *pLocalLightList_;
}

W4dEntityImpl::W4dEntityImpl(PerConstructor)
    : hasMesh_(true)
    , solid_(W4dEntity::NOT_SOLID)
    , timeGlobalTransformLastUpdated_(-1.0)
    , ownedByParent_(true)
    , visible_(true)
    , isStationary_(true)
    , notScaled_(true)
    , doNotLight_(false)
{
    TEST_INVARIANT;
}

/* End ENTYIMPL.CPP *************************************************/
