/*
 * M A C H L O G . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "machlog/machlog.hpp"

#include "utility/string.hpp"

#include <fstream>
#include <type_traits>

// ObjectType

std::string_view MachLog::toString(ObjectType type)
{
    switch (type)
    {
    case ADMINISTRATOR:
        return "ADMINISTRATOR";
    case AGGRESSOR:
        return "AGGRESSOR";
    case CONSTRUCTOR:
        return "CONSTRUCTOR";
    case GEO_LOCATOR:
        return "GEO_LOCATOR";
    case SPY_LOCATOR:
        return "SPY_LOCATOR";
    case TECHNICIAN:
        return "TECHNICIAN";
    case RESOURCE_CARRIER:
        return "RESOURCE_CARRIER";
    case APC:
        return "APC";
    case BEACON:
        return "BEACON";
    case FACTORY:
        return "FACTORY";
    case GARRISON:
        return "GARRISON";
    case HARDWARE_LAB:
        return "HARDWARE_LAB";
    case ORE_HOLOGRAPH:
        return "ORE_HOLOGRAPH";
    case POD:
        return "POD";
    case MINE:
        return "MINE";
    case MISSILE_EMPLACEMENT:
        return "MISSILE_EMPLACEMENT";
    case SOFTWARE_LAB:
        return "SOFTWARE_LAB";
    case SMELTER:
        return "SMELTER";
    case DEBRIS:
        return "DEBRIS";
    case SQUADRON:
        return "SQUADRON";
    case LAND_MINE:
        return "LAND_MINE";
    case ARTEFACT:
        return "ARTEFACT";
    case N_OBJECT_TYPES:
        break;
    }
    return {};
}

std::optional<MachLog::ObjectType> MachLog::toObjectType(std::string_view str)
{
    using Underlying = std::underlying_type_t<ObjectType>;

    for (Underlying i = 0; i < N_OBJECT_TYPES; ++i)
    {
        ObjectType v = static_cast<ObjectType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::ObjectType& t)
{
    return o << MachLog::toString(t);
}

// DefCon

std::string_view MachLog::toString(DefCon type)
{
    switch (type)
    {
    case DEFCON_HIGH:
        return "DEFCON_HIGH";
    case DEFCON_NORMAL:
        return "DEFCON_NORMAL";
    case DEFCON_LOW:
        return "DEFCON_LOW";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::DefCon& t)
{
    return o << MachLog::toString(t);
}

// TargetSystemType

std::string_view MachLog::toString(TargetSystemType type)
{
    switch (type)
    {
    case TARGET_NORMAL:
        return "TARGET_NORMAL";
    case TARGET_RESEARCH:
        return "TARGET_RESEARCH";
    case TARGET_RESOURCE:
        return "TARGET_RESOURCE";
    case TARGET_OBJECT:
        return "TARGET_OBJECT";
    case TARGET_AGGRESSIVES:
        return "TARGET_AGGRESSIVES";
    case FAVOUR_STATIC_TARGETS:
        return "FAVOUR_STATIC_TARGETS";
    case TARGET_AIR_UNITS:
        return "TARGET_AIR_UNITS";
    case DONT_TARGET_AIR_UNITS:
        return "DONT_TARGET_AIR_UNITS";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::TargetSystemType& t)
{
    return o << MachLog::toString(t);
}

// BeaconType

std::string_view MachLog::toString(BeaconType type)
{
    switch (type)
    {
    case NO_BEACON:
        return "NO_BEACON";
    case LEVEL_1_BEACON:
        return "LEVEL_1_BEACON";
    case LEVEL_3_BEACON:
        return "LEVEL_3_BEACON";
    }
    return {};
}

std::optional<MachLog::BeaconType> MachLog::toBeaconType(std::string_view str)
{
    using Underlying = std::underlying_type_t<BeaconType>;

    for (Underlying i = 0; i <= LEVEL_3_BEACON; ++i)
    {
        BeaconType v = static_cast<BeaconType>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::BeaconType& t)
{
    return o << MachLog::toString(t);
}

// SelectableType

std::string_view MachLog::toString(SelectableType type)
{
    switch (type)
    {
    case NOT_SELECTABLE:
        return "NOT_SELECTABLE";
    case FULLY_SELECTABLE:
        return "FULLY_SELECTABLE";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::SelectableType& t)
{
    return o << MachLog::toString(t);
}

// RandomStarts

std::string_view MachLog::toString(RandomStarts type)
{
    switch (type)
    {
    case RANDOM_START_LOCATIONS:
        return "RANDOM_START_LOCATIONS";
    case FIXED_START_LOCATIONS:
        return "FIXED_START_LOCATIONS";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::RandomStarts& t)
{
    return o << MachLog::toString(t);
}

// PlayerType

std::string_view MachLog::toString(PlayerType type)
{
    switch (type)
    {
    case PC_LOCAL:
        return "PC_LOCAL";
    case PC_REMOTE:
        return "PC_REMOTE";
    case AI_LOCAL:
        return "AI_LOCAL";
    case AI_REMOTE:
        return "AI_REMOTE";
    case NOT_DEFINED:
        return "NOT_DEFINED";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::PlayerType& t)
{
    return o << MachLog::toString(t);
}

// ResourcesAvailable

std::string_view MachLog::toString(ResourcesAvailable type)
{
    switch (type)
    {
    case RES_DEFAULT:
        return "RES_DEFAULT";
    case RES_LOW:
        return "RES_LOW";
    case RES_MEDIUM:
        return "RES_MEDIUM";
    case RES_HIGH:
        return "RES_HIGH";
    }
    return {};
}

std::optional<MachLog::ResourcesAvailable> MachLog::toResourcesAvailable(std::string_view str)
{
    using Underlying = std::underlying_type_t<ResourcesAvailable>;

    for (Underlying i = 0; i <= RES_HIGH; ++i)
    {
        ResourcesAvailable v = static_cast<ResourcesAvailable>(i);
        if (Utils::caseInsensitiveEqual(toString(v), str))
            return v;
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::ResourcesAvailable& t)
{
    return o << MachLog::toString(t);
}

// StartingResources

std::string_view MachLog::toString(StartingResources type)
{
    switch (type)
    {
    case STARTING_RESOURCES_DEFAULT:
        return "STARTING_RESOURCES_DEFAULT";
    case STARTING_RESOURCES_RES_LOW:
        return "STARTING_RESOURCES_RES_LOW";
    case STARTING_RESOURCES_MEDIUM:
        return "STARTING_RESOURCES_MEDIUM";
    case STARTING_RESOURCES_HIGH:
        return "STARTING_RESOURCES_HIGH";
    case STARTING_RESOURCES_VERY_HIGH:
        return "STARTING_RESOURCES_VERY_HIGH";
    case STARTING_RESOURCES_SUPER_HIGH:
        return "STARTING_RESOURCES_SUPER_HIGH";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::StartingResources& t)
{
    return o << MachLog::toString(t);
}

// TechnologyLevel

std::string_view MachLog::toString(TechnologyLevel type)
{
    switch (type)
    {
    case TECH_LEVEL_DEFAULT:
        return "TECH_LEVEL_DEFAULT";
    case TECH_LEVEL_LOW:
        return "TECH_LEVEL_LOW";
    case TECH_LEVEL_MEDIUM:
        return "TECH_LEVEL_MEDIUM";
    case TECH_LEVEL_HIGH:
        return "TECH_LEVEL_HIGH";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::TechnologyLevel& t)
{
    return o << MachLog::toString(t);
}

// VictoryCondition

std::string_view MachLog::toString(VictoryCondition type)
{
    switch (type)
    {
    case VICTORY_DEFAULT:
        return "VICTORY_DEFAULT";
    case VICTORY_ANNIHILATION:
        return "VICTORY_ANNIHILATION";
    case VICTORY_POD:
        return "VICTORY_POD";
    case VICTORY_TIMER:
        return "VICTORY_TIMER";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::VictoryCondition& t)
{
    return o << MachLog::toString(t);
}

// GameType

std::string_view MachLog::toString(GameType type)
{
    switch (type)
    {
    case CAMPAIGN_SINGLE_PLAYER:
        return "CAMPAIGN_SINGLE_PLAYER";
    case SKIRMISH_SINGLE_PLAYER:
        return "SKIRMISH_SINGLE_PLAYER";
    case MULTIPLAYER:
        return "MULTIPLAYER";
    }
    return {};
}

std::ostream& operator<<(std::ostream& o, const MachLog::GameType& t)
{
    return o << MachLog::toString(t);
}
/* End MACHLOG.CPP ***************************************************/
