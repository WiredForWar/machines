/*
 * M A C D M G . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions
#include "machphys/machdmg.hpp"
#include "machphys/machine.hpp"
#include "render/texture.hpp"
#include "machphys/effects.hpp"
#include "machphys/random.hpp"
#include "machphys/machphys.hpp"
#include "machphys/smkcloud.hpp"
#include "machphys/dblflame.hpp"

#include "world4d/entity.hpp"
#include "world4d/link.hpp"
#include "world4d/composit.hpp"

#include "mathex/degrees.hpp"
#include "mathex/eulerang.hpp"
#include "mathex/abox3d.hpp"

#include "sim/manager.hpp"

PER_DEFINE_PERSISTENT(MachPhysMachineDamage);

MachPhysMachineDamage::MachPhysMachineDamage(MachPhysMachine* pTarget)
    : pTarget_(pTarget)
    , damageLevel_(0)
    , pFlame1_(nullptr)
    , pFlame2_(nullptr)
    , pSmoke1_(nullptr)
    , pSmoke2_(nullptr)
{
    PRE(damageLevel_ >= 0);

    update();

    TEST_INVARIANT;
}

MachPhysMachineDamage::MachPhysMachineDamage(PerConstructor)
{
}

MachPhysMachineDamage::~MachPhysMachineDamage()
{

    delete pFlame1_;
    delete pFlame2_;
    delete pSmoke1_;
    delete pSmoke2_;

    TEST_INVARIANT;
}

void MachPhysMachineDamage::CLASS_INVARIANT
{
    INVARIANT(damageLevel_ >= 0);
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysMachineDamage& t)
{

    o << "MachPhysMachineDamage " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysMachineDamage " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachPhysMachineDamage::update()
{
    PRE(pTarget_);

    PhysAbsoluteTime now = SimManager::instance().currentTime();

    if ((damageLevel_ < 65 || damageLevel_ >= 90) && pSmoke1_)
    {
        delete pSmoke1_;
        pSmoke1_ = nullptr;
    }

    if ((damageLevel_ < 90 || damaged()) && pSmoke2_)
    {
        delete pSmoke2_;
        pSmoke2_ = nullptr;
    }
    if (damageLevel_ < 90 && pFlame1_)
    {
        delete pFlame1_;
        pFlame1_ = nullptr;
    }

    if (damageLevel_ < 90 && pFlame2_)
    {
        delete pFlame2_;
        pFlame2_ = nullptr;
    }
    MATHEX_SCALAR sizeX, sizeY, sizeZ;

    sizeX = pTarget_->compositeBoundingVolume().xLength();
    sizeY = pTarget_->compositeBoundingVolume().yLength();
    sizeZ = pTarget_->compositeBoundingVolume().zLength();

    if (damageLevel_ >= 65 && damageLevel_ < 90)
    {
        if (pSmoke1_ == nullptr)
        {
            MATHEX_SCALAR wispSize = std::max(sizeX, sizeY);

            pSmoke1_ = new MachPhysSmokeCloud(pTarget_, MexPoint3d(0, 0, sizeZ * 0.9));

            pSmoke1_->startSmoking(
                1, // nWisps
                MexPoint3d(0, 0, sizeZ), // endPosition
                MexVec3(0, 0, 0), // randomOffset
                now, // startTime
                0.5, // wispInterval
                wispSize,
                0.0, // zDepthOffset
                MachPhysEffects::smokeMaterialPlan(MachPhysEffects::BLACK),
                10000,
                4);
        }
    }

    if (damageLevel_ >= 90)
    {
        // Smoke # 2 initiated. Colour of smoke to be dark rust red.
        if (pSmoke2_ == nullptr)
        {
            MATHEX_SCALAR wispSize = std::max(sizeX, sizeY);

            pSmoke2_ = new MachPhysSmokeCloud(pTarget_, MexPoint3d(0, 0, sizeZ * 0.9));

            pSmoke2_->startSmoking(
                3, // nWisps
                MexPoint3d(0, 0, sizeZ * 1.5), // endPosition
                MexVec3(0.0, 0.02, 0), // randomOffset
                now, // startTime
                0.05, // wispInterval
                wispSize,
                0.0, // zDepthOffset
                MachPhysEffects::smokeMaterialPlan(MachPhysEffects::BLACK),
                1000,
                5);
        }

        static double flame1Width, flame2Width, flameHeight;

        if (pFlame1_ == nullptr || pFlame2_ == nullptr)
        {
            delete pFlame1_;
            pFlame1_ = nullptr;
            delete pFlame2_;
            pFlame2_ = nullptr;

            // Traverse the links of the machine
            const W4dComposite::W4dLinks& mLinks = pTarget_->links();

            W4dLink* pTopLink = *mLinks.begin();
            MATHEX_SCALAR height = pTopLink->boundingVolume().maxCorner().z();

            // See if a faceplate link is defined to make sure that flame will not set on it.
            W4dLink* pFaceplate_ = nullptr;
            bool linkFound = pTarget_->findLink("faceplate", &pFaceplate_) || pTarget_->findLink("fplat", &pFaceplate_);

            for (W4dComposite::W4dLinks::const_iterator it = mLinks.begin(); it != mLinks.end(); ++it)
            {
                // get the volume of this link
                W4dLink* pLink = *it;
                MATHEX_SCALAR linkHeight = pLink->boundingVolume().maxCorner().z();
                if (linkHeight > height && pLink != pFaceplate_)
                {
                    height = linkHeight;
                    pTopLink = pLink;
                }
            }

            flame1Width = pTopLink->boundingVolume().yLength();
            flame2Width = pTopLink->boundingVolume().xLength();
            flameHeight = 2.0 * pTopLink->boundingVolume().zLength();

            MexPoint3d centroid = pTopLink->boundingVolume().centroid();

            MexTransform3d linkTransform;
            pTarget_->linkTransform(*pTopLink, &linkTransform);

            linkTransform.transform(&centroid);
            MexTransform3d flameTransform(centroid);
            flameTransform.translate(MexPoint3d(0, 0, flameHeight * 0.48));

            MexTransform3d flame1Transform(MexPoint3d(0, 0, flameHeight * 0.48));
            pFlame1_ = new MachPhysDoublesidedFlame(pTopLink, flame1Transform);

            flameTransform.rotate(MexEulerAngles(MexDegrees(90), 0, 0));

            MexTransform3d flame2Transform(MexPoint3d(0, 0, flameHeight * 0.48));
            flame2Transform.rotate(MexEulerAngles(MexDegrees(90), 0, 0));

            pFlame2_ = new MachPhysDoublesidedFlame(pTopLink, flame2Transform);

            pTarget_->hold(pFlame1_, pTopLink, flame1Transform);
            pTarget_->hold(pFlame2_, pTopLink, flame2Transform);
        }

        pFlame1_->startFlame(flame1Width, flameHeight, now, 60);
        pFlame2_->startFlame(flame2Width, flameHeight, now, 60);
    }
}

void MachPhysMachineDamage::damageLevel(const double& percent)
{
    damageLevel_ = percent;
    update();
}

const double& MachPhysMachineDamage::damageLevel() const
{
    return damageLevel_;
}

bool MachPhysMachineDamage::damaged() const
{
    return damageLevel_ > 100;
}

void perWrite(PerOstream& str, const MachPhysMachineDamage& t)
{
    str << t.pTarget_;
    str << t.damageLevel_;
    str << t.pFlame1_;
    str << t.pFlame2_;
    str << t.pSmoke1_;
    str << t.pSmoke2_;
}

void perRead(PerIstream& str, MachPhysMachineDamage& t)
{
    str >> t.pTarget_;
    str >> t.damageLevel_;
    str >> t.pFlame1_;
    str >> t.pFlame2_;
    str >> t.pSmoke1_;
    str >> t.pSmoke2_;
}

/* End MACHDMG.CPP ***************************************************/
