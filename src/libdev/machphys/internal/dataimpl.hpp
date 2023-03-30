/*
 * D A T A I M P L . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPhysDataImplementation

    A brief description of the class should go in here
*/

#ifndef _MACHPHYS_DATAIMPL_HPP
#define _MACHPHYS_DATAIMPL_HPP

#include "base/base.hpp"
#include "ctl/fixedvec.hpp"

#include "machphys/machphys.hpp"

class MachPhysAggressorData;
class MachPhysAdministratorData;
class MachPhysAPCData;
class MachPhysResourceCarrierData;
class MachPhysGeoLocatorData;
class MachPhysSpyLocatorData;
class MachPhysTechnicianData;
class MachPhysConstructorData;

class MachPhysHardwareLabData;
class MachPhysSmelterData;
class MachPhysFactoryData;
class MachPhysMissileEmplacementData;
class MachPhysGarrisonData;
class MachPhysMineData;
class MachPhysBeaconData;
class MachPhysPodData;

class MachPhysOreHolographData;

class MachPhysMachineData;
class MachPhysConstructionData;
class MachPhysWeaponData;
class MachPhysGeneralData;

class MachPhysDataImplementation
{
public:
    //  Singleton class
    static MachPhysDataImplementation& instance();
    ~MachPhysDataImplementation();

    // Machine Types
    const MachPhysAggressorData& aggressorData(MachPhys::AggressorSubType, size_t hwLevel, size_t swLevel) const;
    const MachPhysAdministratorData&
    administratorData(MachPhys::AdministratorSubType, size_t hwLevel, size_t swLevel) const;
    const MachPhysAPCData& APCData(size_t hwLevel, size_t swLevel) const;
    const MachPhysResourceCarrierData& resourceCarrierData(size_t hwLevel, size_t swLevel) const;
    const MachPhysGeoLocatorData& geoLocatorData(size_t hwLevel, size_t swLevel) const;
    const MachPhysSpyLocatorData& spyLocatorData(size_t hwLevel, size_t swLevel) const;
    const MachPhysTechnicianData& technicianData(MachPhys::TechnicianSubType, size_t hwLevel, size_t swLevel) const;
    const MachPhysConstructorData& constructorData(MachPhys::ConstructorSubType, size_t hwLevel, size_t swLevel) const;

    // Construction Types
    const MachPhysHardwareLabData& hardwareLabData(MachPhys::HardwareLabSubType subType, size_t hwLevel) const;
    const MachPhysSmelterData& smelterData(size_t hwLevel) const;
    const MachPhysFactoryData& factoryData(MachPhys::FactorySubType, size_t hwLevel) const;
    const MachPhysMissileEmplacementData&
    missileEmplacementData(MachPhys::MissileEmplacementSubType, size_t hwLevel) const;
    const MachPhysGarrisonData& garrisonData(size_t hwLevel) const;
    const MachPhysMineData& mineData(size_t hwLevel) const;
    const MachPhysBeaconData& beaconData(size_t hwLevel) const;
    const MachPhysPodData& podData(size_t hwLevel) const;

    // Specials
    const MachPhysOreHolographData& oreHolographData() const;
    const MachPhysWeaponData& weaponData(MachPhys::WeaponType type) const;
    const MachPhysGeneralData& generalData();

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPhysDataImplementation& t);

