#include "machgui/ConsoleCommands.hpp"

#include "machgui/Cameras.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/StartupScreens.hpp"
#include "machlog/actmaker.hpp"
#include "machlog/camera.hpp"
#include "machlog/cntrl.hpp"
#include "machlog/races.hpp"
#include "mathex/eulerang.hpp"
#include "mathex/point2d.hpp"
#include "mathex/point3d.hpp"
#include "mathex/radians.hpp"
#include "mathex/transf3d.hpp"
#include "system/IConsole.hpp"

#include <cmath>
#include <sstream>

namespace MachGui
{

namespace
{

using Arg = System::IConsole::ArgumentType;
using Request = System::IConsole::CommandRequest;
using Console = System::IConsole;

MachInGameScreen* getInGameScreen(MachGuiStartupScreens* pStartup, Console& console)
{
    if (!pStartup || pStartup->gameType() == MachGuiStartupScreens::NOGAME)
    {
        console.writeLine("No game is currently active.");
        return nullptr;
    }
    return &pStartup->inGameScreen();
}

std::string formatFloat(double v, int precision = 2)
{
    std::ostringstream oss;
    oss.precision(precision);
    oss << std::fixed << v;
    return oss.str();
}

MachPhys::Race parseRace(const std::string& name)
{
    if (name == "red")
        return MachPhys::RED;
    if (name == "blue")
        return MachPhys::BLUE;
    if (name == "green")
        return MachPhys::GREEN;
    if (name == "yellow")
        return MachPhys::YELLOW;
    return MachPhys::NORACE;
}

MachPhys::Race raceArgOrPlayer(const Request& request, std::size_t argIndex, Console& console)
{
    if (argIndex < request.arguments.size() && request.arguments[argIndex].provided)
    {
        const std::string& raceName = std::get<std::string>(request.arguments[argIndex].value);
        MachPhys::Race race = parseRace(raceName);
        if (race == MachPhys::NORACE)
        {
            console.writeLine("Unknown race: " + raceName + ". Use red, blue, green, or yellow.");
        }
        return race;
    }
    return MachLogRaces::instance().pcController().race();
}

// ============================================================
// Camera commands
// ============================================================

void camTypeCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return;

    MachCameras* pCameras = pScreen->cameras();

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        if (pCameras->isZenithCameraActive())
            console.writeLine("zenith");
        else if (pCameras->isGroundCameraActive())
            console.writeLine("ground");
        else if (pCameras->is1stPersonCameraActive())
            console.writeLine("1stperson");
        else
            console.writeLine("unknown");
        return;
    }

    const std::string& type = std::get<std::string>(request.arguments[0].value);
    if (type == "zenith")
        pCameras->useZenithCamera();
    else if (type == "ground")
        pCameras->useGroundCamera();
    else if (type == "1stperson")
        pCameras->use1stPersonCamera();
    else
        console.writeLine("Unknown camera type: " + type + ". Use zenith, ground, or 1stperson.");
}

void camPosCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return;

    MachCameras* pCameras = pScreen->cameras();

    if (request.arguments.size() < 2 || !request.arguments[0].provided)
    {
        if (pCameras->isZenithCameraActive())
        {
            const auto data = pCameras->zenithCameraData();
            console.writeLine(
                formatFloat(data.x) + " " + formatFloat(data.y) + " " + formatFloat(data.zoomDistance));
        }
        else if (pCameras->isGroundCameraActive())
        {
            const MexPoint3d pos = pCameras->currentCamera()->globalTransform().position();
            console.writeLine(
                formatFloat(pos.x()) + " " + formatFloat(pos.y()) + " " + formatFloat(pos.z()));
        }
        else
        {
            const MexPoint3d pos = pCameras->currentCamera()->globalTransform().position();
            console.writeLine(formatFloat(pos.x()) + " " + formatFloat(pos.y()) + " " + formatFloat(pos.z()));
        }
        return;
    }

    const double x = std::get<double>(request.arguments[0].value);
    const double y = std::get<double>(request.arguments[1].value);

    if (pCameras->isZenithCameraActive())
    {
        auto data = pCameras->zenithCameraData();
        data.x = x;
        data.y = y;
        if (request.arguments.size() > 2 && request.arguments[2].provided)
            data.zoomDistance = std::get<double>(request.arguments[2].value);
        pCameras->zenithCameraData(data);
    }
    else if (pCameras->isGroundCameraActive())
    {
        const MexPoint3d currentPos = pCameras->currentCamera()->globalTransform().position();
        const double z = (request.arguments.size() > 2 && request.arguments[2].provided)
            ? std::get<double>(request.arguments[2].value)
            : currentPos.z();
        pCameras->setGroundCameraPosition(x, y, z);
    }
    else
    {
        MexTransform3d xform = pCameras->currentCamera()->globalTransform();
        MexPoint3d pos = xform.position();
        pos.x(x);
        pos.y(y);
        if (request.arguments.size() > 2 && request.arguments[2].provided)
            pos.z(std::get<double>(request.arguments[2].value));
        xform.position(pos);
        pCameras->currentCamera()->globalTransform(xform);
    }
    console.writeLine("Camera position set.");
}

void camDirCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return;

    MachCameras* pCameras = pScreen->cameras();

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        if (pCameras->isZenithCameraActive())
        {
            const auto data = pCameras->zenithCameraData();
            const double yawDeg = data.heading.asScalar() * 180.0 / M_PI;
            console.writeLine(formatFloat(yawDeg));
        }
        else
        {
            MexEulerAngles angles;
            pCameras->currentCamera()->globalTransform().rotation(&angles);
            const double yawDeg = angles.azimuth().asScalar() * 180.0 / M_PI;
            const double pitchDeg = angles.elevation().asScalar() * 180.0 / M_PI;
            const double rollDeg = angles.roll().asScalar() * 180.0 / M_PI;
            if (pCameras->isGroundCameraActive())
                console.writeLine(formatFloat(yawDeg));
            else
                console.writeLine(formatFloat(yawDeg) + " " + formatFloat(pitchDeg) + " " + formatFloat(rollDeg));
        }
        return;
    }

    const double yawDeg = std::get<double>(request.arguments[0].value);
    const MexRadians yaw(yawDeg * M_PI / 180.0);

    if (pCameras->isZenithCameraActive())
    {
        auto data = pCameras->zenithCameraData();
        data.heading = yaw;
        pCameras->zenithCameraData(data);
    }
    else
    {
        MachLogCamera* pCamera = pCameras->currentCamera();
        MexEulerAngles angles;
        pCamera->globalTransform().rotation(&angles);
        angles.azimuth(yaw);
        if (request.arguments.size() > 1 && request.arguments[1].provided)
            angles.elevation(MexRadians(std::get<double>(request.arguments[1].value) * M_PI / 180.0));
        if (request.arguments.size() > 2 && request.arguments[2].provided)
            angles.roll(MexRadians(std::get<double>(request.arguments[2].value) * M_PI / 180.0));
        MexTransform3d newXform(angles, pCamera->globalTransform().position());
        pCamera->globalTransform(newXform);
    }
    console.writeLine("Camera direction set.");
}

void camLookatCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return;

    const double x = std::get<double>(request.arguments[0].value);
    const double y = std::get<double>(request.arguments[1].value);
    pScreen->cameras()->lookAt(MexPoint2d(x, y));
    console.writeLine("Camera looking at (" + formatFloat(x) + ", " + formatFloat(y) + ").");
}

// ============================================================
// Resource commands
// ============================================================

void giveBmuCommand(const Request& request, Console& console)
{
    const auto amount = static_cast<MachPhys::BuildingMaterialUnits>(std::get<std::int64_t>(request.arguments[0].value));
    MachPhys::Race race = raceArgOrPlayer(request, 1, console);
    if (race == MachPhys::NORACE)
        return;

    MachPhys::BuildingMaterialUnits added = MachLogRaces::instance().smartAddBMUs(race, amount);
    console.writeLine("Added " + std::to_string(added) + " BMUs.");
}

void setBmuCommand(const Request& request, Console& console)
{
    const auto amount = static_cast<MachPhys::BuildingMaterialUnits>(std::get<std::int64_t>(request.arguments[0].value));
    MachPhys::Race race = raceArgOrPlayer(request, 1, console);
    if (race == MachPhys::NORACE)
        return;

    MachLogRaces::instance().nBuildingMaterialUnits(race, amount);
    console.writeLine("BMUs set to " + std::to_string(amount) + ".");
}

void getBmuCommand(const Request& request, Console& console)
{
    MachPhys::Race race = raceArgOrPlayer(request, 0, console);
    if (race == MachPhys::NORACE)
        return;

    MachPhys::BuildingMaterialUnits bmu = MachLogRaces::instance().nBuildingMaterialUnits(race);
    console.writeLine("BMUs: " + std::to_string(bmu));
}

