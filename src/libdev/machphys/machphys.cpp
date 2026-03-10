/*
 * M A C H P H Y S . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "machphys/machphys.hpp"

#include "utility/String.hpp"

#include <ostream>
#include <type_traits>

// Race

std::string_view MachPhys::toString(Race race)
{
    switch (race)
    {
    case RED:
        return "RED";
    case BLUE:
        return "BLUE";
    case GREEN:
        return "GREEN";
    case YELLOW:
        return "YELLOW";
    case N_RACES:
        return "N_RACES";
    case NORACE:
        break;
    }
    return {};
}

std::optional<MachPhys::Race> MachPhys::toRace(std::string_view str)
{
    for (Race r : AllRaces)
        if (Utils::caseInsensitiveEqual(toString(r), str))
            return r;
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::Race type)
{
    return o << MachPhys::toString(type);
}

// MachineType

std::string_view MachPhys::toString(MachineType type)
{
    switch (type)
    {
    case AGGRESSOR:
        return "AGGRESSOR";
    case GEO_LOCATOR:
        return "GEO_LOCATOR";
    case SPY_LOCATOR:
        return "SPY_LOCATOR";
    case CONSTRUCTOR:
        return "CONSTRUCTOR";
    case ADMINISTRATOR:
        return "ADMINISTRATOR";
    case RESOURCE_CARRIER:
        return "RESOURCE_CARRIER";
    case APC:
        return "APC";
    case TECHNICIAN:
        return "TECHNICIAN";
    }
    return {};
}

std::optional<MachPhys::MachineType> MachPhys::toMachineType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::MachineType>;

    for (Underlying i = 0; i <= TECHNICIAN; ++i)
    {
        MachineType v = static_cast<MachineType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::MachineType type)
{
    return o << MachPhys::toString(type);
}

// ConstructionType

std::string_view MachPhys::toString(ConstructionType type)
{
    switch (type)
    {
    case HARDWARE_LAB:
        return "HARDWARE_LAB";
    case SMELTER:
        return "SMELTER";
    case FACTORY:
        return "FACTORY";
    case MISSILE_EMPLACEMENT:
        return "MISSILE_EMPLACEMENT";
    case GARRISON:
        return "GARRISON";
    case MINE:
        return "MINE";
    case BEACON:
        return "BEACON";
    case POD:
        return "POD";
    }
    return {};
}

std::optional<MachPhys::ConstructionType> MachPhys::toConstructionType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::ConstructionType>;

    for (Underlying i = 0; i <= POD; ++i)
    {
        ConstructionType v = static_cast<ConstructionType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::ConstructionType type)
{
    return o << MachPhys::toString(type);
}

// AggressorSubType

std::string_view MachPhys::toString(AggressorSubType type)
{
    switch (type)
    {
    case GRUNT:
        return "GRUNT";
    case ASSASSIN:
        return "ASSASSIN";
    case KNIGHT:
        return "KNIGHT";
    case BALLISTA:
        return "BALLISTA";
    case NINJA:
        return "NINJA";
    }
    return {};
}

std::optional<MachPhys::AggressorSubType> MachPhys::toAggressorSubType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::AggressorSubType>;

    for (Underlying i = 0; i <= NINJA; ++i)
    {
        AggressorSubType v = static_cast<AggressorSubType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::AggressorSubType type)
{
    return o << MachPhys::toString(type);
}

// AdministratorSubType

std::string_view MachPhys::toString(AdministratorSubType type)
{
    switch (type)
    {
    case BOSS:
        return "BOSS";
    case OVERSEER:
        return "OVERSEER";
    case COMMANDER:
        return "COMMANDER";
    }
    return {};
}

std::optional<MachPhys::AdministratorSubType> MachPhys::toAdministratorSubType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::AdministratorSubType>;

    for (Underlying i = 0; i <= COMMANDER; ++i)
    {
        AdministratorSubType v = static_cast<AdministratorSubType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::AdministratorSubType type)
{
    return o << MachPhys::toString(type);
}

// TechnicianSubType

std::string_view MachPhys::toString(TechnicianSubType type)
{
    switch (type)
    {
    case LAB_TECH:
        return "LAB_TECH";
    case TECH_BOY:
        return "TECH_BOY";
    case BRAIN_BOX:
        return "BRAIN_BOX";
    }
    return {};
}

std::optional<MachPhys::TechnicianSubType> MachPhys::toTechnicianSubType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::TechnicianSubType>;

    for (Underlying i = 0; i <= BRAIN_BOX; ++i)
    {
        TechnicianSubType v = static_cast<TechnicianSubType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::TechnicianSubType type)
{
    return o << MachPhys::toString(type);
}

// ConstructorSubType

std::string_view MachPhys::toString(ConstructorSubType type)
{
    switch (type)
    {
    case DOZER:
        return "DOZER";
    case BUILDER:
        return "BUILDER";
    case BEHEMOTH:
        return "BEHEMOTH";
    }
    return {};
}

std::optional<MachPhys::ConstructorSubType> MachPhys::toConstructorSubType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::ConstructorSubType>;

    for (Underlying i = 0; i <= BEHEMOTH; ++i)
    {
        ConstructorSubType v = static_cast<ConstructorSubType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::ConstructorSubType type)
{
    return o << MachPhys::toString(type);
}

// FactorySubType

std::string_view MachPhys::toString(FactorySubType type)
{
    switch (type)
    {
    case CIVILIAN:
        return "CIVILIAN";
    case MILITARY:
        return "MILITARY";
    }
    return {};
}

std::optional<MachPhys::FactorySubType> MachPhys::toFactorySubType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::FactorySubType>;

    for (Underlying i = 0; i <= MILITARY; ++i)
    {
        FactorySubType v = static_cast<FactorySubType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::FactorySubType type)
{
    return o << MachPhys::toString(type);
}

// HardwareLabSubType

std::string_view MachPhys::toString(HardwareLabSubType type)
{
    switch (type)
    {
    case LAB_CIVILIAN:
        return "LAB_CIVILIAN";
    case LAB_MILITARY:
        return "LAB_MILITARY";
    }
    return {};
}

std::optional<MachPhys::HardwareLabSubType> MachPhys::toHardwareLabSubType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::HardwareLabSubType>;

    for (Underlying i = 0; i <= LAB_MILITARY; ++i)
    {
        HardwareLabSubType v = static_cast<HardwareLabSubType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::HardwareLabSubType type)
{
    return o << MachPhys::toString(type);
}

// MissileEmplacementSubType

std::string_view MachPhys::toString(MissileEmplacementSubType type)
{
    switch (type)
    {
    case TURRET:
        return "TURRET";
    case SENTRY:
        return "SENTRY";
    case LAUNCHER:
        return "LAUNCHER";
    case ICBM:
        return "ICBM";
    }
    return {};
}

std::optional<MachPhys::MissileEmplacementSubType> MachPhys::toMissileEmplacementSubType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::MissileEmplacementSubType>;

    for (Underlying i = 0; i <= ICBM; ++i)
    {
        MissileEmplacementSubType v = static_cast<MissileEmplacementSubType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::MissileEmplacementSubType type)
{
    return o << MachPhys::toString(type);
}

// WeaponType

std::string_view MachPhys::toString(WeaponType type)
{
    switch (type)
    {
    case ELECTRIC_CHARGE:
        return "ELECTRIC_CHARGE";
    case FLAME_THROWER1:
        return "FLAME_THROWER1";
    case FLAME_THROWER2:
        return "FLAME_THROWER2";
    case BOLTER:
        return "BOLTER";
    case AUTO_CANNON:
        return "AUTO_CANNON";
    case GORILLA_PUNCH:
        return "GORILLA_PUNCH";
    case PLASMA_RIFLE:
        return "PLASMA_RIFLE";
    case PULSE_RIFLE:
        return "PULSE_RIFLE";
    case PLASMA_CANNON1:
        return "PLASMA_CANNON1";
    case PLASMA_CANNON2:
        return "PLASMA_CANNON2";
    case PULSE_CANNON:
        return "PULSE_CANNON";
    case HEAVY_BOLTER1:
        return "HEAVY_BOLTER1";
    case HEAVY_BOLTER2:
        return "HEAVY_BOLTER2";
    case HOMING_MISSILE:
        return "HOMING_MISSILE";
    case LARGE_MISSILE:
        return "LARGE_MISSILE";
    case MULTI_LAUNCHER1:
        return "MULTI_LAUNCHER1";
    case MULTI_LAUNCHER2:
        return "MULTI_LAUNCHER2";
    case MULTI_LAUNCHER3:
        return "MULTI_LAUNCHER3";
    case MULTI_LAUNCHER4:
        return "MULTI_LAUNCHER4";
    case MULTI_LAUNCHER5:
        return "MULTI_LAUNCHER5";
    case MULTI_LAUNCHER6:
        return "MULTI_LAUNCHER6";
    case MULTI_LAUNCHER7:
        return "MULTI_LAUNCHER7";
    case SUPERCHARGE_ADVANCED:
        return "SUPERCHARGE_ADVANCED";
    case SUPERCHARGE_SUPER:
        return "SUPERCHARGE_SUPER";
    case BEE_BOMB:
        return "BEE_BOMB";
    case NUCLEAR_MISSILE:
        return "NUCLEAR_MISSILE";
    case VORTEX:
        return "VORTEX";
    case ION_ORBITAL_CANNON:
        return "ION_ORBITAL_CANNON";
    case ORB_OF_TREACHERY:
        return "ORB_OF_TREACHERY";
    case VIRUS:
        return "VIRUS";
    case WASP_LIGHT_STING:
        return "WASP_LIGHT_STING";
    case WASP_METAL_STING:
        return "WASP_METAL_STING";
    case N_WEAPON_TYPES:
        break;
    }
    return {};
}

std::optional<MachPhys::WeaponType> MachPhys::toWeaponType(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::WeaponType>;

    for (Underlying i = 0; i < N_WEAPON_TYPES; ++i)
    {
        WeaponType v = static_cast<WeaponType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::WeaponType type)
{
    return o << MachPhys::toString(type);
}

// Mounting

std::string_view MachPhys::toString(Mounting type)
{
    switch (type)
    {
    case RIGHT:
        return "RIGHT";
    case LEFT:
        return "LEFT";
    case TOP:
        return "TOP";
    case N_MOUNTINGS:
        break;
    }
    return {};
}

std::optional<MachPhys::Mounting> MachPhys::toMounting(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::Mounting>;

    for (Underlying i = 0; i < N_MOUNTINGS; ++i)
    {
        Mounting v = static_cast<Mounting>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::Mounting type)
{
    return o << MachPhys::toString(type);
}

// WeaponCombo

std::string_view MachPhys::toString(WeaponCombo type)
{
    switch (type)
    {
    case L_BOLTER:
        return "L_BOLTER";
    case T_ION_ORBITAL_CANNON:
        return "T_ION_ORBITAL_CANNON";
    case L_ELECTRIC_CHARGE:
        return "L_ELECTRIC_CHARGE";
    case R_BOLTER:
        return "R_BOLTER";
    case T_WASP_LIGHT_STING:
        return "T_WASP_LIGHT_STING";
    case R_ELECTRIC_CHARGE:
        return "R_ELECTRIC_CHARGE";
    case LR_PULSE_RIFLE_X2:
        return "LR_PULSE_RIFLE_X2";
    case L_PULSE_RIFLE_R_SUPERCHARGE_ADVANCED:
        return "L_PULSE_RIFLE_R_SUPERCHARGE_ADVANCED";
    case T_WASP_METAL_STING:
        return "T_WASP_METAL_STING";
    case LR_PLASMA_RIFLE_X2:
        return "LR_PLASMA_RIFLE_X2";
    case T_BEE_BOMB:
        return "T_BEE_BOMB";
    case L_MULTI_LAUNCHER5_R_SUPERCHARGE_SUPER:
        return "L_MULTI_LAUNCHER5_R_SUPERCHARGE_SUPER";
    case L_MULTI_LAUNCHER5_R_TREACHERY:
        return "L_MULTI_LAUNCHER5_R_TREACHERY";
    case LR_PULSE_CANNON_X2:
        return "LR_PULSE_CANNON_X2";
    case L_PULSE_CANNON_R_SUPERCHARGE_SUPER:
        return "L_PULSE_CANNON_R_SUPERCHARGE_SUPER";
    case L_PULSE_CANNON_R_VIRUS:
        return "L_PULSE_CANNON_R_VIRUS";
    case L_FLAME_THROWER2:
        return "L_FLAME_THROWER2";
    case LR_HEAVY_BOLTER1_X2:
        return "LR_HEAVY_BOLTER1_X2";
    case L_MULTI_LAUNCHER7_T_GORILLA_PUNCH_R_MULTI_LAUNCHER7:
        return "L_MULTI_LAUNCHER7_T_GORILLA_PUNCH_R_MULTI_LAUNCHER7";
    case L_AUTO_CANNON:
        return "L_AUTO_CANNON";
    case L_PLASMA_RIFLE:
        return "L_PLASMA_RIFLE";
    case LR_AUTO_CANNON_X2:
        return "LR_AUTO_CANNON_X2";
    case T_NUCLEAR_MISSILE:
        return "T_NUCLEAR_MISSILE";
    case T_MULTI_LAUNCHER1:
        return "T_MULTI_LAUNCHER1";
    case T_VORTEX:
        return "T_VORTEX";
    case LR_MULTI_LAUNCHER2_X2:
        return "LR_MULTI_LAUNCHER2_X2";
    case LR_HEAVY_BOLTER2_X2:
        return "LR_HEAVY_BOLTER2_X2";
    case LR_PLASMA_CANNON1_X2:
        return "LR_PLASMA_CANNON1_X2";
    case LR_MULTI_LAUNCHER3_X2:
        return "LR_MULTI_LAUNCHER3_X2";
    case LR_PLASMA_CANNON2_X2:
        return "LR_PLASMA_CANNON2_X2";
    case LRT_MULTI_LAUNCHER4_X3:
        return "LRT_MULTI_LAUNCHER4_X3";
    case LRT_PLASMA_CANNON2_X3:
        return "LRT_PLASMA_CANNON2_X3";
    case LR_MULTI_LAUNCHER7_X2:
        return "LR_MULTI_LAUNCHER7_X2";
    case T_FLAME_THROWER1:
        return "T_FLAME_THROWER1";
    case LR_MULTI_LAUNCHER5_X2:
        return "LR_MULTI_LAUNCHER5_X2";
    case LR_MULTI_LAUNCHER6_X2:
        return "LR_MULTI_LAUNCHER6_X2";
    case LR_LARGE_MISSILE_X2:
        return "LR_LARGE_MISSILE_X2";
    case N_WEAPON_COMBOS:
        break;
    }
    return {};
}

std::optional<MachPhys::WeaponCombo> MachPhys::toWeaponCombo(std::string_view str)
{
    using Underlying = std::underlying_type_t<MachPhys::WeaponCombo>;

    for (Underlying i = 0; i < N_WEAPON_COMBOS; ++i)
    {
        WeaponCombo v = static_cast<WeaponCombo>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::WeaponCombo type)
{
    return o << MachPhys::toString(type);
}

// FinalState

std::string_view MachPhys::toString(FinalState type)
{
    switch (type)
    {
    case AT_REST:
        return "AT_REST";
    case IN_MOTION:
        return "IN_MOTION";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, MachPhys::FinalState type)
{
    return o << MachPhys::toString(type);
}

// AnimationIds

std::ostream& operator<<(std::ostream& o, MachPhys::AnimationIds animId)
{
    switch (animId)
    {
    case MachPhys::CONSTRUCTION_WORKING:
        o << "CONSTRUCTION_WORKING";
        break;
    case MachPhys::CONSTRUCTION_CONSTRUCTING:
        o << "CONSTRUCTION_CONSTRUCTING";
        break;
    case MachPhys::MACHINE_LOCOMOTION:
        o << "MACHINE_LOCOMOTION";
        break;
    case MachPhys::CONSTRUCTION_NOT_WORKING:
        o << "CONSTRUCTION_NOT_WORKING";
        break;
    }
    return o;
}

// MachPhysPuffType

std::ostream& operator<<(std::ostream& o, MachPhysPuffType type)
{
    switch (type)
    {
    case PUFF_1:
        o << "PUFF_1";
        break;
    case PUFF_2:
        o << "PUFF_2";
        break;
    case PUFF_3:
        o << "PUFF_3";
        break;
    case PUFF_4:
        o << "PUFF_4";
        break;
    default:
        o << static_cast<int>(type);
        break;
    }
    return o;
}

// MachPhysFireballType

std::ostream& operator<<(std::ostream& o, MachPhysFireballType type)
{
    switch (type)
    {
    case FIREBALL_1:
        o << "FIREBALL_1";
        break;
    case FIREBALL_2:
        o << "FIREBALL_2";
        break;
    case FIREBALL_3:
        o << "FIREBALL_3";
        break;
    case FIREBALL_4:
        o << "FIREBALL_4";
        break;
    default:
        o << static_cast<int>(type);
        break;
    }
    return o;
}