private:
    // Operation deliberately revoked
    MachPhysDataImplementation(const MachPhysDataImplementation&);

    // Operation deliberately revoked
    MachPhysDataImplementation& operator=(const MachPhysDataImplementation&);

    // Operation deliberately revoked
    bool operator==(const MachPhysDataImplementation&);

    MachPhysDataImplementation();

    enum
    {
        MAX_HARDWARE_LEVELS = 5
    };
    enum
    {
        MAX_SOFTWARE_LEVELS = 5
    };

    enum
    {
        STORE_HARDWARE_SIZE = MAX_HARDWARE_LEVELS + 1
    };
    enum
    {
        STORE_SOFTWARE_SIZE = MAX_SOFTWARE_LEVELS + 1
    };

    friend class MachPhysDataParser;

    using AdministratorDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysAdministratorData*>>;
    using AggressorDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysAggressorData*>>;
    using ConstructorDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysConstructorData*>>;
    using GeoLocatorDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysGeoLocatorData*>>;
    using APCDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysAPCData*>>;
    using ResourceCarrierDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysResourceCarrierData*>>;
    using SpyLocatorDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysSpyLocatorData*>>;
    using TechnicianDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysTechnicianData*>>;

    AdministratorDataStore administratorBossData_;
    AdministratorDataStore administratorOverseerData_;
    AdministratorDataStore administratorCommanderData_;
    AggressorDataStore aggressorGruntData_;
    AggressorDataStore aggressorAssassinData_;
    AggressorDataStore aggressorKnightData_;
    AggressorDataStore aggressorBallistaData_;
    AggressorDataStore aggressorNinjaData_;
    ConstructorDataStore constructorDozerData_;
    ConstructorDataStore constructorBuilderData_;
    ConstructorDataStore constructorBehemothData_;
    GeoLocatorDataStore geoLocatorData_;
    APCDataStore APCData_;
    ResourceCarrierDataStore resourceCarrierData_;
    SpyLocatorDataStore spyLocatorData_;
    TechnicianDataStore technicianLabTechData_;
    TechnicianDataStore technicianTechBoyData_;
    TechnicianDataStore technicianBrainBoxData_;

    using HardwareLabDataStore = ctl_fixed_vector<MachPhysHardwareLabData*>;
    using SmelterDataStore = ctl_fixed_vector<MachPhysSmelterData*>;
    using MineDataStore = ctl_fixed_vector<MachPhysMineData*>;
    using GarrisonDataStore = ctl_fixed_vector<MachPhysGarrisonData*>;
    using BeaconDataStore = ctl_fixed_vector<MachPhysBeaconData*>;
    using PodDataStore = ctl_fixed_vector<MachPhysPodData*>;
    using FactoryDataStore = ctl_fixed_vector<MachPhysFactoryData*>;
    using MissileEmplacementDataStore = ctl_fixed_vector<MachPhysMissileEmplacementData*>;

    HardwareLabDataStore hardwareLabCivilianData_;
    HardwareLabDataStore hardwareLabMilitaryData_;
    SmelterDataStore smelterData_;
    MineDataStore mineData_;
    GarrisonDataStore garrisonData_;
    BeaconDataStore beaconData_;
    PodDataStore podData_;
    FactoryDataStore factoryCivilianData_;
    FactoryDataStore factoryMilitaryData_;
    MissileEmplacementDataStore missileEmplacementTurretData_;
    MissileEmplacementDataStore missileEmplacementSentryData_;
    MissileEmplacementDataStore missileEmplacementLauncherData_;
    MissileEmplacementDataStore missileEmplacementICBMData_;

    using MachineDataStore = ctl_fixed_vector<ctl_fixed_vector<MachPhysMachineData*>>;
    using ConstructionDataStore = ctl_fixed_vector<MachPhysConstructionData*>;

    MachPhysOreHolographData* pOreHolographData_;

    const AggressorDataStore* pDataStore(MachPhys::AggressorSubType subType) const;
    const AdministratorDataStore* pDataStore(MachPhys::AdministratorSubType subType) const;
    const ConstructorDataStore* pDataStore(MachPhys::ConstructorSubType subType) const;
    const TechnicianDataStore* pDataStore(MachPhys::TechnicianSubType subType) const;

    AggressorDataStore* pDataStore(MachPhys::AggressorSubType subType);
    AdministratorDataStore* pDataStore(MachPhys::AdministratorSubType subType);
    ConstructorDataStore* pDataStore(MachPhys::ConstructorSubType subType);
    TechnicianDataStore* pDataStore(MachPhys::TechnicianSubType subType);

    const FactoryDataStore* pDataStore(MachPhys::FactorySubType subType) const;
    const HardwareLabDataStore* pDataStore(MachPhys::HardwareLabSubType subType) const;
    const MissileEmplacementDataStore* pDataStore(MachPhys::MissileEmplacementSubType subType) const;

    FactoryDataStore* pDataStore(MachPhys::FactorySubType subType);
    HardwareLabDataStore* pDataStore(MachPhys::HardwareLabSubType subType);
    MissileEmplacementDataStore* pDataStore(MachPhys::MissileEmplacementSubType subType);

    // data for weapons. No hw or sw subtypes involved.
    using WeaponDataStore = ctl_fixed_vector<MachPhysWeaponData*>;
    WeaponDataStore weaponData_;

    MachPhysWeaponData* pWeaponData(MachPhys::WeaponType);
    WeaponDataStore* pWeaponDataStore();

    MachPhysGeneralData* pGeneralData_; // Stores general data
    MachPhysGeneralData** pGeneralDataStore();
};

#endif

/* End DATAIMPL.HPP *************************************************/