// ============================================================
// Spawn commands
// ============================================================

void printSpawnMachineUsage(Console& console)
{
    console.writeLine("Usage: spawn_machine <type> <subtype> <hwlevel> <x> <y> [rotation] [race]");
    console.writeLine("  Types: aggressor (grunt|assassin|knight|ballista|ninja),");
    console.writeLine("         constructor (dozer|builder|behemoth),");
    console.writeLine("         technician (lab_tech|tech_boy|brain_box),");
    console.writeLine("         administrator (boss|overseer|commander),");
    console.writeLine("         geo_locator, spy_locator, resource_carrier, apc");
}

void printSpawnBuildingUsage(Console& console)
{
    console.writeLine("Usage: spawn_building <type> <subtype> <hwlevel> <x> <y> [rotation] [race]");
    console.writeLine("  Types: factory (civilian|military), hardware_lab (civilian|military),");
    console.writeLine("         smelter, mine, garrison, beacon, pod,");
    console.writeLine("         missile_emplacement (turret|sentry|launcher|icbm)");
}

struct MachineTypeInfo
{
    MachLog::ObjectType type;
    int subType;
};

bool parseMachineType(const std::string& typeStr, const std::string& subTypeStr, MachineTypeInfo& info, Console& console)
{
    if (typeStr == "aggressor")
    {
        info.type = MachLog::AGGRESSOR;
        if (subTypeStr == "grunt")
            info.subType = MachPhys::GRUNT;
        else if (subTypeStr == "assassin")
            info.subType = MachPhys::ASSASSIN;
        else if (subTypeStr == "knight")
            info.subType = MachPhys::KNIGHT;
        else if (subTypeStr == "ballista")
            info.subType = MachPhys::BALLISTA;
        else if (subTypeStr == "ninja")
            info.subType = MachPhys::NINJA;
        else
        {
            console.writeLine("Unknown aggressor subtype: " + subTypeStr);
            return false;
        }
    }
    else if (typeStr == "constructor")
    {
        info.type = MachLog::CONSTRUCTOR;
        if (subTypeStr == "dozer")
            info.subType = MachPhys::DOZER;
        else if (subTypeStr == "builder")
            info.subType = MachPhys::BUILDER;
        else if (subTypeStr == "behemoth")
            info.subType = MachPhys::BEHEMOTH;
        else
        {
            console.writeLine("Unknown constructor subtype: " + subTypeStr);
            return false;
        }
    }
    else if (typeStr == "technician")
    {
        info.type = MachLog::TECHNICIAN;
        if (subTypeStr == "lab_tech")
            info.subType = MachPhys::LAB_TECH;
        else if (subTypeStr == "tech_boy")
            info.subType = MachPhys::TECH_BOY;
        else if (subTypeStr == "brain_box")
            info.subType = MachPhys::BRAIN_BOX;
        else
        {
            console.writeLine("Unknown technician subtype: " + subTypeStr);
            return false;
        }
    }
    else if (typeStr == "geo_locator")
    {
        info.type = MachLog::GEO_LOCATOR;
        info.subType = 0;
    }
    else if (typeStr == "spy_locator")
    {
        info.type = MachLog::SPY_LOCATOR;
        info.subType = 0;
    }
    else if (typeStr == "resource_carrier")
    {
        info.type = MachLog::RESOURCE_CARRIER;
        info.subType = 0;
    }
    else if (typeStr == "apc")
    {
        info.type = MachLog::APC;
        info.subType = 0;
    }
    else if (typeStr == "administrator")
    {
        info.type = MachLog::ADMINISTRATOR;
        if (subTypeStr == "boss")
            info.subType = MachPhys::BOSS;
        else if (subTypeStr == "overseer")
            info.subType = MachPhys::OVERSEER;
        else if (subTypeStr == "commander")
            info.subType = MachPhys::COMMANDER;
        else
        {
            console.writeLine("Unknown administrator subtype: " + subTypeStr);
            return false;
        }
    }
    else
    {
        console.writeLine("Unknown machine type: " + typeStr);
        return false;
    }
    return true;
}

