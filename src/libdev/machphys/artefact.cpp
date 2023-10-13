/*
 * A R T E F A C T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/artefact.hpp"

#include "base/diag.hpp"
#include "machphys/artfdata.hpp"
#include "machphys/damage.hpp"
#include "machphys/objexplo.hpp"

#include "ctl/countptr.hpp"
#include "mathex/point3d.hpp"
#include "mathex/cvexpgon.hpp"

#include "world4d/entity.hpp"
#include "world4d/composit.hpp"
#include "world4d/compplan.hpp"
#include "sim/manager.hpp"

#ifndef _INLINE
#include "machphys/artefact.ipp"
#endif

MachPhysArtefact::MachPhysArtefact(W4dEntity* pModel, const MachPhysArtefactData& data)
    : pModel_(pModel)
    , pData_(&data)
    , pDamage_(nullptr)
{
    HAL_STREAM("MachPhysArtefact::CTOR " << (void*)this << std::endl);
    // If the model is a composite and has an animation, play it
    if (pModel->isComposite())
    {
        W4dComposite& compositeModel = pModel->asComposite();
        W4dCompositePlanPtr planPtr;
        if (compositeModel.cycleAnims(&planPtr))
            compositeModel.plan(*planPtr, SimManager::instance().currentTime(), 30000);
    }

    TEST_INVARIANT;
}

MachPhysArtefact::~MachPhysArtefact()
{
    HAL_STREAM("MachPhysArtefact::DTOR " << (void*)this << std::endl);
    delete pDamage_;
    TEST_INVARIANT;
    HAL_STREAM("MachPhysArtefact::DTOR DONE " << (void*)this << std::endl);
}

void MachPhysArtefact::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysArtefact& t)
{

    o << "MachPhysArtefact " << (void*)&t << " start" << std::endl;
    o << "MachPhysArtefact " << (void*)&t << " end" << std::endl;

    return o;
}

MexConvexPolygon2d* MachPhysArtefact::newGlobalBoundary() const
{
    MexConvexPolygon2d* pNewPoly = new MexConvexPolygon2d(artefactData().obstaclePolygon());
    pNewPoly->transform(pModel_->globalTransform());

    return pNewPoly;
}

void MachPhysArtefact::damageLevel(const double& percent)
{
    HAL_STREAM("MachPhysArtefact::damageLevel " << percent << " " << (void*)this << std::endl);
    if (percent > 0 && pDamage_ == nullptr)
    {
        pDamage_ = new MachPhysEntityDamage(pModel_);
    }

    if (pDamage_ != nullptr)
    {
        pDamage_->damageLevel(percent);

        if (percent == 0.0)
        {
            HAL_STREAM(" damage level is zero so deleting pDamage " << std::endl);
            delete pDamage_;
            pDamage_ = nullptr;
        }
        if (percent <= MexEpsilon::instance())
        {
            HAL_STREAM(" damage level is <= MexEpsilon so Assert_fail " << std::endl);
            ASSERT_FAIL("zero check was not sufficient in damageLevel\n");
            delete pDamage_;
            pDamage_ = nullptr;
        }
    }
}

const double MachPhysArtefact::damageLevel() const
{
    if (pDamage_)

        return pDamage_->damageLevel();
    else

        return 0;
}

void MachPhysArtefact::updateDamageLevel()
{
    if (pDamage_)
        pDamage_->update();
    if (damaged())
        destroy(SimManager::instance().currentTime());
}

bool MachPhysArtefact::damaged() const
{
    return damageLevel() >= 100;
}

// does nothing
PhysRelativeTime MachPhysArtefact::destroy(const PhysAbsoluteTime& startTime)
{
    // Elimiante any animations in a composite
    if (pModel_->isComposite())
        pModel_->asComposite().clearAllPlans();

    // Construct and use an explosion
    MachPhysObjectExplosion exploder(pModel_);
    PhysRelativeTime animationDuration = exploder.explode(startTime);

    return animationDuration;
}

const MachPhysArtefactData& MachPhysArtefact::artefactData() const
{
    return *pData_;
}

/* End ARTEFACT.CPP *************************************************/
