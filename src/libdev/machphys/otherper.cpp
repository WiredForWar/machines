/*
 * O T H E R P E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  persistence of all classes in machphys other than MachPhysWeapon, MachPhysConstruction, ans MachPhysMachine

#include "machphys/private/otherper.hpp"

#include "render/mesh.hpp"
#include "render/meshinst.hpp"

#include "world4d/generic.hpp"

#include "machphys/beam.hpp"
#include "machphys/blast.hpp"
#include "machphys/bsphere.hpp"
#include "machphys/crakfire.hpp"
#include "machphys/dblflame.hpp"
#include "machphys/damagwav.hpp"
#include "machphys/debrisun.hpp"
#include "machphys/electro.hpp"
#include "machphys/halo.hpp"
#include "machphys/healaura.hpp"
#include "machphys/hsphere.hpp"
#include "machphys/jetringm.hpp"
#include "machphys/mushshaf.hpp"
#include "machphys/mushtop.hpp"
#include "machphys/mushedge.hpp"
#include "machphys/oreholo.hpp"
#include "machphys/particle.hpp"
#include "machphys/plasaura.hpp"
#include "machphys/plasplat.hpp"
#include "machphys/plasmbol.hpp"
#include "machphys/pulsaura.hpp"
#include "machphys/pulsplat.hpp"
#include "machphys/pulseblb.hpp"
#include "machphys/sparks.hpp"
#include "machphys/splat.hpp"
#include "machphys/shockwav.hpp"
#include "machphys/nukewave.hpp"
#include "machphys/radidisc.hpp"
#include "machphys/waterfal.hpp"
#include "machphys/wsphere.hpp"
#include "machphys/vsphere.hpp"
#include "machphys/beebomb.hpp"
#include "machphys/beewave.hpp"
#include "machphys/stingl.hpp"
#include "machphys/stingm.hpp"
#include "machphys/stlaura.hpp"
#include "machphys/stlsplat.hpp"
#include "machphys/loading.hpp"
#include "machphys/loadligh.hpp"
#include "machphys/loadover.hpp"
#include "machphys/loadunde.hpp"
#include "machphys/scavenge.hpp"
#include "machphys/locator.hpp"
#include "machphys/light.hpp"
#include "machphys/lmine.hpp"

PER_DEFINE_PERSISTENT(MachPhysOtherPersistence);

// a pimple for the MachPhysOtherPersistence singlton
struct MachPhysOtherPersistenceImpl
{
    MachPhysPulseBlob* pPulseRifle_;
    MachPhysPulseBlob* pPulseCannon_;

    MachPhysPlasmaBolt* pPlasmaBoltRifle_;
    MachPhysPlasmaBolt* pPlasmaBoltCannon1_;
    MachPhysPlasmaBolt* pPlasmaBoltCannon2_;

    MachPhysBeam* pBeam_;
    MachPhysParticles* pParticles_;

    MachPhysBlackSphere* pBlackSphere_;
    MachPhysWhiteSphere* pWhiteSphere_;

    MachPhysBlast* pBlast_;
    MachPhysCrackFire* pGarillaCrackFire_;
    MachPhysCrackFire* pBeeCrackFire_;

    MachPhysMushroomEdge* pMushroomEdge_;
    MachPhysMushroomShaft* pMushroomShaft_;
    MachPhysMushroomTop* pMushroomTop_;

    MachPhysElectro* pElectricElectro_;
    MachPhysElectro* pVirusElectro_;
    MachPhysLightningHalo* pHaloElectric_;
    MachPhysLightningHalo* pHaloVirus_;

    W4dGeneric* pCentralCrossHealAura_;
    W4dGeneric* pRadialCrossHealAura_;

    MachPhysHelix* pLeftHelix_;
    MachPhysHelix* pRightHelix_;

    MachPhysHemiSphere* pHemiSphere_;
    MachPhysJetRing* pJetRing_;
    MachPhysSparks* pSparks_;

    // MachPhysGroundScorch*     pGroundScorch_;
    MachPhysGroundSplat* pGroundSplat_;
    MachPhysDoublesidedFlame* pDoubleSidedFlame_;

    MachPhysShockWave* pShockWave_;
    MachPhysBurstWave* pBurstWave_;
    MachPhysNukeWave* pNukeWave_;

    MachPhysRadialDisc* pRadialDisc_;
    MachPhysWaterfall* pWaterfall_;

    MachPhysPlasmaAura* pPlasmaAura_;
    MachPhysPlasmaSplat* pPlasmaSplat_;
    MachPhysPulseAura* pPulseAura_;
    MachPhysPulseSplat* pPulseSplat_;

    MachPhysVortexSphere* pVortexSphere_;
    MachPhysBeeBomb* pBeeBomb_;
    MachPhysBeeBombWave* pBeeBombWave_;
    MachPhysBeeBlast* pBeeBlast_;

    MachPhysLightSting* pLightSting_;
    MachPhysLightStingAura* pLightStingAura_;
    MachPhysLightStingSplat* pLightStingSplat_;

    MachPhysMetalSting* pMetalSting_;

    MachPhysFlashDisc* pFlashDiscYellow_;
    MachPhysFlashDisc* pFlashDiscWhite_;

    MachPhysLoadingLight* pLoadingLight_;
    MachPhysLoadingOver* pLoadingOver_;
    MachPhysLoadingUnder* pLoadingUnder_;
    MachPhysScavenger* pScavenger_;
    MachPhysLocator* pLocator_;

    MachPhysFlameBall* pGruntFlameBall_;
    MachPhysFlameBall* pTurretFlameBall_;

    MachPhysDebrisUnit* pDebris_;
    MachPhysOreHolograph* pHolograph_;

    MachPhysLight* pLightShadow_;
    MachPhysLandMine* pLandMine_;
};

// static
MachPhysOtherPersistence& MachPhysOtherPersistence::instance()
{
    static MachPhysOtherPersistence instance_;
    return instance_;
}

MachPhysOtherPersistence::MachPhysOtherPersistence()
    : pImpl_(new MachPhysOtherPersistenceImpl())
    , root_(50000)
{
    // pImpl_->root_ = W4dRoot(50000 ;
    pImpl_->pPulseRifle_ = nullptr;
    pImpl_->pPulseCannon_ = nullptr;
    pImpl_->pPlasmaBoltRifle_ = nullptr;
    pImpl_->pPlasmaBoltCannon1_ = nullptr;
    pImpl_->pPlasmaBoltCannon2_ = nullptr;
    pImpl_->pBeam_ = nullptr;
    pImpl_->pParticles_ = nullptr;
    pImpl_->pBlackSphere_ = nullptr;
    pImpl_->pWhiteSphere_ = nullptr;
    pImpl_->pBlast_ = nullptr;
    pImpl_->pGarillaCrackFire_ = nullptr;
    pImpl_->pBeeCrackFire_ = nullptr;
    pImpl_->pMushroomEdge_ = nullptr;
    pImpl_->pMushroomShaft_ = nullptr;
    pImpl_->pMushroomTop_ = nullptr;
    pImpl_->pElectricElectro_ = nullptr;
    pImpl_->pVirusElectro_ = nullptr;
    pImpl_->pHaloElectric_ = nullptr;
    pImpl_->pHaloVirus_ = nullptr;
    pImpl_->pCentralCrossHealAura_ = nullptr;
    pImpl_->pRadialCrossHealAura_ = nullptr;
    pImpl_->pLeftHelix_ = nullptr;
    pImpl_->pRightHelix_ = nullptr;
    pImpl_->pHemiSphere_ = nullptr;
    pImpl_->pJetRing_ = nullptr;
    pImpl_->pSparks_ = nullptr;
    // pGroundScorch_ = NULL;
    pImpl_->pGroundSplat_ = nullptr;
    pImpl_->pDoubleSidedFlame_ = nullptr;
    pImpl_->pShockWave_ = nullptr;
    pImpl_->pBurstWave_ = nullptr;
    pImpl_->pNukeWave_ = nullptr;
    pImpl_->pRadialDisc_ = nullptr;
    pImpl_->pWaterfall_ = nullptr;
    pImpl_->pPlasmaAura_ = nullptr;
    pImpl_->pPlasmaSplat_ = nullptr;
    pImpl_->pPulseAura_ = nullptr;
    pImpl_->pPulseSplat_ = nullptr;
    pImpl_->pVortexSphere_ = nullptr;
    pImpl_->pBeeBomb_ = nullptr;
    pImpl_->pBeeBombWave_ = nullptr;
    pImpl_->pBeeBlast_ = nullptr;
    pImpl_->pLightSting_ = nullptr;
    pImpl_->pLightStingAura_ = nullptr;
    pImpl_->pLightStingSplat_ = nullptr;
    pImpl_->pMetalSting_ = nullptr;
    pImpl_->pFlashDiscYellow_ = nullptr;
    pImpl_->pFlashDiscWhite_ = nullptr;
    pImpl_->pLoadingLight_ = nullptr;
    pImpl_->pLoadingOver_ = nullptr;
    pImpl_->pLoadingUnder_ = nullptr;
    pImpl_->pScavenger_ = nullptr;
    pImpl_->pLocator_ = nullptr;
    pImpl_->pGruntFlameBall_ = nullptr;
    pImpl_->pTurretFlameBall_ = nullptr;
    pImpl_->pDebris_ = nullptr;
    pImpl_->pHolograph_ = nullptr;
    pImpl_->pLightShadow_ = nullptr;
    pImpl_->pLandMine_ = nullptr;

    TEST_INVARIANT;
}

MachPhysOtherPersistence::~MachPhysOtherPersistence()
{
    delete pImpl_;
    TEST_INVARIANT;
}

W4dRoot* MachPhysOtherPersistence::pRoot()
{
    return &root_;
}

const MachPhysPulseBlob& MachPhysOtherPersistence::pulseBlobExemplar(MachPhys::WeaponType type)
{
    MachPhysPulseBlob** ppBlob = nullptr;

    switch (type)
    {
        case MachPhys::PULSE_RIFLE:
            ppBlob = &(pImpl_->pPulseRifle_);
            break;
        case MachPhys::PULSE_CANNON:
            ppBlob = &(pImpl_->pPulseCannon_);
            break;
            DEFAULT_ASSERT_BAD_CASE(type);
    }

    if (*ppBlob == nullptr)
        *ppBlob = new MachPhysPulseBlob(type);

    return **ppBlob;
}

const MachPhysPlasmaBolt& MachPhysOtherPersistence::plasmaBoltExemplar(MachPhys::WeaponType type)
{
    MachPhysPlasmaBolt** ppBolt = nullptr;

    switch (type)
    {
        case MachPhys::PLASMA_RIFLE:
            ppBolt = &(pImpl_->pPlasmaBoltRifle_);
            break;
        case MachPhys::PLASMA_CANNON1:
            ppBolt = &(pImpl_->pPlasmaBoltCannon1_);
            break;
        case MachPhys::PLASMA_CANNON2:
            ppBolt = &(pImpl_->pPlasmaBoltCannon2_);
            break;
            DEFAULT_ASSERT_BAD_CASE(type);
    }

    if (*ppBolt == nullptr)
        *ppBolt = new MachPhysPlasmaBolt(type);

    return **ppBolt;
}

const MachPhysBeam& MachPhysOtherPersistence::beamExemplar()
{
    if (pImpl_->pBeam_ == nullptr)
        pImpl_->pBeam_ = new MachPhysBeam();

    return *(pImpl_->pBeam_);
}

const MachPhysBlast& MachPhysOtherPersistence::blastExemplar()
{
    if (pImpl_->pBlast_ == nullptr)
        pImpl_->pBlast_ = new MachPhysBlast();

    return *(pImpl_->pBlast_);
}

const MachPhysBlackSphere& MachPhysOtherPersistence::blackSphereExemplar()
{
    if (pImpl_->pBlackSphere_ == nullptr)
        pImpl_->pBlackSphere_ = new MachPhysBlackSphere();

    return *(pImpl_->pBlackSphere_);
}

const MachPhysCrackFire& MachPhysOtherPersistence::crackFireExemplar(MachPhysCrackFire::ChasmType type)
{
    MachPhysCrackFire** ppCrackFire = nullptr;

    switch (type)
    {
        case MachPhysCrackFire::GARILLA_PUNCH_CHASM:

            ppCrackFire = &(pImpl_->pGarillaCrackFire_);
            break;

        case MachPhysCrackFire::BEE_BOMB_CHASM:

            ppCrackFire = &(pImpl_->pBeeCrackFire_);
            break;
    }

    if (*ppCrackFire == nullptr)
        *ppCrackFire = new MachPhysCrackFire(type);

    return **ppCrackFire;
}

const MachPhysBurstWave& MachPhysOtherPersistence::burstWaveExemplar()
{
    if (pImpl_->pBurstWave_ == nullptr)
        pImpl_->pBurstWave_ = new MachPhysBurstWave();

    return *(pImpl_->pBurstWave_);
}

const MachPhysDoublesidedFlame& MachPhysOtherPersistence::doubleSidedFlameExemplar()
{
    if (pImpl_->pDoubleSidedFlame_ == nullptr)
        pImpl_->pDoubleSidedFlame_ = new MachPhysDoublesidedFlame();

    return *(pImpl_->pDoubleSidedFlame_);
}

const MachPhysElectro& MachPhysOtherPersistence::electroExemplar(MachPhysElectro::FlashType type)
{
    MachPhysElectro** ppElectro = nullptr;

    switch (type)
    {
        case MachPhysElectro::VIRUS:
            {
                ppElectro = &(pImpl_->pVirusElectro_);
                break;
            }
        case MachPhysElectro::ELECTRIC:
            {
                ppElectro = &(pImpl_->pElectricElectro_);
                break;
            }
    }
    if (*ppElectro == nullptr)
    {
        *ppElectro = new MachPhysElectro(type);

        // TBD: Disable the back fuce cull via cast here.
        // These needs to be changed so the property is defined in the .x file.
        // Ren::ConstMeshPtr myMesh = _CONST_CAST( const W4dEntity&, *ppElectro).mesh().mesh();
        //_CONST_CAST( RenMesh&, *myMesh ).backFaceAll( false );
    }

    return **ppElectro;
}

const MachPhysLightningHalo& MachPhysOtherPersistence::haloExemplar(MachPhysLightningHalo::HaloType type)
{
    MachPhysLightningHalo** ppHalo;

    switch (type)
    {
        case MachPhysLightningHalo::VIRUS:
            {
                ppHalo = &(pImpl_->pHaloVirus_);
                break;
            }
        case MachPhysLightningHalo::ELECTRIC:
            {
                ppHalo = &(pImpl_->pHaloElectric_);
                break;
            }
    }
    if (*ppHalo == nullptr)
        *ppHalo = new MachPhysLightningHalo(type);

    return **ppHalo;
}

const W4dGeneric& MachPhysOtherPersistence::healAuraCentralCrossExemplar()
{
    if (pImpl_->pCentralCrossHealAura_ == nullptr)
        pImpl_->pCentralCrossHealAura_ = &MachPhysHealAura::newCentralCross();

    return *(pImpl_->pCentralCrossHealAura_);
}

const W4dGeneric& MachPhysOtherPersistence::healAuraRadialCrossExemplar()
{

    if (pImpl_->pRadialCrossHealAura_ == nullptr)
        pImpl_->pRadialCrossHealAura_ = &MachPhysHealAura::newRadialCross();

    return *(pImpl_->pRadialCrossHealAura_);
}

const MachPhysHelix& MachPhysOtherPersistence::helixExemplar(MachPhysHelix::Twist twist)
{
    MachPhysHelix** ppHelix = nullptr;

    switch (twist)
    {
        case MachPhysHelix::LEFT_TWIST:
            ppHelix = &(pImpl_->pLeftHelix_);
            break;
        case MachPhysHelix::RIGHT_TWIST:
            ppHelix = &(pImpl_->pRightHelix_);
            break;
    }
    if (*ppHelix == nullptr)
        *ppHelix = new MachPhysHelix(twist);

    return **ppHelix;
}

const MachPhysJetRing& MachPhysOtherPersistence::jetRingExemplar()
{
    if (pImpl_->pJetRing_ == nullptr)
        pImpl_->pJetRing_ = new MachPhysJetRing();

    return *(pImpl_->pJetRing_);
}

const MachPhysMushroomEdge& MachPhysOtherPersistence::mushroomEdgeExemplar()
{
    if (pImpl_->pMushroomEdge_ == nullptr)
        pImpl_->pMushroomEdge_ = new MachPhysMushroomEdge();

    return *(pImpl_->pMushroomEdge_);
}

const MachPhysMushroomShaft& MachPhysOtherPersistence::mushroomShaftExemplar()
{
    if (pImpl_->pMushroomShaft_ == nullptr)
        pImpl_->pMushroomShaft_ = new MachPhysMushroomShaft();

    return *(pImpl_->pMushroomShaft_);
}

const MachPhysMushroomTop& MachPhysOtherPersistence::mushroomTopExemplar()
{
    if (pImpl_->pMushroomTop_ == nullptr)
        pImpl_->pMushroomTop_ = new MachPhysMushroomTop();

    return *(pImpl_->pMushroomTop_);
}

const MachPhysParticles& MachPhysOtherPersistence::particlesExemplar()
{
    if (pImpl_->pParticles_ == nullptr)
        pImpl_->pParticles_ = new MachPhysParticles();

    return *(pImpl_->pParticles_);
}

const MachPhysRadialDisc& MachPhysOtherPersistence::radialDiscExemplar()
{
    if (pImpl_->pRadialDisc_ == nullptr)
        pImpl_->pRadialDisc_ = new MachPhysRadialDisc();

    return *(pImpl_->pRadialDisc_);
}

/*
const MachPhysGroundScorch& MachPhysOtherPersistence::groundScorchExemplar()
{
    if( pGroundScorch_ == NULL )
        pGroundScorch_ = new MachPhysGroundScorch();

    return *pGroundScorch_;
}
*/

