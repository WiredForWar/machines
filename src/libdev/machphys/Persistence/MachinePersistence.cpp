/*
 * M A C H P E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/Persistence/MachinePersistence.hpp"

#include "machphys/Machines/Administrator.hpp"
#include "machphys/Machines/Aggressor.hpp"
#include "machphys/Machines/APC.hpp"
#include "machphys/Machines/Constructor.hpp"
#include "machphys/Machines/GeoLocator.hpp"
#include "machphys/Machines/ResourceCarrier.hpp"
#include "machphys/Machines/SpyLocator.hpp"
#include "machphys/Machines/Technician.hpp"

#include "machphys/ObjectFactory.hpp"
#include "machphys/ModelExporter.hpp"

PER_DEFINE_PERSISTENT(MachPhysMachinePersistence);

// static
MachPhysMachinePersistence& MachPhysMachinePersistence::instance()
{
    static MachPhysMachinePersistence instance_;
    return instance_;
}

MachPhysMachinePersistence::MachPhysMachinePersistence()
{

    TEST_INVARIANT;
}

MachPhysMachinePersistence::~MachPhysMachinePersistence()
{
    TEST_INVARIANT;
}

void MachPhysMachinePersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysMachinePersistence& t)
{

    o << "MachPhysMachinePersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysMachinePersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachPhysMachinePersistence&)
{
    ostr << MachPhysAdministrator::factory();
    ostr << MachPhysAggressor::factory();
    ostr << MachPhysAPC::factory();
    ostr << MachPhysConstructor::factory();
    ostr << MachPhysGeoLocator::factory();
    ostr << MachPhysResourceCarrier::factory();
    ostr << MachPhysSpyLocator::factory();
    ostr << MachPhysTechnician::factory();
}

void perRead(PerIstream& istr, MachPhysMachinePersistence&)
{
    istr >> MachPhysAdministrator::factory();
    istr >> MachPhysAggressor::factory();
    istr >> MachPhysAPC::factory();
    istr >> MachPhysConstructor::factory();
    istr >> MachPhysGeoLocator::factory();
    istr >> MachPhysResourceCarrier::factory();
    istr >> MachPhysSpyLocator::factory();
    istr >> MachPhysTechnician::factory();
}

static void addPath(CdfPathMap& m, const SysPathName& p)
{
    std::string s = p.pathname();
    auto slash = s.rfind('/');
    std::string key = (slash != std::string::npos) ? s.substr(slash + 1) : s;
    m[key] = s;
}

void MachPhysMachinePersistence::exportModels(const SysPathName& outputDir) const
{
    CdfPathMap m;

    // Administrator: BOSS 1-2, OVERSEER 2-3, COMMANDER 4-5
    for (size_t lv : {1, 2})
        addPath(m, MachPhysAdministrator::compositeFileName(MachPhys::BOSS, lv));
    for (size_t lv : {2, 3})
        addPath(m, MachPhysAdministrator::compositeFileName(MachPhys::OVERSEER, lv));
    for (size_t lv : {4, 5})
        addPath(m, MachPhysAdministrator::compositeFileName(MachPhys::COMMANDER, lv));

    // Aggressor: GRUNT 1-3, ASSASSIN 2-3, BALLISTA 3-4, KNIGHT 3-5, NINJA 5
    for (size_t lv : {1, 2, 3})
        addPath(m, MachPhysAggressor::compositeFileName(MachPhys::GRUNT, lv));
    for (size_t lv : {2, 3})
        addPath(m, MachPhysAggressor::compositeFileName(MachPhys::ASSASSIN, lv));
    for (size_t lv : {3, 4})
        addPath(m, MachPhysAggressor::compositeFileName(MachPhys::BALLISTA, lv));
    for (size_t lv : {3, 4, 5})
        addPath(m, MachPhysAggressor::compositeFileName(MachPhys::KNIGHT, lv));
    for (size_t lv : {5})
        addPath(m, MachPhysAggressor::compositeFileName(MachPhys::NINJA, lv));

    // APC: 2, 4, 5
    for (size_t lv : {2, 4, 5})
        addPath(m, MachPhysAPC::compositeFileName(lv));

    // Constructor: DOZER 1, BUILDER 3-4, BEHEMOTH 4-5
    for (size_t lv : {1})
        addPath(m, MachPhysConstructor::compositeFileName(MachPhys::DOZER, lv));
    for (size_t lv : {3})
        addPath(m, MachPhysConstructor::compositeFileName(MachPhys::BUILDER, lv));
    for (size_t lv : {4, 5})
        addPath(m, MachPhysConstructor::compositeFileName(MachPhys::BEHEMOTH, lv));

    // GeoLocator: 1-3
    for (size_t lv : {1, 2, 3})
        addPath(m, MachPhysGeoLocator::compositeFileName(lv));

    // ResourceCarrier: 1-5
    for (size_t lv : {1, 2, 3, 4, 5})
        addPath(m, MachPhysResourceCarrier::compositeFileName(lv));

    // SpyLocator: 3, 5
    for (size_t lv : {3, 5})
        addPath(m, MachPhysSpyLocator::compositeFileName(lv));

    // Technician: LAB_TECH 2, TECH_BOY 3, BRAIN_BOX 5
    for (size_t lv : {2})
        addPath(m, MachPhysTechnician::compositeFileName(MachPhys::LAB_TECH, lv));
    for (size_t lv : {3})
        addPath(m, MachPhysTechnician::compositeFileName(MachPhys::TECH_BOY, lv));
    for (size_t lv : {5})
        addPath(m, MachPhysTechnician::compositeFileName(MachPhys::BRAIN_BOX, lv));

    MachPhysExportFromRoot(MachPhysAdministrator::factory().root(), outputDir, "admin", m);
    MachPhysExportFromRoot(MachPhysAggressor::factory().root(), outputDir, "aggressor", m);
    MachPhysExportFromRoot(MachPhysAPC::factory().root(), outputDir, "apc", m);
    MachPhysExportFromRoot(MachPhysConstructor::factory().root(), outputDir, "constructor", m);
    MachPhysExportFromRoot(MachPhysGeoLocator::factory().root(), outputDir, "geolocator", m);
    MachPhysExportFromRoot(MachPhysResourceCarrier::factory().root(), outputDir, "rescarrier", m);
    MachPhysExportFromRoot(MachPhysSpyLocator::factory().root(), outputDir, "spylocator", m);
    MachPhysExportFromRoot(MachPhysTechnician::factory().root(), outputDir, "technician", m);

    // Export shared models (wheels, tracks, hover, faceplate) from mesh data
    // embedded in machine exemplars.
    std::vector<const W4dRoot*> roots = {
        &MachPhysAdministrator::factory().root(),
        &MachPhysAggressor::factory().root(),
        &MachPhysAPC::factory().root(),
        &MachPhysConstructor::factory().root(),
        &MachPhysGeoLocator::factory().root(),
        &MachPhysResourceCarrier::factory().root(),
        &MachPhysSpyLocator::factory().root(),
        &MachPhysTechnician::factory().root(),
    };
    MachPhysModelExporter::exportSharedModels(outputDir, roots);
}

/* End MACHPER.CPP **************************************************/