void spawnMachineCommand(const Request& request, Console& console)
{
    const std::string& typeStr = std::get<std::string>(request.arguments[0].value);
    const std::string& subTypeStr = std::get<std::string>(request.arguments[1].value);
    const int hwLevel = static_cast<int>(std::get<std::int64_t>(request.arguments[2].value));
    const double x = std::get<double>(request.arguments[3].value);
    const double y = std::get<double>(request.arguments[4].value);

    MachineTypeInfo info{};
    if (!parseMachineType(typeStr, subTypeStr, info, console))
    {
        printSpawnMachineUsage(console);
        return;
    }

    double rotation = 0.0;
    if (request.arguments.size() > 5 && request.arguments[5].provided)
        rotation = std::get<double>(request.arguments[5].value);

    MachPhys::Race race = raceArgOrPlayer(request, 6, console);
    if (race == MachPhys::NORACE)
        return;

    const MexPoint3d location(x, y, 0);
    MachLogActorMaker::newLogMachine(
        info.type,
        info.subType,
        hwLevel,
        1,
        race,
        location,
        MachPhys::L_BOLTER);

    console.writeLine("Spawned " + typeStr + " " + subTypeStr + " at (" + formatFloat(x) + ", " + formatFloat(y) + ").");
}

struct BuildingTypeInfo
{
    MachLog::ObjectType type;
    int subType;
};

bool parseBuildingType(const std::string& typeStr, const std::string& subTypeStr, BuildingTypeInfo& info, Console& console)
{
    if (typeStr == "factory")
    {
        info.type = MachLog::FACTORY;
        if (subTypeStr == "civilian")
            info.subType = MachPhys::CIVILIAN;
        else if (subTypeStr == "military")
            info.subType = MachPhys::MILITARY;
        else
        {
            console.writeLine("Unknown factory subtype: " + subTypeStr);
            return false;
        }
    }
    else if (typeStr == "hardware_lab")
    {
        info.type = MachLog::HARDWARE_LAB;
        if (subTypeStr == "civilian")
            info.subType = MachPhys::LAB_CIVILIAN;
        else if (subTypeStr == "military")
            info.subType = MachPhys::LAB_MILITARY;
        else
        {
            console.writeLine("Unknown hardware_lab subtype: " + subTypeStr);
            return false;
        }
    }
    else if (typeStr == "smelter")
    {
        info.type = MachLog::SMELTER;
        info.subType = 0;
    }
    else if (typeStr == "mine")
    {
        info.type = MachLog::MINE;
        info.subType = 0;
    }
    else if (typeStr == "garrison")
    {
        info.type = MachLog::GARRISON;
        info.subType = 0;
    }
    else if (typeStr == "beacon")
    {
        info.type = MachLog::BEACON;
        info.subType = 0;
    }
    else if (typeStr == "pod")
    {
        info.type = MachLog::POD;
        info.subType = 0;
    }
    else if (typeStr == "missile_emplacement")
    {
        info.type = MachLog::MISSILE_EMPLACEMENT;
        if (subTypeStr == "turret")
            info.subType = MachPhys::TURRET;
        else if (subTypeStr == "sentry")
            info.subType = MachPhys::SENTRY;
        else if (subTypeStr == "launcher")
            info.subType = MachPhys::LAUNCHER;
        else if (subTypeStr == "icbm")
            info.subType = MachPhys::ICBM;
        else
        {
            console.writeLine("Unknown missile_emplacement subtype: " + subTypeStr);
            return false;
        }
    }
    else
    {
        console.writeLine("Unknown building type: " + typeStr);
        return false;
    }
    return true;
}

void spawnBuildingCommand(const Request& request, Console& console)
{
    const std::string& typeStr = std::get<std::string>(request.arguments[0].value);
    const std::string& subTypeStr = std::get<std::string>(request.arguments[1].value);
    const int hwLevel = static_cast<int>(std::get<std::int64_t>(request.arguments[2].value));
    const double x = std::get<double>(request.arguments[3].value);
    const double y = std::get<double>(request.arguments[4].value);

    BuildingTypeInfo info{};
    if (!parseBuildingType(typeStr, subTypeStr, info, console))
    {
        printSpawnBuildingUsage(console);
        return;
    }

    double rotationDeg = 0.0;
    if (request.arguments.size() > 5 && request.arguments[5].provided)
        rotationDeg = std::get<double>(request.arguments[5].value);

    MachPhys::Race race = raceArgOrPlayer(request, 6, console);
    if (race == MachPhys::NORACE)
        return;

    const MexPoint3d location(x, y, 0);
    const MexRadians angle(rotationDeg * M_PI / 180.0);
    MachLogActorMaker::newLogConstruction(info.type, info.subType, hwLevel, location, angle, race);

    console.writeLine(
        "Spawned " + typeStr + " " + subTypeStr + " at (" + formatFloat(x) + ", " + formatFloat(y) + ").");
}

} // anonymous namespace