const MachPhysShockWave& MachPhysOtherPersistence::shockWaveExemplar()
{
    if (pImpl_->pShockWave_ == nullptr)
        pImpl_->pShockWave_ = new MachPhysShockWave();

    return *(pImpl_->pShockWave_);
}

const MachPhysNukeWave& MachPhysOtherPersistence::nukeWaveExemplar()
{
    if (pImpl_->pNukeWave_ == nullptr)
        pImpl_->pNukeWave_ = new MachPhysNukeWave();

    return *(pImpl_->pNukeWave_);
}

const MachPhysSparks& MachPhysOtherPersistence::sparksExemplar()
{
    if (pImpl_->pSparks_ == nullptr)
        pImpl_->pSparks_ = new MachPhysSparks();

    return *(pImpl_->pSparks_);
}

const MachPhysGroundSplat& MachPhysOtherPersistence::groundSplatExemplar()
{
    if (pImpl_->pGroundSplat_ == nullptr)
        pImpl_->pGroundSplat_ = new MachPhysGroundSplat();

    return *(pImpl_->pGroundSplat_);
}

const MachPhysWaterfall& MachPhysOtherPersistence::waterfallExemplar()
{
    if (pImpl_->pWaterfall_ == nullptr)
        pImpl_->pWaterfall_ = new MachPhysWaterfall();

    return *(pImpl_->pWaterfall_);
}

