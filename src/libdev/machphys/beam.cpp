/*
 * B E A M . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "mathex/point2d.hpp"
#include "machphys/beam.hpp"

#include "stdlib/string.hpp"
#include "ctl/vector.hpp"
#include "mathex/transf3d.hpp"

#include "system/pathname.hpp"
#include "machphys/private/otherper.hpp"

#include "phys/rampacce.hpp"
#include "phys/asclplan.hpp"
#include "phys/lsclplan.hpp"
#include "phys/timeangl.hpp"
#include "phys/motplan.hpp"
#include "phys/linemoti.hpp"

#include "mathex/angle.hpp"
#include "mathex/degrees.hpp"

#include "render/scale.hpp"
#include "render/mesh.hpp"
#include "render/meshinst.hpp"

#include "world4d/entity.hpp"
#include "world4d/link.hpp"
#include "world4d/entyplan.hpp"
#include "world4d/visplan.hpp"
#include "world4d/gusplan.hpp"
#include "world4d/simplsca.hpp"
#include "world4d/compplan.hpp"

#include "machphys/ionbeam.hpp"

PER_DEFINE_PERSISTENT(MachPhysBeam);

static const PhysScalarPlanPtr& waveScalarPlanPtr();
static const PhysScalarPlanPtr& beamScalarPlanPtr();
static PhysRelativeTime waveExpansionTime();
static PhysRelativeTime expansionTime();

// One-time ctor
MachPhysBeam::MachPhysBeam()
    : W4dComposite(MachPhysOtherPersistence::instance().pRoot(), MexTransform3d(), W4dEntity::SOLID)
    , pBeam_(nullptr)
    , pFlare_(nullptr)
    , pOuter_(nullptr)
    , pWave_(nullptr)
{
    // Load the mesh data
    readCompositeFile(SysPathName("models/weapons/ioncann/beam.cdf"));

    findLink("beam", &pBeam_);
    findLink("flare", &pFlare_);
    findLink("outer", &pOuter_);
    findLink("wave", &pWave_);

    ASSERT(pBeam_, "Can't find beam mesh");
    ASSERT(pOuter_, "Can't find outet mesh");
    ASSERT(pFlare_, "Can't find flare mesh");
    ASSERT(pWave_, "Can't find wave mesh");

    setMaterialFogMultipliers();

    TEST_INVARIANT;
}

// public ctor
MachPhysBeam::MachPhysBeam(W4dEntity* pParent, const MexTransform3d& localTransform)
    : W4dComposite(exemplar(), pParent, localTransform)
    , pBeam_(nullptr)
    , pFlare_(nullptr)
    , pOuter_(nullptr)
    , pWave_(nullptr)
{
    pBeam_ = links()[exemplar().pBeam_->id()];
    pFlare_ = links()[exemplar().pFlare_->id()];
    pOuter_ = links()[exemplar().pOuter_->id()];
    pWave_ = links()[exemplar().pWave_->id()];

    // make invisible until required
    // visible( false );
    doNotLight(false);

    pBeam_->visible(false);
    pFlare_->visible(false);
    pOuter_->visible(false);
    pWave_->visible(false);

    pBeam_->doNotLight(false);
    pFlare_->doNotLight(false);
    pOuter_->doNotLight(false);
    pWave_->doNotLight(false);

    TEST_INVARIANT;
}

MachPhysBeam::~MachPhysBeam()
{
    TEST_INVARIANT;
}

// static
const MachPhysBeam& MachPhysBeam::exemplar()
{
    return MachPhysOtherPersistence::instance().beamExemplar();
}

void MachPhysBeam::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysBeam& t)
{

    o << "MachPhysBeam " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysBeam " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachPhysBeam::intersectsLine(const MexLine3d&, MATHEX_SCALAR*, Accuracy) const
{
    return false;
}

MachPhysBeam::MachPhysBeam(PerConstructor con)
    : W4dComposite(con)
{
}

void perWrite(PerOstream& ostr, const MachPhysBeam& beam)
{
    const W4dComposite& base = beam;

    ostr << base;
    ostr << beam.pBeam_;
    ostr << beam.pFlare_;
    ostr << beam.pOuter_;
    ostr << beam.pWave_;
}

void perRead(PerIstream& istr, MachPhysBeam& beam)
{
    W4dComposite& base = beam;

    istr >> base;
    istr >> beam.pBeam_;
    istr >> beam.pFlare_;
    istr >> beam.pOuter_;
    istr >> beam.pWave_;
}

static PhysRelativeTime expansionTime()
{
    static const PhysRelativeTime TIME = MachPhysIonBeam::duration();
    return TIME;
}

static PhysRelativeTime waveExpansionTime()
{
    static const PhysRelativeTime TIME = MachPhysIonBeam::duration() + 0.25;
    POST(TIME > expansionTime());

    return TIME;
}

static const PhysScalarPlanPtr& beamScalarPlanPtr()
{
    static PhysScalarPlanPtr scalarPlanPtr;
    static bool first = true;

    if (first)
    {
        first = false;

        PRE(expansionTime() != 0);

        PhysLinearScalarPlan::ScalarVec bTimes;
        bTimes.reserve(1);
        bTimes.push_back(expansionTime());

        PhysLinearScalarPlan::ScalarVec bScales;
        bScales.reserve(2);
        bScales.push_back(0.01);
        bScales.push_back(MachPhysIonBeam::radius());

        scalarPlanPtr = new PhysLinearScalarPlan(bTimes, bScales);
    }
    return scalarPlanPtr;
}

static const PhysScalarPlanPtr& waveScalarPlanPtr()
{
    const PhysRelativeTime& time1 = expansionTime();
    const PhysRelativeTime& time2 = waveExpansionTime();

    ASSERT_INFO(time1);
    ASSERT_INFO(time2);
    ASSERT(time1 > 0, "");
    ASSERT(time2 > time1, "");

    static PhysScalarPlanPtr scalarPlanPtr;
    static bool first = true;

    if (first)
    {
        first = false;

        PhysLinearScalarPlan::ScalarVec wTimes;
        wTimes.reserve(2);
        wTimes.push_back(time1);
        wTimes.push_back(time2);

        PhysLinearScalarPlan::ScalarVec wScales;
        wScales.reserve(3);
        wScales.push_back(0.01);
        wScales.push_back(MachPhysIonBeam::radius());
        wScales.push_back(MachPhysIonBeam::radius());

        scalarPlanPtr = new PhysLinearScalarPlan(wTimes, wScales);
    }
    return scalarPlanPtr;
}

PhysRelativeTime MachPhysBeam::startBeam(const PhysAbsoluteTime& startTime)
{
    // visibility plans
    W4dVisibilityPlanPtr bVisibilityPlanPtr(new W4dVisibilityPlan(true));
    bVisibilityPlanPtr->add(false, expansionTime());

    pBeam_->entityPlanForEdit().visibilityPlan(bVisibilityPlanPtr, startTime);
    pFlare_->entityPlanForEdit().visibilityPlan(bVisibilityPlanPtr, startTime);

    const PhysRelativeTime shrinkTime = 0.25;
    PhysRelativeTime outerDuration = expansionTime() + shrinkTime;

    W4dVisibilityPlanPtr oVisibilityPlanPtr(new W4dVisibilityPlan(true));
    oVisibilityPlanPtr->add(false, outerDuration);

    pOuter_->entityPlanForEdit().visibilityPlan(oVisibilityPlanPtr, startTime);

    const PhysRelativeTime waveShrinkTime = shrinkTime * 3.0;
    PhysRelativeTime waveDuration = expansionTime() + waveShrinkTime;
    W4dVisibilityPlanPtr wVisibilityPlanPtr(new W4dVisibilityPlan(true));
    wVisibilityPlanPtr->add(false, waveDuration);

    pWave_->entityPlanForEdit().visibilityPlan(wVisibilityPlanPtr, startTime);

    // scale plan
    static const W4dScalePlanPtr beamScalePlanPtr(new W4dGeneralUniformScalePlan(beamScalarPlanPtr()));
    pBeam_->propogateScalePlan(beamScalePlanPtr, startTime);
    pFlare_->propogateScalePlan(beamScalePlanPtr, startTime);
    pOuter_->propogateScalePlan(beamScalePlanPtr, startTime);

    PhysAbsoluteTime endTime = startTime + expansionTime();
    MATHEX_SCALAR scalar = beamScalarPlanPtr()->scalar(expansionTime());

    // outer shrink to a vertical line in 0.25 second
    RenNonUniformScale a(scalar, scalar, scalar);
    RenNonUniformScale b(0.01, 0.01, 100);
    W4dScalePlanPtr oPlanPtr(new W4dSimpleNonUniformScalePlan(a, b, shrinkTime));

    pOuter_->propogateScalePlan(oPlanPtr, endTime);

    // wave expand to radius in outerDuration then shrink to a vertical line in 0.5 second
    static const W4dScalePlanPtr wScalePlanPtr(new W4dGeneralUniformScalePlan(waveScalarPlanPtr()));
    pWave_->propogateScalePlan(wScalePlanPtr, startTime);

    PhysAbsoluteTime waveExpEndTime = startTime + outerDuration;
    b = RenNonUniformScale(0.2, 0.2, 0.5);
    W4dScalePlanPtr wPlanPtr(new W4dSimpleNonUniformScalePlan(a, b, shrinkTime));
    pWave_->propogateScalePlan(wPlanPtr, waveExpEndTime);

    RenNonUniformScale c(0.005, 0.005, 2000);
    W4dScalePlanPtr wPlanPtr2(new W4dSimpleNonUniformScalePlan(b, c, shrinkTime));
    pWave_->propogateScalePlan(wPlanPtr2, waveExpEndTime + shrinkTime);

    // let the beam coil
    W4dCompositePlanPtr coilPlanPtr;

    if (findCompositePlan("coil", &coilPlanPtr))
    {
        plan(*coilPlanPtr, startTime, 1000000);
    }

    return expansionTime() + waveShrinkTime;
}
MATHEX_SCALAR MachPhysBeam::outerRadius(const PhysRelativeTime& timeOffset)
{
    PRE(timeOffset <= expansionTime());
    PRE(timeOffset >= 0);

    static const PhysScalarPlanPtr& scalarPlanPtr = beamScalarPlanPtr();
    return scalarPlanPtr->scalar(timeOffset);
}

MATHEX_SCALAR MachPhysBeam::beamRadius(const PhysRelativeTime& timeOffset)
{
    PRE(timeOffset <= expansionTime());
    PRE(timeOffset >= 0);

    static const PhysScalarPlanPtr& scalarPlanPtr = beamScalarPlanPtr();
    return scalarPlanPtr->scalar(timeOffset) * 0.242072;
}

MATHEX_SCALAR MachPhysBeam::waveRadius(const PhysRelativeTime& timeOffset)
{
    PRE(timeOffset <= waveExpansionTime());
    PRE(timeOffset >= 0);

    static const PhysScalarPlanPtr& scalarPlanPtr = waveScalarPlanPtr();
    return scalarPlanPtr->scalar(timeOffset) * 3.0; // wave default radius ~= 3.0
}

void setFogMultipliers(const W4dEntity* entity)
{
    PRE(entity);

    if (entity->hasMesh())
    {
        RenMesh* meshPtr(_CONST_CAST(RenMesh*, &(*entity->mesh().mesh())));

        std::unique_ptr<RenMaterialVec> materialASet = meshPtr->materialVec();
        RenMaterialVec& materialSet = *materialASet;
        for (RenMaterialVec::iterator i = materialSet.begin(); i != materialSet.end(); ++i)
        {
            if ((*i).hasFogMultiplier())
            {
                (*i).fogMultiplier(2.0);
            }
        }

        meshPtr->materialVec(&materialSet);
    }
}

void MachPhysBeam::setMaterialFogMultipliers()
{
    setFogMultipliers(this);
    for (W4dComposite::W4dLinks::const_iterator it = links().begin(); it != links().end(); ++it)
    {
        setFogMultipliers((*it));
    }
}

/* End BEAM.CPP *************************************************/
