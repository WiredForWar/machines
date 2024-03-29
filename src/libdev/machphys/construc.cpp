/*
 * C O N S T R U C . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/ofactory.hpp"
#include "machphys/subid.hpp"

#include "machphys/construc.hpp"

// Include this prior to mexpdata.hpp or you'll go to the bad place
#include "stdlib/string.hpp"
#include "machphys/mexpdata.hpp"

#include "sim/manager.hpp"

#include "machphys/mphydata.hpp"
#include "machphys/ctordata.hpp"
#include "machphys/levels.hpp"
#include "machphys/light.hpp"
#include "machphys/constron.hpp"
#include "machphys/sparks.hpp"
#include "machphys/jetringp.hpp"
#include "machphys/jetringm.hpp"
#include "machphys/orientpl.hpp"

#include "mathex/transf3d.hpp"
#include "mathex/line3d.hpp"
#include "mathex/abox3d.hpp"

#include "render/colour.hpp"

#include "world4d/light.hpp"
#include "world4d/link.hpp"
#include "world4d/compplan.hpp"
#include "world4d/visplan.hpp"
#include "world4d/entyplan.hpp"
#include "world4d/composit.hpp"

#include "system/pathname.hpp"

PER_DEFINE_PERSISTENT(MachPhysConstructor);

MachPhysConstructor::MachPhysConstructor(
    W4dEntity* pParent,
    const W4dTransform3d& localTransform,
    MachPhys::ConstructorSubType subType,
    size_t bodyLevel,
    size_t brainLevel,
    MachPhys::Race race)
    : MachPhysMachine(
        part(subType, bodyLevel),
        pParent,
        localTransform,
        bodyLevel,
        brainLevel,
        race,
        MachPhysData::instance().constructorData(subType, bodyLevel, brainLevel))
    , subType_(subType)
    , isConstructing_(false)
    , pLJet_(nullptr)
    , pRJet_(nullptr)
    , pLJetRing_(nullptr)
    , pRJetRing_(nullptr)
    , pLSparks_(nullptr)
    , pRSparks_(nullptr)
    , pLLight_(nullptr)
    , pRLight_(nullptr)
    , pLLightShadow_(nullptr)
    , pRLightShadow_(nullptr)
{
    if (part(subType, bodyLevel).pLJet_ != nullptr)
    {
        pLJet_ = links()[part(subType, bodyLevel).pLJet_->id()];
        pLJet_->visible(false);
    }

    if (part(subType, bodyLevel).pRJet_ != nullptr)
    {
        pRJet_ = links()[part(subType, bodyLevel).pRJet_->id()];
        pRJet_->visible(false);
    }

    TEST_INVARIANT;
}

//  This is the constructor that is used by the factory. It is the
//  only constructor that actually builds a constructor from scratch

MachPhysConstructor::MachPhysConstructor(W4dEntity* pParent, Id id)
    : MachPhysMachine(
        pParent,
        W4dTransform3d(),
        compositeFileName(id.subType_, id.level_),
        MachPhysData::instance().constructorData(id.subType_, id.level_, 1))
    , subType_(id.subType_)
    , isConstructing_(false)
    , pLJet_(nullptr)
    , pRJet_(nullptr)
    , pLJetRing_(nullptr)
    , pRJetRing_(nullptr)
    , pLSparks_(nullptr)
    , pRSparks_(nullptr)
    , pLLight_(nullptr)
    , pRLight_(nullptr)
    , pLLightShadow_(nullptr)
    , pRLightShadow_(nullptr)
{
    if (!findLink("l_jet", &pLJet_))
        pLJet_ = nullptr;

    if (!findLink("r_jet", &pRJet_))
        pRJet_ = nullptr;

    createExplosionData();
    TEST_INVARIANT;
}

MachPhysConstructor::MachPhysConstructor(PerConstructor con)
    : MachPhysMachine(con)
    , pLJet_(nullptr)
    , pRJet_(nullptr)
    , pLJetRing_(nullptr)
    , pRJetRing_(nullptr)
    , pLSparks_(nullptr)
    , pRSparks_(nullptr)
    , pLLight_(nullptr)
    , pRLight_(nullptr)
    , pLLightShadow_(nullptr)
    , pRLightShadow_(nullptr)
{
}

MachPhysConstructor::~MachPhysConstructor()
{

    TEST_INVARIANT;
}

MachPhys::ConstructorSubType MachPhysConstructor::subType() const
{
    return subType_;
}

// virtual
const MachPhysMachineData& MachPhysConstructor::machineData() const
{
    return data();
}

const MachPhysConstructorData& MachPhysConstructor::data() const
{
    return MachPhysData::instance().constructorData(subType_, bodyLevel(), brainLevel());
}

SysPathName MachPhysConstructor::compositeFileName(MachPhys::ConstructorSubType subType, size_t bodyLevel) const
{
    SysPathName result;

    switch (subType)
    {
        case MachPhys::DOZER:
            {
                switch (bodyLevel)
                {
                    case 1:
                        result = "models/construc/dozer/level1/cod1.cdf";
                        break;

                    default:
                        ASSERT_BAD_CASE_INFO(bodyLevel);
                        break;
                }
            }
            break;

        case MachPhys::BUILDER:

            switch (bodyLevel)
            {
                case 3:
                    result = "models/construc/builder/level3/cob3.cdf";
                    break;

                default:
                    ASSERT_BAD_CASE_INFO(bodyLevel);
                    break;
            }

            break;

        case MachPhys::BEHEMOTH:

            switch (bodyLevel)
            {
                case 4:
                    result = "models/construc/behemoth/level4/como4.cdf";
                    break;

                case 5:
                    result = "models/construc/behemoth/level5/com5.cdf";
                    break;

                default:
                    ASSERT_BAD_CASE_INFO(bodyLevel);
                    break;
            }

            break;

        default:
            ASSERT_BAD_CASE_INFO(subType);
    }

    return result;
}

// static
MachPhysConstructor& MachPhysConstructor::part(MachPhys::ConstructorSubType subType, size_t hardwareLevel)
{
    return factory().part(
        Id(subType, hardwareLevel),
        MachPhysLevels::instance().uniqueHardwareIndex(subType, hardwareLevel));
}

// static
MachPhysConstructor::Factory& MachPhysConstructor::factory()
{
    static Factory factory_(MachPhysLevels::instance().nHardwareIndices(MachPhys::CONSTRUCTOR));

    return factory_;
}

void MachPhysConstructor::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysConstructor& t)
{

    o << "MachPhysConstructor " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysConstructor " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}
//////////////////////////////////////////////////////////////////////////////////////////

PhysRelativeTime MachPhysConstructor::construct()
{
    PhysRelativeTime interval;
    static int animation = 0;

    W4dCompositePlanPtr thePlan;
    if (! cycleAnims(&thePlan))
        interval = 1;
    else
    {
        const uint nTimes = 4;
        if (animation)
        {
            cycleAnims(&thePlan);
            --animation;
        }
        else
            ++animation;

        plan(*thePlan, SimManager::instance().currentTime(), nTimes - 1);
        interval = nTimes * thePlan->finishTime();
    }

    return interval;
}

void MachPhysConstructor::createExplosionData()
{

    switch (subType())
    {
        case MachPhys::DOZER:
            break;
        case MachPhys::BUILDER:
            break;
        case MachPhys::BEHEMOTH:
            {
                MachPhysMachineExplosionData& expData = explosionDataForEdit();
                // just update the position of the explosion center
                MexPoint3d explosionCenter(expData.explosionCenter());
                explosionCenter.x(explosionCenter.x() - 1.0);
                explosionCenter.z(explosionCenter.z() + 0.7);
                expData.explosionCenter(explosionCenter);
                break;
            }
        default:
            ASSERT_BAD_CASE_INFO(subType());
    }
}

void perWrite(PerOstream& ostr, const MachPhysConstructor& machine)
{
    const MachPhysMachine& base = machine;
    ostr << base;

    ostr << machine.subType_;
    ostr << machine.isConstructing_;

    ostr << machine.pLJet_;
    ostr << machine.pRJet_;
    ostr << machine.pLJetRing_;
    ostr << machine.pRJetRing_;
    ostr << machine.pLSparks_;
    ostr << machine.pRSparks_;
    ostr << machine.pLLight_;
    ostr << machine.pRLight_;
    ostr << machine.pLLightShadow_;
    ostr << machine.pRLightShadow_;
}

void perRead(PerIstream& istr, MachPhysConstructor& machine)
{
    MachPhysMachine& base = machine;
    istr >> base;

    istr >> machine.subType_;
    istr >> machine.isConstructing_;

    istr >> machine.pLJet_;
    istr >> machine.pRJet_;
    istr >> machine.pLJetRing_;
    istr >> machine.pRJetRing_;
    istr >> machine.pLSparks_;
    istr >> machine.pRSparks_;
    istr >> machine.pLLight_;
    istr >> machine.pRLight_;
    istr >> machine.pLLightShadow_;
    istr >> machine.pRLightShadow_;
}

void MachPhysConstructor::startConstructing(const MachPhysConstruction& construction, const PhysAbsoluteTime& startTime)
{
    if (! isConstructing())
    {
        W4dCompositePlanPtr constructingPlanPtr;
        MexPoint3d targetOffsetGlobal = construction.compositeBoundingVolume().centroid();
        construction.globalTransform().transform(&targetOffsetGlobal);

        if (cycleAnims(&constructingPlanPtr))
            plan(*constructingPlanPtr, startTime, 1000000, MachPhys::CONSTRUCTION_CONSTRUCTING);

        if (pLJet_ != nullptr)
        {
            // lJetRing
            if (pLJetRing_ == nullptr)
                pLJetRing_ = new MachPhysJetRing(pLJet_, MexTransform3d());

            hold(pLJetRing_, pLJet_, MexTransform3d());
            pLJetRing_->startGlow(startTime, targetOffsetGlobal);

            // lSparks
            if (pLSparks_ == nullptr)
                pLSparks_ = new MachPhysSparks(pLJet_, MexTransform3d());

            hold(pLSparks_, pLJet_, MexTransform3d());

            W4dCompositePlanPtr sparksPlanPtr;

            if (pLSparks_->findCompositePlan("sparks", &sparksPlanPtr))
            {
                pLSparks_->plan(*sparksPlanPtr, startTime, 1000000, MachPhys::CONSTRUCTION_CONSTRUCTING);
            }

            W4dVisibilityPlanPtr sparksVisibilityPlanPtr(new W4dVisibilityPlan(true));
            sparksVisibilityPlanPtr->add(false, 360000);

            pLSparks_->entityPlanForEdit()
                .visibilityPlan(sparksVisibilityPlanPtr, startTime, 0, MachPhys::CONSTRUCTION_CONSTRUCTING);

            // lLight
            if (pLLight_ == nullptr)
            {
                pLLight_ = new W4dPointLight(this, MexVec3(1, 0, 0), 10.0);
                pLLight_->colour(RenColour(3.0, 4.0, 5.0));
                pLLight_->constantAttenuation(0);
                pLLight_->linearAttenuation(0.7);
                pLLight_->quadraticAttenuation(0.3);
                pLLight_->scope(W4dLight::DYNAMIC);

                MexTransform3d lightXform;
                globalTransform().transformInverse(pLJet_->globalTransform(), &lightXform);
                pLLight_->localTransform(lightXform);
            }

            static W4dVisibilityPlanPtr lLightVisibilityPlanPtr;
            static bool first = true;
            if (first)
            {
                first = false;
                lLightVisibilityPlanPtr = new W4dVisibilityPlan(true);
                lLightVisibilityPlanPtr->add(false, 0.3);
                lLightVisibilityPlanPtr->add(true, 0.5);
                lLightVisibilityPlanPtr->add(false, 0.6);
                lLightVisibilityPlanPtr->add(true, 0.8);
                lLightVisibilityPlanPtr->add(false, 0.9);
                lLightVisibilityPlanPtr->add(true, 1.1);
            }

            pLLight_->entityPlanForEdit()
                .visibilityPlan(lLightVisibilityPlanPtr, startTime, 10000, MachPhys::CONSTRUCTION_CONSTRUCTING);

            // lLightShadow
            if (pLLightShadow_ == nullptr)
            {
                pLLightShadow_ = new MachPhysLight(pLJet_, MexTransform3d());

                MexTransform3d lLightShadowXform = pLJet_->globalTransform();
                MexPoint3d lLightShadowPosition = lLightShadowXform.position();
                lLightShadowPosition.z(construction.globalTransform().position().z() + 0.066);

                lLightShadowXform.position(lLightShadowPosition);

                // this = inV(this) * t
                MexTransform3d holdXform = pLJet_->pParent()->globalTransform();
                holdXform.transformInverse(lLightShadowXform);

                hold(pLLightShadow_, pLJet_, holdXform);
            }

            pLLightShadow_->entityPlanForEdit()
                .visibilityPlan(lLightVisibilityPlanPtr, startTime, 10000, MachPhys::CONSTRUCTION_CONSTRUCTING);

            PhysMotionPlanPtr lShadowPlanPtr(new MachPhysOrientationAndPositionPlan(pLLightShadow_, 200.0));
            pLLightShadow_->entityPlanForEdit()
                .absoluteMotion(lShadowPlanPtr, startTime, 10000, MachPhys::CONSTRUCTION_CONSTRUCTING);
        }

        if (pRJet_ != nullptr)
        {
            if (pRJetRing_ == nullptr)
                pRJetRing_ = new MachPhysJetRing(pRJet_, MexTransform3d());

            hold(pRJetRing_, pRJet_, MexTransform3d());
            pRJetRing_->startGlow(startTime, targetOffsetGlobal);

            // rSparks
            if (pRSparks_ == nullptr)
                pRSparks_ = new MachPhysSparks(pRJet_, MexTransform3d());

            hold(pRSparks_, pRJet_, MexTransform3d());

            W4dCompositePlanPtr sparksPlanPtr;

            if (pRSparks_->findCompositePlan("sparks", &sparksPlanPtr))
            {
                pRSparks_->plan(*sparksPlanPtr, startTime, 1000000, MachPhys::CONSTRUCTION_CONSTRUCTING);
            }

            W4dVisibilityPlanPtr sparksVisibilityPlanPtr(new W4dVisibilityPlan(true));
            sparksVisibilityPlanPtr->add(false, 360000);

            pRSparks_->entityPlanForEdit()
                .visibilityPlan(sparksVisibilityPlanPtr, startTime, 0, MachPhys::CONSTRUCTION_CONSTRUCTING);

            // rLight
            if (pRLight_ == nullptr)
            {
                pRLight_ = new W4dPointLight(this, MexVec3(1, 0, 0), 10.0);
                pRLight_->colour(RenColour(3.0, 4.0, 5.0));
                pRLight_->constantAttenuation(0);
                pRLight_->linearAttenuation(0.7);
                pRLight_->quadraticAttenuation(0.3);
                pRLight_->scope(W4dLight::DYNAMIC);

                MexTransform3d lightXform;
                globalTransform().transformInverse(pRJet_->globalTransform(), &lightXform);
                pRLight_->localTransform(lightXform);
            }

            static W4dVisibilityPlanPtr rLightVisibilityPlanPtr;
            static bool once = true;
            if (once)
            {
                once = false;
                rLightVisibilityPlanPtr = new W4dVisibilityPlan(true);
                rLightVisibilityPlanPtr->add(false, 0.1);
                rLightVisibilityPlanPtr->add(true, 0.2);
                rLightVisibilityPlanPtr->add(false, 0.5);
                rLightVisibilityPlanPtr->add(true, 0.8);
                rLightVisibilityPlanPtr->add(false, 0.9);
                rLightVisibilityPlanPtr->add(true, 1.1);
            }

            pRLight_->entityPlanForEdit()
                .visibilityPlan(rLightVisibilityPlanPtr, startTime, 10000, MachPhys::CONSTRUCTION_CONSTRUCTING);

            // rLightShadow
            if (pRLightShadow_ == nullptr)
            {
                pRLightShadow_ = new MachPhysLight(pRJet_, MexTransform3d());

                MexTransform3d rLightShadowXform = pRJet_->globalTransform();
                MexPoint3d rLightShadowPosition = rLightShadowXform.position();
                rLightShadowPosition.z(construction.globalTransform().position().z() + 0.066);

                rLightShadowXform.position(rLightShadowPosition);

                // this = inV(this) * t
                MexTransform3d holdXform = pRJet_->pParent()->globalTransform();
                holdXform.transformInverse(rLightShadowXform);

                hold(pRLightShadow_, pRJet_, holdXform);
            }

            pRLightShadow_->entityPlanForEdit()
                .visibilityPlan(rLightVisibilityPlanPtr, startTime, 10000, MachPhys::CONSTRUCTION_CONSTRUCTING);

            PhysMotionPlanPtr rShadowPlanPtr(new MachPhysOrientationAndPositionPlan(pRLightShadow_, 200.0));
            pRLightShadow_->entityPlanForEdit()
                .absoluteMotion(rShadowPlanPtr, startTime, 10000, MachPhys::CONSTRUCTION_CONSTRUCTING);
        }

        isConstructing_ = true;
    }
}
void MachPhysConstructor::stopConstructing()
{
    if (isConstructing())
    {
        finishAnimation(MachPhys::CONSTRUCTION_CONSTRUCTING);

        if (pLJet_)
        {
            // pLSparks_->clearAnimation( MachPhys::CONSTRUCTION_CONSTRUCTING );
            // pLSparks_->visible( false );

            // pLJetRing_->clearAnimation( MachPhys::CONSTRUCTION_CONSTRUCTING );
            // pLJetRing_->visible( false );

            // pLLight_->entityPlanForEdit().clearAnimation( MachPhys::CONSTRUCTION_CONSTRUCTING );
            // pLLight_->visible( false );

            // pLLightShadow_->entityPlanForEdit().clearAnimation( MachPhys::CONSTRUCTION_CONSTRUCTING );
            // pLLightShadow_->visible( false );

            delete pLSparks_;
            delete pLJetRing_;
            delete pLLight_;
            delete pLLightShadow_;

            pLSparks_ = nullptr;
            pLJetRing_ = nullptr;
            pLLight_ = nullptr;
            pLLightShadow_ = nullptr;
        }

        if (pRJet_)
        {
            // pRSparks_->clearAnimation( MachPhys::CONSTRUCTION_CONSTRUCTING );
            // pRSparks_->visible( false );

            // pRJetRing_->clearAnimation( MachPhys::CONSTRUCTION_CONSTRUCTING );
            // pRJetRing_->visible( false );

            // pRLight_->entityPlanForEdit().clearAnimation( MachPhys::CONSTRUCTION_CONSTRUCTING );
            // pRLight_->visible( false );

            // pRLightShadow_->entityPlanForEdit().clearAnimation( MachPhys::CONSTRUCTION_CONSTRUCTING );
            // pRLightShadow_->visible( false );

            delete pRSparks_;
            delete pRJetRing_;
            delete pRLight_;
            delete pRLightShadow_;

            pRSparks_ = nullptr;
            pRJetRing_ = nullptr;
            pRLight_ = nullptr;
            pRLightShadow_ = nullptr;
        }

        isConstructing_ = false;
    }
}

bool MachPhysConstructor::isConstructing() const
{
    return isConstructing_;
}

void MachPhysConstructor::isConstructing(bool isConstructing)
{
    isConstructing_ = isConstructing;
}

//////////////////////////////////////////////////////////////////////////////////////////
/* End CONSTRUC.CPP *************************************************/