const MachPhysWhiteSphere& MachPhysOtherPersistence::whiteSphereExemplar()
{
    if (pImpl_->pWhiteSphere_ == nullptr)
        pImpl_->pWhiteSphere_ = new MachPhysWhiteSphere();

    return *(pImpl_->pWhiteSphere_);
}

const MachPhysPlasmaAura& MachPhysOtherPersistence::plasmaAuraExemplar()
{
    if (pImpl_->pPlasmaAura_ == nullptr)
        pImpl_->pPlasmaAura_ = new MachPhysPlasmaAura();

    return *(pImpl_->pPlasmaAura_);
}

const MachPhysPlasmaSplat& MachPhysOtherPersistence::plasmaSplatExemplar()
{
    if (pImpl_->pPlasmaSplat_ == nullptr)
        pImpl_->pPlasmaSplat_ = new MachPhysPlasmaSplat();

    return *(pImpl_->pPlasmaSplat_);
}

const MachPhysPulseAura& MachPhysOtherPersistence::pulseAuraExemplar()
{
    if (pImpl_->pPulseAura_ == nullptr)
        pImpl_->pPulseAura_ = new MachPhysPulseAura();

    return *(pImpl_->pPulseAura_);
}

const MachPhysPulseSplat& MachPhysOtherPersistence::pulseSplatExemplar()
{
    if (pImpl_->pPulseSplat_ == nullptr)
        pImpl_->pPulseSplat_ = new MachPhysPulseSplat();

    return *(pImpl_->pPulseSplat_);
}

