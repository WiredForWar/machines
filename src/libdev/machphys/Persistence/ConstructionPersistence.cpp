/*
 * C O N S P E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Persistence/ConstructionPersistence.hpp"

#include "machphys/Constructions/Beacon.hpp"
#include "machphys/Constructions/Factory.hpp"
#include "machphys/Constructions/Garrison.hpp"
#include "machphys/Constructions/Mine.hpp"
#include "machphys/Constructions/MissileEmplacement.hpp"
#include "machphys/Constructions/Pod.hpp"
#include "machphys/Constructions/Smelter.hpp"
#include "machphys/Constructions/HardwareLab.hpp"
#include "machphys/Weapons/Missile.hpp"

#include "machphys/ModelExporter.hpp"

PER_DEFINE_PERSISTENT(MachPhysConstructionPersistence);

// static
MachPhysConstructionPersistence& MachPhysConstructionPersistence::instance()
{
    static MachPhysConstructionPersistence instance_;
    return instance_;
}

MachPhysConstructionPersistence::MachPhysConstructionPersistence()
{

    TEST_INVARIANT;
}

MachPhysConstructionPersistence::~MachPhysConstructionPersistence()
{
    TEST_INVARIANT;
}

void MachPhysConstructionPersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysConstructionPersistence& t)
{

    o << "MachPhysConstructionPersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysConstructionPersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachPhysConstructionPersistence&)
{
    ostr << MachPhysBeacon::factory();
    ostr << MachPhysFactory::factory();
    ostr << MachPhysGarrison::factory();
    ostr << MachPhysMine::factory();
    ostr << MachPhysMissileEmplacement::factory();
    ostr << MachPhysPod::factory();
    ostr << MachPhysSmelter::factory();
    ostr << MachPhysHardwareLab::factory();
    ostr << MachPhysMissile::factory();
}

void perRead(PerIstream& istr, MachPhysConstructionPersistence&)
{
    istr >> MachPhysBeacon::factory();
    istr >> MachPhysFactory::factory();
    istr >> MachPhysGarrison::factory();
    istr >> MachPhysMine::factory();
    istr >> MachPhysMissileEmplacement::factory();
    istr >> MachPhysPod::factory();
    istr >> MachPhysSmelter::factory();
    istr >> MachPhysHardwareLab::factory();
    istr >> MachPhysMissile::factory();
}

static void addPath(CdfPathMap& m, const SysPathName& p)
{
    std::string s = p.pathname();
    if (s.empty())
        return;
    auto slash = s.rfind('/');
    std::string key = (slash != std::string::npos) ? s.substr(slash + 1) : s;
    m[key] = s;
}

void MachPhysConstructionPersistence::exportModels(const SysPathName& outputDir) const
{
    CdfPathMap m;

    // Beacon: 1-3
    for (size_t lv : {1, 2, 3})
        addPath(m, MachPhysBeacon::compositeFileName(lv));

    // Factory: MILITARY 1,3,4,5; CIVILIAN 1,3,5
    for (size_t lv : {1, 3, 4, 5})
    {
        addPath(m, MachPhysFactory::compositeFileName(MachPhys::MILITARY, lv));
        addPath(m, MachPhysFactory::interiorCompositeFileName(MachPhys::MILITARY, lv));
    }
    for (size_t lv : {1, 3, 5})
    {
        addPath(m, MachPhysFactory::compositeFileName(MachPhys::CIVILIAN, lv));
        addPath(m, MachPhysFactory::interiorCompositeFileName(MachPhys::CIVILIAN, lv));
    }

    // Garrison: 1
    addPath(m, MachPhysGarrison::compositeFileName(1));
    addPath(m, MachPhysGarrison::interiorCompositeFileName(1));

    // Mine: 1, 3, 5
    for (size_t lv : {1, 3, 5})
        addPath(m, MachPhysMine::compositeFileName(lv));

    // MissileEmplacement: TURRET 1-3, SENTRY 3-4, LAUNCHER 4, ICBM 5
    for (size_t lv : {1, 2, 3})
        addPath(m, MachPhysMissileEmplacement::compositeFileName(MachPhys::TURRET, lv));
    for (size_t lv : {3, 4})
        addPath(m, MachPhysMissileEmplacement::compositeFileName(MachPhys::SENTRY, lv));
    addPath(m, MachPhysMissileEmplacement::compositeFileName(MachPhys::LAUNCHER, 4));
    addPath(m, MachPhysMissileEmplacement::compositeFileName(MachPhys::ICBM, 5));

    // Pod: 1
    addPath(m, MachPhysPod::compositeFileName(1));
    addPath(m, MachPhysPod::interiorCompositeFileName(1));

    // Smelter: 1, 3
    for (size_t lv : {1, 3})
    {
        addPath(m, MachPhysSmelter::compositeFileName(lv));
        addPath(m, MachPhysSmelter::interiorCompositeFileName(lv));
    }

    // HardwareLab: LAB_MILITARY 1,3; LAB_CIVILIAN 1,3
    for (auto st : {MachPhys::LAB_MILITARY, MachPhys::LAB_CIVILIAN})
        for (size_t lv : {1, 3})
        {
            addPath(m, MachPhysHardwareLab::compositeFileName(st, lv));
            addPath(m, MachPhysHardwareLab::interiorCompositeFileName(st, lv));
        }

    // Missile: 1-7
    for (size_t lv = 1; lv <= 7; ++lv)
        addPath(m, MachPhysMissile::compositeFileName(lv));

    MachPhysExportFromRoot(MachPhysBeacon::factory().root(), outputDir, "beacon", m);
    MachPhysExportFromRoot(MachPhysFactory::factory().root(), outputDir, "factory", m);
    MachPhysExportFromRoot(MachPhysGarrison::factory().root(), outputDir, "garrison", m);
    MachPhysExportFromRoot(MachPhysMine::factory().root(), outputDir, "mine", m);
    MachPhysExportFromRoot(MachPhysMissileEmplacement::factory().root(), outputDir, "missemp", m);
    MachPhysExportFromRoot(MachPhysPod::factory().root(), outputDir, "pod", m);
    MachPhysExportFromRoot(MachPhysSmelter::factory().root(), outputDir, "smelter", m);
    MachPhysExportFromRoot(MachPhysHardwareLab::factory().root(), outputDir, "hwlab", m);
    MachPhysExportFromRoot(MachPhysMissile::factory().root(), outputDir, "missile", m);
}

/* End CONSPER.CPP **************************************************/
