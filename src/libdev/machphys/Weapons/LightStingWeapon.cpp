/*
 * S T G L W E P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Weapons/LightStingWeapon.hpp"

#include "system/PathName.hpp"

#include "ctl/Vector.hpp"

#include "mathex/Transform3d.hpp"
#include "render/Mesh.hpp"
#include "render/MeshInstance.hpp"
#include "render/Colour.hpp"
#include "render/MaterialVec.hpp"
#include "render/Material.hpp"
#include "render/Texture.hpp"
#include "render/TextureManager.hpp"

#include "world4d/Entity/Composite.hpp"
#include "world4d/Entity/Link.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Entity/EntityPlan.hpp"
#include "world4d/Plans/ColourPulseData.hpp"
#include "world4d/Plans/ColourPulsePlan.hpp"
#include "world4d/Plans/MaterialPlan.hpp"
#include "world4d/Entity/EntityIter.hpp"
#include "world4d/Sound/SoundManager.hpp"
#include "world4d/Entity/CompositePlan.hpp"

#include "machphys/Persistence/WeaponPersistence.hpp"
#include "machphys/Weapons/LightSting.hpp"
#include "machphys/Weapons/WeaponData.hpp"
#include "machphys/Data/SoundData.hpp"

#include "sim/Manager.hpp"

PER_DEFINE_PERSISTENT(MachPhysLightStingWeapon);

// publiv constructor
MachPhysLightStingWeapon::MachPhysLightStingWeapon(W4dEntity* pParent, const MexTransform3d& localTransform)
    : MachPhysLinearWeapon(exemplar(), MachPhys::TOP, pParent, localTransform)
{

    startGlow(SimManager::instance().currentTime());
    TEST_INVARIANT;
}

// one time constructor
MachPhysLightStingWeapon::MachPhysLightStingWeapon()
    : MachPhysLinearWeapon(
        MachPhysWeaponPersistence::instance().pRoot(),
        MexTransform3d(),
        SysPathName(compositeFilePath()),
        MachPhys::WASP_LIGHT_STING,
        MachPhys::TOP)
{

    TEST_INVARIANT;
}

// static
const char* MachPhysLightStingWeapon::compositeFilePath()
{
    return "models/weapons/wasp/light/lightgun.cdf";
}

MachPhysLightStingWeapon::~MachPhysLightStingWeapon()
{
    TEST_INVARIANT;
}

void MachPhysLightStingWeapon::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysLightStingWeapon& t)
{

    o << "MachPhysLightStingWeapon " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysLightStingWeapon " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// static
const MachPhysLightStingWeapon& MachPhysLightStingWeapon::exemplar()
{
    return MachPhysWeaponPersistence::instance().lightStingExemplar();
}

// virtual
MachPhysLinearProjectile* MachPhysLightStingWeapon::createProjectile(
    const PhysAbsoluteTime& burstStartTime,
    uint index,
    W4dEntity* pParent,
    const W4dEntity& target,
    const MexPoint3d& targetOffset)
{

    return createLightSting(burstStartTime, index, pParent, target, targetOffset);
}

// virtual
PhysRelativeTime MachPhysLightStingWeapon::fire(const PhysAbsoluteTime& startTime, int numberInBurst)
{
    const PhysRelativeTime duration = 3.0;

    lighting(RenColour(10.2, 8.0, 2.0), startTime, duration);

    W4dSoundManager::instance().play(this, SID_WASP_STING_LIGHT, 0, 1);

    // the weapon stops glowing for while then start to glow again
    stopGlow();
    startGlow(startTime + duration);

    // do recoil if any
    return recoil(startTime, numberInBurst);
}

MachPhysLightSting* MachPhysLightStingWeapon::createLightSting(
    const PhysAbsoluteTime& burstStartTime,
    uint index,
    W4dEntity* pParent,
    const W4dEntity& target,
    const MexPoint3d& targetOffset)
{
    MexTransform3d startTransform;
    MATHEX_SCALAR distance;
    PhysAbsoluteTime launchTime
        = launchData(burstStartTime, index, pParent, target, targetOffset, &startTransform, &distance);

    MachPhysLightSting* pSting = new MachPhysLightSting(pParent, startTransform);

    // let it spin if can
    W4dCompositePlanPtr spinPlanPtr;

    if (pSting->findCompositePlan("spin", &spinPlanPtr))
    {
        pSting->plan(*spinPlanPtr, launchTime, 1000000);
    }

    // Make it fly
    pSting->move(launchTime, weaponData());

    return pSting;
}

MachPhysLightStingWeapon::MachPhysLightStingWeapon(PerConstructor con)
    : MachPhysLinearWeapon(con)
{
}

void perWrite(PerOstream& ostr, const MachPhysLightStingWeapon& weapon)
{
    const MachPhysLinearWeapon& base = weapon;

    ostr << base;
}

void perRead(PerIstream& istr, MachPhysLightStingWeapon& weapon)
{
    MachPhysLinearWeapon& base = weapon;

    istr >> base;
}

static const ctl_pvector<W4dColourPulseData>& colourPulseDataVec()
{
    //      MACH_B FROM 20 20 20 TO 255 150 0 EVERY 0.1 MAX_LOD 0
    static W4dColourPulseData pulseData(
        "MACH_B.bmp",
        RenColour(20.0 / 255.0, 20.0 / 255.0, 20.0 / 255.0),
        RenColour(1, 150.0 / 255.0, 0),
        0.1);
    static ctl_pvector<W4dColourPulseData> dataVec;
    static bool first = true;
    if (first)
    {
        dataVec.reserve(1);
        dataVec.push_back(&pulseData);
    }
    return dataVec;
}

static const RenTexture& glowTexture()
{
    static const RenTexture texture = RenTexManager::instance().createTexture("MACH_B.bmp");

    return texture;
}

void MachPhysLightStingWeapon::startGlow(const PhysAbsoluteTime& startTime)
{
    static const ctl_pvector<W4dColourPulseData>& dataVec = colourPulseDataVec();

    size_t nLinks = links().size();

    for (size_t iLink = 0; iLink < nLinks; ++iLink)
    {
        W4dLink* pLink = links()[iLink];
        if (pLink->hasMesh())
        {
            W4dColourPulsePlan::makePlan(pLink, dataVec, startTime, GLOWING);
        }
    }
}

void MachPhysLightStingWeapon::stopGlow()
{
    // Clear the plans with animation id animId immediately.
    // Does this for the composite and all its links.
    clearAnimation(GLOWING);

    size_t nLinks = links().size();

    for (size_t iLink = 0; iLink < nLinks; ++iLink)
    {
        W4dLink* pLink = links()[iLink];
        // pLink->entityPlanForEdit().clearAnimation( GLOWING );

        if (pLink->hasMesh())
        {
            W4dColourPulsePlan::changeColour(pLink, glowTexture(), RenColour::black());
        }
    }
}

/* End STGLWEP.CPP **************************************************/