const MachPhysHemiSphere& MachPhysOtherPersistence::hemiSphereExemplar()
{
    if (pImpl_->pHemiSphere_ == nullptr)
        pImpl_->pHemiSphere_ = new MachPhysHemiSphere();

    return *(pImpl_->pHemiSphere_);
}

const MachPhysVortexSphere& MachPhysOtherPersistence::vortexSphereExemplar()
{
    if (pImpl_->pVortexSphere_ == nullptr)
        pImpl_->pVortexSphere_ = new MachPhysVortexSphere();

    return *(pImpl_->pVortexSphere_);
}

const MachPhysBeeBomb& MachPhysOtherPersistence::beeBombExemplar()
{
    if (pImpl_->pBeeBomb_ == nullptr)
        pImpl_->pBeeBomb_ = new MachPhysBeeBomb();

    return *(pImpl_->pBeeBomb_);
}

const MachPhysBeeBombWave& MachPhysOtherPersistence::beeBombWaveExemplar()
{
    if (pImpl_->pBeeBombWave_ == nullptr)
        pImpl_->pBeeBombWave_ = new MachPhysBeeBombWave();

    return *(pImpl_->pBeeBombWave_);
}

const MachPhysLightSting& MachPhysOtherPersistence::lightStingExemplar()
{
    if (pImpl_->pLightSting_ == nullptr)
        pImpl_->pLightSting_ = new MachPhysLightSting();

    return *(pImpl_->pLightSting_);
}