void registerConsoleCommands(System::IConsole& console, MachGuiStartupScreens* pStartup)
{
    // ---- Camera commands ----

    console.registerCommand(
        {
            "cam_type",
            "Get/set camera type (zenith, ground, 1stperson).",
            { { "type", Arg::Identifier, true, "Camera type." } },
        },
        [pStartup](const Request& request, Console& console) { camTypeCommand(pStartup, request, console); });

    console.registerCommand(
        {
            "cam_pos",
            "Get/set camera position. Zenith: x y [zoom]. Ground: x y [z]. 1stperson: x y [z].",
            {
                { "x", Arg::Float, true, "X coordinate." },
                { "y", Arg::Float, true, "Y coordinate." },
                { "z", Arg::Float, true, "Z or zoom distance." },
            },
        },
        [pStartup](const Request& request, Console& console) { camPosCommand(pStartup, request, console); });

    console.registerCommand(
        {
            "cam_dir",
            "Get/set camera direction in degrees. Zenith/ground: yaw. 1stperson: yaw [pitch] [roll].",
            {
                { "yaw", Arg::Float, true, "Yaw in degrees." },
                { "pitch", Arg::Float, true, "Pitch in degrees (1stperson only)." },
                { "roll", Arg::Float, true, "Roll in degrees (1stperson only)." },
            },
        },
        [pStartup](const Request& request, Console& console) { camDirCommand(pStartup, request, console); });

    console.registerCommand(
        {
            "cam_lookat",
            "Point camera at a world XY position.",
            {
                { "x", Arg::Float, false, "X coordinate." },
                { "y", Arg::Float, false, "Y coordinate." },
            },
        },
        [pStartup](const Request& request, Console& console) { camLookatCommand(pStartup, request, console); });

    // ---- Resource commands ----

    console.registerCommand(
        {
            "give_bmu",
            "Add BMUs to a race. Defaults to player race.",
            {
                { "amount", Arg::Integer, false, "Amount of BMUs to add." },
                { "race", Arg::Identifier, true, "Race: red, blue, green, yellow." },
            },
        },
        [](const Request& request, Console& console) { giveBmuCommand(request, console); });

    console.registerCommand(
        {
            "set_bmu",
            "Set BMUs to an exact value. Defaults to player race.",
            {
                { "amount", Arg::Integer, false, "BMU amount." },
                { "race", Arg::Identifier, true, "Race: red, blue, green, yellow." },
            },
        },
        [](const Request& request, Console& console) { setBmuCommand(request, console); });

    console.registerCommand(
        {
            "get_bmu",
            "Print current BMU count. Defaults to player race.",
            { { "race", Arg::Identifier, true, "Race: red, blue, green, yellow." } },
        },
        [](const Request& request, Console& console) { getBmuCommand(request, console); });

    // ---- Spawn commands ----

    console.registerCommand(
        {
            "spawn_machine",
            "Spawn a machine at a position.",
            {
                { "type", Arg::Identifier, false, "Machine type (aggressor, constructor, etc.)." },
                { "subtype", Arg::Identifier, false, "Subtype (grunt, dozer, etc.)." },
                { "hwlevel", Arg::Integer, false, "Hardware level." },
                { "x", Arg::Float, false, "X position." },
                { "y", Arg::Float, false, "Y position." },
                { "rotation", Arg::Float, true, "Rotation in degrees." },
                { "race", Arg::Identifier, true, "Race: red, blue, green, yellow." },
            },
        },
        [](const Request& request, Console& console) { spawnMachineCommand(request, console); });

    console.registerCommand(
        {
            "spawn_building",
            "Spawn a building at a position.",
            {
                { "type", Arg::Identifier, false, "Building type (factory, smelter, etc.)." },
                { "subtype", Arg::Identifier, false, "Subtype (civilian, military, etc.)." },
                { "hwlevel", Arg::Integer, false, "Hardware level." },
                { "x", Arg::Float, false, "X position." },
                { "y", Arg::Float, false, "Y position." },
                { "rotation", Arg::Float, true, "Rotation in degrees." },
                { "race", Arg::Identifier, true, "Race: red, blue, green, yellow." },
            },
        },
        [](const Request& request, Console& console) { spawnBuildingCommand(request, console); });
}

} // namespace MachGui
