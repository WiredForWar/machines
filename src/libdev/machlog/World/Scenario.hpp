/*
 * S C E N A R I O . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    Scenario

    A brief description of the class should go in here
*/

#ifndef _SCENARIO_HPP
#define _SCENARIO_HPP

#include "base/base.hpp"
#include "machlog/World/MachLog.hpp"
#include "machlog/World/Camera.hpp"

class MachLogGameCreationData;
class SysPathName;
class UtlLineTokeniser;

class MachLogScenario
{
public:
    MachLogScenario() {};
    ~MachLogScenario();

    static void load(const SysPathName& scenarioFilePath, const MachLogGameCreationData&);

    // Sets up what a scenario file would otherwise set up, for a world that has
    // no scenario: the non-race that owns artefacts, and a local player race for
    // every colour the creation data defines. Nothing is placed, and no victory
    // condition is created -- a lone race would win an annihilation on the first
    // cycle.
    static void loadBareWorld(const MachLogGameCreationData&);

    static MachLog::ObjectType objectType(const std::string& type);
    static int objectSubType(MachLog::ObjectType, const std::string& type);
    static MachPhys::TechnicianSubType technicianSubType(const std::string& subType);
    static MachPhys::ConstructorSubType constructorSubType(const std::string& subType);
    static MachPhys::AggressorSubType aggressorSubType(const std::string& subType);
    static MachPhys::AdministratorSubType administratorSubType(const std::string& subType);

    static MachPhys::FactorySubType factorySubType(const std::string& subType);
    static MachPhys::HardwareLabSubType hardwareLabSubType(const std::string& subType);
    static MachPhys::MissileEmplacementSubType missileEmplacementSubType(const std::string& subType);

    static MachPhys::WeaponCombo weaponCombo(const std::string& weaponCombo);
    static MachPhys::Race machPhysRace(const std::string& race);
    static MachLog::ObjectType physConstructionToLogObject(MachPhys::ConstructionType);
    static MachLogCamera::Type cameraType(const std::string& cameraType);
    static MachLog::BeaconType virtualBeaconType(const std::string& beaconType);
    static MachLog::ResourcesAvailable resourceAvailable(const std::string& resourceAvailableToken);
    static int startingResourcesToInt(MachLog::StartingResources);
    static void parseRestrictConstruction(const UtlLineTokeniser&);

    void CLASS_INVARIANT;

private:
    // Operation deliberately revoked
    MachLogScenario(const MachLogScenario&);

    // Operation deliberately revoked
    MachLogScenario& operator=(const MachLogScenario&);

    // Operation deliberately revoked
    bool operator==(const MachLogScenario&);
};

#endif

/* End SCENARIO.HPP *************************************************/