const MachPhysLightStingAura& MachPhysOtherPersistence::lightStingAuraExemplar()
{
    if (pImpl_->pLightStingAura_ == nullptr)
        pImpl_->pLightStingAura_ = new MachPhysLightStingAura();

    return *(pImpl_->pLightStingAura_);
}

const MachPhysLightStingSplat& MachPhysOtherPersistence::lightStingSplatExemplar()
{
    if (pImpl_->pLightStingSplat_ == nullptr)
        pImpl_->pLightStingSplat_ = new MachPhysLightStingSplat();

    return *(pImpl_->pLightStingSplat_);
}

const MachPhysMetalSting& MachPhysOtherPersistence::metalStingExemplar()
{
    if (pImpl_->pMetalSting_ == nullptr)
        pImpl_->pMetalSting_ = new MachPhysMetalSting();

    return *(pImpl_->pMetalSting_);
}

const MachPhysFlashDisc&
MachPhysOtherPersistence::flashDiscExemplar(const MATHEX_SCALAR& size, MachPhysFlashDisc::ColourType colour)
{
    MachPhysFlashDisc** ppFlashDisc = nullptr;

    switch (colour)
    {
        case MachPhysFlashDisc::YELLOW:

            ppFlashDisc = &(pImpl_->pFlashDiscYellow_);
            break;

        case MachPhysFlashDisc::WHITE:

            ppFlashDisc = &(pImpl_->pFlashDiscWhite_);
            break;
    }

    // each coloured type can only have one size which is defined in the parmdata file,
    // but if some other size id defined first, then the size define in the file will not be in effect, which is an
    // error

    if (*ppFlashDisc == nullptr)
        *ppFlashDisc = new MachPhysFlashDisc(size, colour);

    return **ppFlashDisc;
}

const MachPhysLoadingLight& MachPhysOtherPersistence::lightLoadingExemplar()
{
    if (pImpl_->pLoadingLight_ == nullptr)
        pImpl_->pLoadingLight_ = new MachPhysLoadingLight();

    return *(pImpl_->pLoadingLight_);
}

const MachPhysLoadingOver& MachPhysOtherPersistence::overLoadingExemplar()
{
    if (pImpl_->pLoadingOver_ == nullptr)
        pImpl_->pLoadingOver_ = new MachPhysLoadingOver();

    return *(pImpl_->pLoadingOver_);
}

const MachPhysLoadingUnder& MachPhysOtherPersistence::underLoadingExemplar()
{
    if (pImpl_->pLoadingUnder_ == nullptr)
        pImpl_->pLoadingUnder_ = new MachPhysLoadingUnder();

    return *(pImpl_->pLoadingUnder_);
}

const MachPhysScavenger& MachPhysOtherPersistence::scavengerExemplar()
{
    if (pImpl_->pScavenger_ == nullptr)
        pImpl_->pScavenger_ = new MachPhysScavenger();

    return *(pImpl_->pScavenger_);
}

const MachPhysLocator& MachPhysOtherPersistence::locatorExemplar()
{
    if (pImpl_->pLocator_ == nullptr)
        pImpl_->pLocator_ = new MachPhysLocator();

    return *(pImpl_->pLocator_);
}

const MachPhysFlameBall& MachPhysOtherPersistence::flameBallExemplar(MachPhysFlameBall::FlameBallType type)
{
    MachPhysFlameBall** ppFlameBall = nullptr;
    switch (type)
    {
        case MachPhysFlameBall::GRUNT_FLAME_BALL:

            ppFlameBall = &(pImpl_->pGruntFlameBall_);
            break;

        case MachPhysFlameBall::TURRET_FLAME_BALL:

            ppFlameBall = &(pImpl_->pTurretFlameBall_);
            break;
    }
    if (*ppFlameBall == nullptr)
        *ppFlameBall = new MachPhysFlameBall(type);

    return **ppFlameBall;
}

const MachPhysDebrisUnit& MachPhysOtherPersistence::debrisExemplar()
{
    if (pImpl_->pDebris_ == nullptr)
        pImpl_->pDebris_ = new MachPhysDebrisUnit(pRoot(), MexTransform3d());

    return *(pImpl_->pDebris_);
}

const MachPhysOreHolograph& MachPhysOtherPersistence::holographExemplar()
{
    if (pImpl_->pHolograph_ == nullptr)
        pImpl_->pHolograph_ = new MachPhysOreHolograph(pRoot(), MexTransform3d());

    return *(pImpl_->pHolograph_);
}

const MachPhysLight& MachPhysOtherPersistence::lightShadowExemplar()
{
    if (pImpl_->pLightShadow_ == nullptr)
        pImpl_->pLightShadow_ = new MachPhysLight();

    return *(pImpl_->pLightShadow_);
}

const MachPhysLandMine& MachPhysOtherPersistence::landMineExemplar()
{
    if (pImpl_->pLandMine_ == nullptr)
        pImpl_->pLandMine_ = new MachPhysLandMine();

    return *(pImpl_->pLandMine_);
}

void MachPhysOtherPersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysOtherPersistence& t)
{

    o << "MachPhysOtherPersistence " << (void*)&t << " start" << std::endl;
    o << "MachPhysOtherPersistence " << (void*)&t << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachPhysOtherPersistence& otherPersist)
{
    ostr << otherPersist.root_;

    ostr << otherPersist.pImpl_->pPulseRifle_;
    ostr << otherPersist.pImpl_->pPulseCannon_;

    ostr << otherPersist.pImpl_->pPlasmaBoltRifle_;
    ostr << otherPersist.pImpl_->pPlasmaBoltCannon1_;
    ostr << otherPersist.pImpl_->pPlasmaBoltCannon2_;

    ostr << otherPersist.pImpl_->pBeam_;
    ostr << otherPersist.pImpl_->pParticles_;

    ostr << otherPersist.pImpl_->pBlackSphere_;
    ostr << otherPersist.pImpl_->pWhiteSphere_;

    ostr << otherPersist.pImpl_->pBlast_;
    ostr << otherPersist.pImpl_->pGarillaCrackFire_;
    ostr << otherPersist.pImpl_->pBeeCrackFire_;

    ostr << otherPersist.pImpl_->pMushroomEdge_;
    ostr << otherPersist.pImpl_->pMushroomShaft_;
    ostr << otherPersist.pImpl_->pMushroomTop_;

    ostr << otherPersist.pImpl_->pElectricElectro_;
    ostr << otherPersist.pImpl_->pVirusElectro_;
    ostr << otherPersist.pImpl_->pHaloElectric_;
    ostr << otherPersist.pImpl_->pHaloVirus_;

    ostr << otherPersist.pImpl_->pCentralCrossHealAura_;
    ostr << otherPersist.pImpl_->pRadialCrossHealAura_;

    ostr << otherPersist.pImpl_->pLeftHelix_;
    ostr << otherPersist.pImpl_->pRightHelix_;

    ostr << otherPersist.pImpl_->pHemiSphere_;
    ostr << otherPersist.pImpl_->pJetRing_;
    ostr << otherPersist.pImpl_->pSparks_;

    // ostr << otherPersist.pImpl_->pGroundScorch_;
    ostr << otherPersist.pImpl_->pGroundSplat_;
    ostr << otherPersist.pImpl_->pDoubleSidedFlame_;

    ostr << otherPersist.pImpl_->pShockWave_;
    ostr << otherPersist.pImpl_->pBurstWave_;
    ostr << otherPersist.pImpl_->pNukeWave_;

    ostr << otherPersist.pImpl_->pRadialDisc_;
    ostr << otherPersist.pImpl_->pWaterfall_;

    ostr << otherPersist.pImpl_->pPlasmaAura_;
    ostr << otherPersist.pImpl_->pPlasmaSplat_;
    ostr << otherPersist.pImpl_->pPulseAura_;
    ostr << otherPersist.pImpl_->pPulseSplat_;

    ostr << otherPersist.pImpl_->pVortexSphere_;

    ostr << otherPersist.pImpl_->pBeeBomb_;
    ostr << otherPersist.pImpl_->pBeeBombWave_;

    ostr << otherPersist.pImpl_->pLightSting_;
    ostr << otherPersist.pImpl_->pLightStingAura_;
    ostr << otherPersist.pImpl_->pLightStingSplat_;

    ostr << otherPersist.pImpl_->pMetalSting_;

    ostr << otherPersist.pImpl_->pFlashDiscYellow_;
    ostr << otherPersist.pImpl_->pFlashDiscWhite_;

    ostr << otherPersist.pImpl_->pLoadingLight_;
    ostr << otherPersist.pImpl_->pLoadingOver_;
    ostr << otherPersist.pImpl_->pLoadingUnder_;

    ostr << otherPersist.pImpl_->pScavenger_;
    ostr << otherPersist.pImpl_->pLocator_;

    ostr << otherPersist.pImpl_->pGruntFlameBall_;
    ostr << otherPersist.pImpl_->pTurretFlameBall_;

    ostr << otherPersist.pImpl_->pDebris_;
    ostr << otherPersist.pImpl_->pHolograph_;

    ostr << otherPersist.pImpl_->pLightShadow_;
    ostr << otherPersist.pImpl_->pLandMine_;
}

void perRead(PerIstream& istr, MachPhysOtherPersistence& otherPersist)
{
    istr >> otherPersist.root_;

    istr >> otherPersist.pImpl_->pPulseRifle_;
    istr >> otherPersist.pImpl_->pPulseCannon_;

    istr >> otherPersist.pImpl_->pPlasmaBoltRifle_;
    istr >> otherPersist.pImpl_->pPlasmaBoltCannon1_;
    istr >> otherPersist.pImpl_->pPlasmaBoltCannon2_;

    istr >> otherPersist.pImpl_->pBeam_;
    istr >> otherPersist.pImpl_->pParticles_;

    istr >> otherPersist.pImpl_->pBlackSphere_;
    istr >> otherPersist.pImpl_->pWhiteSphere_;

    istr >> otherPersist.pImpl_->pBlast_;
    istr >> otherPersist.pImpl_->pGarillaCrackFire_;
    istr >> otherPersist.pImpl_->pBeeCrackFire_;

    istr >> otherPersist.pImpl_->pMushroomEdge_;
    istr >> otherPersist.pImpl_->pMushroomShaft_;
    istr >> otherPersist.pImpl_->pMushroomTop_;

    istr >> otherPersist.pImpl_->pElectricElectro_;
    istr >> otherPersist.pImpl_->pVirusElectro_;
    istr >> otherPersist.pImpl_->pHaloElectric_;
    istr >> otherPersist.pImpl_->pHaloVirus_;

    istr >> otherPersist.pImpl_->pCentralCrossHealAura_;
    istr >> otherPersist.pImpl_->pRadialCrossHealAura_;

    istr >> otherPersist.pImpl_->pLeftHelix_;
    istr >> otherPersist.pImpl_->pRightHelix_;

    istr >> otherPersist.pImpl_->pHemiSphere_;
    istr >> otherPersist.pImpl_->pJetRing_;
    istr >> otherPersist.pImpl_->pSparks_;

    // istr >> otherPersist.pImpl_->pGroundScorch_;
    istr >> otherPersist.pImpl_->pGroundSplat_;
    istr >> otherPersist.pImpl_->pDoubleSidedFlame_;

    istr >> otherPersist.pImpl_->pShockWave_;
    istr >> otherPersist.pImpl_->pBurstWave_;
    istr >> otherPersist.pImpl_->pNukeWave_;

    istr >> otherPersist.pImpl_->pRadialDisc_;
    istr >> otherPersist.pImpl_->pWaterfall_;

    istr >> otherPersist.pImpl_->pPlasmaAura_;
    istr >> otherPersist.pImpl_->pPlasmaSplat_;
    istr >> otherPersist.pImpl_->pPulseAura_;
    istr >> otherPersist.pImpl_->pPulseSplat_;

    istr >> otherPersist.pImpl_->pVortexSphere_;

    istr >> otherPersist.pImpl_->pBeeBomb_;
    istr >> otherPersist.pImpl_->pBeeBombWave_;

    istr >> otherPersist.pImpl_->pLightSting_;
    istr >> otherPersist.pImpl_->pLightStingAura_;
    istr >> otherPersist.pImpl_->pLightStingSplat_;

    istr >> otherPersist.pImpl_->pMetalSting_;

    istr >> otherPersist.pImpl_->pFlashDiscYellow_;
    istr >> otherPersist.pImpl_->pFlashDiscWhite_;

    istr >> otherPersist.pImpl_->pLoadingLight_;
    istr >> otherPersist.pImpl_->pLoadingOver_;
    istr >> otherPersist.pImpl_->pLoadingUnder_;

    istr >> otherPersist.pImpl_->pScavenger_;
    istr >> otherPersist.pImpl_->pLocator_;

    istr >> otherPersist.pImpl_->pGruntFlameBall_;
    istr >> otherPersist.pImpl_->pTurretFlameBall_;

    istr >> otherPersist.pImpl_->pDebris_;
    istr >> otherPersist.pImpl_->pHolograph_;

    istr >> otherPersist.pImpl_->pLightShadow_;
    istr >> otherPersist.pImpl_->pLandMine_;
}

/* End OTHERPER.CPP **************************************************/
