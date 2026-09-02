#include "machgui/ConsoleCommands.hpp"

#include "machgui/Cameras.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/LoadSaveGameExtras.hpp"
#include "machgui/SaveGames.hpp"
#include "machgui/StartupData.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/db/Database.hpp"
#include "machgui/db/DbPlayer.hpp"
#include "machgui/db/DbSavedGame.hpp"
#include "base/IProgressReporter.hpp"
#include "gui/Manager.hpp"
#include "gui/Screenshots.hpp"
#include "ctl/Vector.hpp"
#include "machlog/Actors/ActorMaker.hpp"
#include "machlog/World/Mapper.hpp"
#include "machlog/World/Camera.hpp"
#include "machlog/Controllers/Controller.hpp"
#include "machlog/Actors/Construction.hpp"
#include "machlog/World/GameCreationData.hpp"
#include "machlog/Actors/Machine.hpp"
#include "machlog/Operations/MoveOperation.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Races.hpp"
#include "machlog/World/Scenario.hpp"
#include "machlog/World/SpacialManipulation.hpp"
#include "machphys/Machines/Machine.hpp"
#include "machphys/Weapons/LegalWeaponCombos.hpp"
#include "machphys/Data/Internal/DataImplementation.hpp"
#include "machphys/Data/Levels.hpp"
#include "mathex/Degrees.hpp"
#include "mathex/EulerAngles.hpp"
#include "mathex/Point2d.hpp"
#include "mathex/Point3d.hpp"
#include "mathex/Radians.hpp"
#include "mathex/Transform3d.hpp"
#include "render/Device.hpp"
#include "sim/Manager.hpp"
#include "system/ConfigVariables.hpp"
#include "system/IConsole.hpp"
#include "system/PathName.hpp"
#include "system/VFS.hpp"
#include "utility/String.hpp"
#include "world4d/Scene/SceneManager.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <optional>
#include <sstream>

namespace MachGui
{

namespace ConsoleImpl
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
        console.reportError("No game is currently active.");
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

std::string formatCoordinates(double x, double y)
{
    return formatFloat(x) + "," + formatFloat(y);
}

std::optional<MexPoint2d> parseCoordinates(const std::string& str)
{
    const std::size_t comma = str.find(',');
    if (comma == std::string::npos || comma == 0 || comma == str.size() - 1)
        return std::nullopt;

    try
    {
        const double x = std::stod(str.substr(0, comma));
        const double y = std::stod(str.substr(comma + 1));
        return MexPoint2d(x, y);
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

std::string toOnOffString(bool value)
{
    return value ? "on" : "off";
}

// The race named, or the player's own where none was. MachPhys::NORACE says the
// name could not be used, and why has already been printed.
MachPhys::Race raceOrPlayer(const std::optional<std::string>& raceName, Console& console)
{
    std::optional<MachPhys::Race> race;
    if (raceName.has_value())
    {
        race = MachPhys::toRace(raceName.value());
        if (!race.has_value())
        {
            console.reportError("Unknown race: " + raceName.value() + ". Use red, blue, green, or yellow.");
            return MachPhys::NORACE;
        }

        // A race the game was not set up with has no controller, no ore, and
        // nothing for an actor to be added to, so spawning into it walks off the
        // end of what does exist and takes the process with it. Refusing says
        // which of the four are actually there, which is the thing the caller
        // needs to know and cannot otherwise find out.
        if (!MachLogRaces::instance().raceInGame(race.value()))
        {
            std::vector<std::string> inGame;
            for (MachPhys::Race colour : MachPhys::AllRaces)
            {
                if (MachLogRaces::instance().raceInGame(colour))
                    inGame.emplace_back(MachPhys::toString(colour));
            }

            console.reportError(
                "Race " + raceName.value() + " is not in this game. In game: "
                + (inGame.empty() ? std::string("none") : Utils::join(inGame, ", ")));
            return MachPhys::NORACE;
        }
    }

    return race.value_or(MachLogRaces::instance().pcController().race());
}

MachPhys::Race raceArgOrPlayer(const Request& request, std::size_t argIndex, Console& console)
{
    std::optional<std::string> raceName;
    if (argIndex < request.arguments.size() && request.arguments[argIndex].provided)
        raceName = std::get<std::string>(request.arguments[argIndex].value);

    return raceOrPlayer(raceName, console);
}

constexpr MachLog::ObjectType AllMachineObjectTypes[] = {
    MachLog::ADMINISTRATOR,
    MachLog::AGGRESSOR,
    MachLog::APC,
    MachLog::CONSTRUCTOR,
    MachLog::GEO_LOCATOR,
    MachLog::RESOURCE_CARRIER,
    MachLog::SPY_LOCATOR,
    MachLog::TECHNICIAN,
};

constexpr MachLog::ObjectType AllConstructionObjectTypes[] = {
    MachLog::BEACON,
    MachLog::FACTORY,
    MachLog::GARRISON,
    MachLog::HARDWARE_LAB,
    MachLog::MINE,
    MachLog::MISSILE_EMPLACEMENT,
    MachLog::POD,
    MachLog::SMELTER,
};

template <std::size_t N>
bool contains(const MachLog::ObjectType (&arr)[N], MachLog::ObjectType type)
{
    return std::find(std::begin(arr), std::end(arr), type) != std::end(arr);
}

bool isMachineObjectType(MachLog::ObjectType type)
{
    return contains(AllMachineObjectTypes, type);
}

bool isConstructionObjectType(MachLog::ObjectType type)
{
    return contains(AllConstructionObjectTypes, type);
}

bool objectTypeHasSubType(MachLog::ObjectType type)
{
    switch (type)
    {
    case MachLog::ADMINISTRATOR:
    case MachLog::AGGRESSOR:
    case MachLog::CONSTRUCTOR:
    case MachLog::TECHNICIAN:
    case MachLog::FACTORY:
    case MachLog::HARDWARE_LAB:
    case MachLog::MISSILE_EMPLACEMENT:
        return true;
    default:
        return false;
    }
}

// ============================================================
// Spawn validation helpers
// ============================================================

// Build a comma-separated string of valid hardware levels for the given machine type/subtype.
std::string validMachineLevelsString(MachLog::ObjectType objType, int subType)
{
    const auto& levels = MachPhysLevels::instance();
    const MachPhys::MachineType machType = MachLogMapper::mapToPhysMachine(objType);
    std::string result;

    auto appendLevel = [&](size_t level)
    {
        if (!result.empty())
            result += ", ";
        result += std::to_string(level);
    };

    if (!objectTypeHasSubType(objType))
    {
        for (size_t i = 0; i < levels.nHardwareLevels(machType); ++i)
            appendLevel(levels.hardwareLevel(machType, i));
    }
    else
    {
        switch (objType)
        {
        case MachLog::AGGRESSOR:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::AggressorSubType>(subType)); ++i)
                appendLevel(levels.hardwareLevel(static_cast<MachPhys::AggressorSubType>(subType), i));
            break;
        case MachLog::ADMINISTRATOR:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::AdministratorSubType>(subType)); ++i)
                appendLevel(levels.hardwareLevel(static_cast<MachPhys::AdministratorSubType>(subType), i));
            break;
        case MachLog::CONSTRUCTOR:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::ConstructorSubType>(subType)); ++i)
                appendLevel(levels.hardwareLevel(static_cast<MachPhys::ConstructorSubType>(subType), i));
            break;
        case MachLog::TECHNICIAN:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::TechnicianSubType>(subType)); ++i)
                appendLevel(levels.hardwareLevel(static_cast<MachPhys::TechnicianSubType>(subType), i));
            break;
        default:
            break;
        }
    }
    return result;
}

bool isMachineLevelValid(MachLog::ObjectType objType, int subType, size_t hwLevel)
{
    const auto& levels = MachPhysLevels::instance();
    const MachPhys::MachineType machType = MachLogMapper::mapToPhysMachine(objType);

    if (!objectTypeHasSubType(objType))
        return levels.levelValid(machType, hwLevel);

    switch (objType)
    {
    case MachLog::AGGRESSOR:
        return levels.levelValid(static_cast<MachPhys::AggressorSubType>(subType), hwLevel);
    case MachLog::ADMINISTRATOR:
        return levels.levelValid(static_cast<MachPhys::AdministratorSubType>(subType), hwLevel);
    case MachLog::CONSTRUCTOR:
        return levels.levelValid(static_cast<MachPhys::ConstructorSubType>(subType), hwLevel);
    case MachLog::TECHNICIAN:
        return levels.levelValid(static_cast<MachPhys::TechnicianSubType>(subType), hwLevel);
    default:
        return false;
    }
}

// Build a comma-separated string of valid hardware levels for the given construction type/subtype.
std::string validConstructionLevelsString(MachLog::ObjectType objType, int subType)
{
    const auto& levels = MachPhysLevels::instance();
    const MachPhys::ConstructionType conType = MachLogMapper::mapToPhysConstruction(objType);
    std::string result;

    auto appendLevel = [&](size_t level)
    {
        if (!result.empty())
            result += ", ";
        result += std::to_string(level);
    };

    if (!objectTypeHasSubType(objType))
    {
        for (size_t i = 0; i < levels.nHardwareLevels(conType); ++i)
            appendLevel(levels.hardwareLevel(conType, i));
    }
    else
    {
        switch (objType)
        {
        case MachLog::FACTORY:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::FactorySubType>(subType)); ++i)
                appendLevel(levels.hardwareLevel(static_cast<MachPhys::FactorySubType>(subType), i));
            break;
        case MachLog::HARDWARE_LAB:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::HardwareLabSubType>(subType)); ++i)
                appendLevel(levels.hardwareLevel(static_cast<MachPhys::HardwareLabSubType>(subType), i));
            break;
        case MachLog::MISSILE_EMPLACEMENT:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::MissileEmplacementSubType>(subType));
                 ++i)
                appendLevel(levels.hardwareLevel(static_cast<MachPhys::MissileEmplacementSubType>(subType), i));
            break;
        default:
            break;
        }
    }
    return result;
}

bool isConstructionLevelValid(MachLog::ObjectType objType, int subType, size_t hwLevel)
{
    const auto& levels = MachPhysLevels::instance();
    const MachPhys::ConstructionType conType = MachLogMapper::mapToPhysConstruction(objType);

    if (!objectTypeHasSubType(objType))
        return levels.levelValid(conType, hwLevel);

    switch (objType)
    {
    case MachLog::FACTORY:
        return levels.levelValid(static_cast<MachPhys::FactorySubType>(subType), hwLevel);
    case MachLog::HARDWARE_LAB:
        return levels.levelValid(static_cast<MachPhys::HardwareLabSubType>(subType), hwLevel);
    case MachLog::MISSILE_EMPLACEMENT:
        return levels.levelValid(static_cast<MachPhys::MissileEmplacementSubType>(subType), hwLevel);
    default:
        return false;
    }
}

// Return the list of legal weapon combos for the given machine type/subtype/hwLevel.
MachPhysLegalWeaponCombos::LegalWeaponCombos legalWeaponCombosFor(
    MachLog::ObjectType objType, int subType, size_t hwLevel)
{
    const auto& combos = MachPhysLegalWeaponCombos::instance();

    if (!objectTypeHasSubType(objType))
        return combos.legalWeaponCombos(MachLogMapper::mapToPhysMachine(objType), hwLevel);

    switch (objType)
    {
    case MachLog::AGGRESSOR:
        return combos.legalWeaponCombos(static_cast<MachPhys::AggressorSubType>(subType), hwLevel);
    case MachLog::ADMINISTRATOR:
        return combos.legalWeaponCombos(static_cast<MachPhys::AdministratorSubType>(subType), hwLevel);
    case MachLog::CONSTRUCTOR:
        return combos.legalWeaponCombos(static_cast<MachPhys::ConstructorSubType>(subType), hwLevel);
    case MachLog::TECHNICIAN:
        return combos.legalWeaponCombos(static_cast<MachPhys::TechnicianSubType>(subType), hwLevel);
    default:
        return {};
    }
}

// Return comma-separated list of valid weapon combos for the given machine type/subtype/hwLevel.
std::string validWeaponCombosString(MachLog::ObjectType objType, int subType, size_t hwLevel)
{
    const auto legal = legalWeaponCombosFor(objType, subType, hwLevel);
    std::string result;
    for (const auto& c : legal)
    {
        if (!result.empty())
            result += ", ";
        result += MachPhys::toString(c);
    }
    return result;
}

bool isWeaponComboValid(MachLog::ObjectType objType, int subType, size_t hwLevel, MachPhys::WeaponCombo combo)
{
    const auto legal = legalWeaponCombosFor(objType, subType, hwLevel);
    for (const auto& c : legal)
    {
        if (c == combo)
            return true;
    }
    return false;
}

static void setDevModeCommand(const System::IConsole::CommandRequest& request, System::IConsole& console)
{
    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        const bool enabled = console.devModeEnabled();
        console.writeLine(std::string("Developer mode is ") + (enabled ? "enabled." : "disabled."));
        return;
    }

    const bool enable = std::get<bool>(request.arguments[0].value);
    Config::devMode.set(enable);

    console.setDevModeEnabled(Config::devMode.get());
    console.writeLine(std::string("Developer mode ") + (enable ? "enabled." : "disabled."));
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
        console.reportError("Unknown camera type: " + type + ". Use zenith, ground, or 1stperson.");
}

// Where the camera is: x and y on the map, and a third value that is the
// camera's own. The zenith camera reports the terrain point in the middle of the
// screen and how far above it the camera sits; every other camera reports where
// it stands, so the third value is its height.
MexPoint3d cameraPosition(MachCameras* pCameras)
{
    if (pCameras->isZenithCameraActive())
    {
        const MachCameras::ZenithCameraData data = pCameras->zenithCameraData();
        return MexPoint3d(data.x, data.y, data.zoomDistance);
    }

    return pCameras->currentCamera()->globalTransform().position();
}

void camPosCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return;

    MachCameras* pCameras = pScreen->cameras();

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        const MexPoint3d position = cameraPosition(pCameras);
        console.writeLine(formatCoordinates(position.x(), position.y()) + " " + formatFloat(position.z()));
        return;
    }

    const std::string& posStr = std::get<std::string>(request.arguments[0].value);
    const std::optional<MexPoint2d> coords = parseCoordinates(posStr);
    if (!coords.has_value())
    {
        console.writeLine("Invalid position format. Use x,y (e.g. 123.4,567.8).");
        return;
    }

    const double x = coords->x();
    const double y = coords->y();

    if (pCameras->isZenithCameraActive())
    {
        auto data = pCameras->zenithCameraData();
        data.x = x;
        data.y = y;
        if (request.arguments.size() > 1 && request.arguments[1].provided)
            data.zoomDistance = std::get<double>(request.arguments[1].value);
        pCameras->zenithCameraData(data);
    }
    else if (pCameras->isGroundCameraActive())
    {
        const MexPoint3d currentPos = pCameras->currentCamera()->globalTransform().position();
        const double z = (request.arguments.size() > 1 && request.arguments[1].provided)
            ? std::get<double>(request.arguments[1].value)
            : currentPos.z();
        pCameras->setGroundCameraPosition(x, y, z);
    }
    else
    {
        MexTransform3d xform = pCameras->currentCamera()->globalTransform();
        MexPoint3d pos = xform.position();
        pos.x(x);
        pos.y(y);
        if (request.arguments.size() > 1 && request.arguments[1].provided)
            pos.z(std::get<double>(request.arguments[1].value));
        xform.position(pos);
        pCameras->currentCamera()->globalTransform(xform);
    }
    console.writeLine("Camera position set.");
}

void fovCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return;

    W4dCamera* pCamera = pScreen->cameras()->currentCamera();

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        const double verticalDeg = pCamera->verticalFOVAngle() * 180.0 / Mathex::PI;
        const double horizontalDeg = pCamera->horizontalFOVAngle() * 180.0 / Mathex::PI;

        std::string message("Field of view is " + formatFloat(verticalDeg) + " degrees vertical, ");
        message += formatFloat(horizontalDeg) + " horizontal.";
        console.writeLine(message);
        return;
    }

    const double verticalDeg = std::get<double>(request.arguments[0].value);

    // A rectilinear projection has no ninety: the frustum's half-width is the
    // tangent of half the angle, which runs away to nothing usable long before
    // it. The ceiling is where the stretch at the edges stops being worth the
    // width, not where the arithmetic fails.
    if (verticalDeg < 10.0 || verticalDeg > 140.0)
    {
        console.reportError("Expected 10 to 140 degrees.");
        return;
    }

    pCamera->setVerticalFOVAngle(verticalDeg * Mathex::PI / 180.0);

    const double horizontalDeg = pCamera->horizontalFOVAngle() * 180.0 / Mathex::PI;
    std::string message("Field of view set to " + formatFloat(verticalDeg) + " degrees vertical, ");
    message += formatFloat(horizontalDeg) + " horizontal.";
    console.writeLine(message);

    // Set on the camera standing, and the other types keep their own, so
    // switching away and back returns to this one still widened. What ends it
    // is a game starting: MachCameras::loadGame() builds all four again, each
    // with the angle its role wants. Worth knowing before a take is lost to it.
    console.writeLine("Set on this camera only; every camera goes back to its own angle when a game starts.");
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
            const double yawDeg = data.heading.asScalar() * 180.0 / Mathex::PI;
            console.writeLine(formatFloat(yawDeg));
        }
        else
        {
            MexEulerAngles angles;
            pCameras->currentCamera()->globalTransform().rotation(&angles);
            const double yawDeg = angles.azimuth().asScalar() * 180.0 / Mathex::PI;
            const double pitchDeg = angles.elevation().asScalar() * 180.0 / Mathex::PI;
            const double rollDeg = angles.roll().asScalar() * 180.0 / Mathex::PI;
            if (pCameras->isGroundCameraActive())
                console.writeLine(formatFloat(yawDeg));
            else
                console.writeLine(formatFloat(yawDeg) + " " + formatFloat(pitchDeg) + " " + formatFloat(rollDeg));
        }
        return;
    }

    const double yawDeg = std::get<double>(request.arguments[0].value);
    const MexRadians yaw(yawDeg * Mathex::PI / 180.0);

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
            angles.elevation(MexRadians(std::get<double>(request.arguments[1].value) * Mathex::PI / 180.0));
        if (request.arguments.size() > 2 && request.arguments[2].provided)
            angles.roll(MexRadians(std::get<double>(request.arguments[2].value) * Mathex::PI / 180.0));
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

    const std::string& posStr = std::get<std::string>(request.arguments[0].value);
    const std::optional<MexPoint2d> coords = parseCoordinates(posStr);
    if (!coords.has_value())
    {
        console.writeLine("Invalid position format. Use x,y (e.g. 123.4,567.8).");
        return;
    }

    pScreen->cameras()->lookAt(coords.value());
    console.writeLine("Camera looking at " + formatCoordinates(coords->x(), coords->y()) + ".");
}

// ============================================================
// Fog of war command
// ============================================================

void fowCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return;

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        console.writeLine(std::string("Fog of war is ") + toOnOffString(pScreen->fogOfWarOn()) + ".");
        return;
    }

    const bool enabled = std::get<bool>(request.arguments[0].value);
    pScreen->fogOfWarOn(enabled);
    console.writeLine(std::string("Fog of war ") + toOnOffString(enabled) + ".");
}

void uiCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return;

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        console.writeLine(std::string("The interface is ") + toOnOffString(pScreen->isUiVisible()) + ".");
        return;
    }

    const bool visible = std::get<bool>(request.arguments[0].value);
    pScreen->setUiVisible(visible);
    console.writeLine(std::string("The interface is ") + toOnOffString(visible) + ".");
}

// ============================================================
// Resource commands
// ============================================================

void giveBmuCommand(const Request& request, Console& console)
{
    const auto amount
        = static_cast<MachPhys::BuildingMaterialUnits>(std::get<std::int64_t>(request.arguments[0].value));
    MachPhys::Race race = raceArgOrPlayer(request, 1, console);
    if (race == MachPhys::NORACE)
        return;

    MachPhys::BuildingMaterialUnits added = MachLogRaces::instance().smartAddBMUs(race, amount);
    console.writeLine("Added " + std::to_string(added) + " BMUs.");
}

void setBmuCommand(const Request& request, Console& console)
{
    const auto amount
        = static_cast<MachPhys::BuildingMaterialUnits>(std::get<std::int64_t>(request.arguments[0].value));
    MachPhys::Race race = raceArgOrPlayer(request, 1, console);
    if (race == MachPhys::NORACE)
        return;

    MachLogRaces::instance().nBuildingMaterialUnits(race, amount);
    console.writeLine("BMUs for " + std::string(MachPhys::toString(race)) + " set to " + std::to_string(amount) + ".");
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

// The arguments both spawn commands need before any of the optional ones: the
// type, the subtype and the hardware level.
constexpr std::size_t spawnRequiredArguments = 3;

// How far in front of the camera something spawned without a position lands.
constexpr MATHEX_SCALAR spawnDistanceInFrontOfCamera = 32;

// Where something spawned without a position goes: what the camera is looking
// at. The zenith camera looks at the middle of the screen, so that is the point.
// Every other camera reports where it stands, so the thing goes out along the
// way the camera faces, to land in front of it rather than under it.
std::optional<MexPoint2d> spawnCommandDefaultPosition(MachGuiStartupScreens* pStartup, Console& console)
{
    MachInGameScreen* pScreen = getInGameScreen(pStartup, console);
    if (!pScreen)
        return std::nullopt;

    MachCameras* pCameras = pScreen->cameras();
    const MexPoint3d position = cameraPosition(pCameras);

    if (pCameras->isZenithCameraActive())
        return MexPoint2d(position.x(), position.y());

    // Levelling the camera's own transform before travelling along its X keeps
    // the distance measured over the ground, so a camera that is looking down
    // still puts the thing the same way out as one looking at the horizon.
    MexTransform3d ahead = pCameras->currentCamera()->globalTransform();
    MexEulerAngles angles;
    ahead.rotation(&angles);
    ahead.rotation(MexEulerAngles(angles.azimuth(), 0.0, 0.0));
    ahead.translate(MexPoint3d(spawnDistanceInFrontOfCamera, 0, 0));

    return MexPoint2d(ahead.position().x(), ahead.position().y());
}

// The rotation is the one optional spawn argument that reads as a number, which
// is what tells a line that gives one from a line that leaves it out.
std::optional<MexDegrees> parseRotationDegrees(const std::string& token)
{
    try
    {
        std::size_t consumed{};
        const double degrees = std::stod(token, &consumed);
        if (consumed == token.size() && std::isfinite(degrees))
            return MexDegrees(degrees);
    }
    catch (const std::exception&)
    {
    }

    return std::nullopt;
}

// The optional arguments the spawn commands share, in the order they are typed.
struct SpawnArguments
{
    std::optional<MexPoint2d> position;
    std::optional<MexDegrees> rotation;
    std::optional<std::string> race;
    std::optional<std::string> weaponCombo;
};

// Read the optional arguments, starting at argIndex. Each of them can be left
// out on its own, so which one a token is comes from its shape rather than from
// where it stands: a position has a comma in it, a rotation reads as a number,
// and what follows them is a name. That is why they are all declared as strings.
std::optional<SpawnArguments>
parseSpawnArguments(const Request& request, std::size_t argIndex, bool takesWeaponCombo, Console& console)
{
    std::vector<std::string> tokens;
    for (std::size_t i = argIndex; i < request.arguments.size() && request.arguments[i].provided; ++i)
        tokens.push_back(std::get<std::string>(request.arguments[i].value));

    SpawnArguments arguments;
    std::size_t next = 0;

    if (next < tokens.size())
    {
        arguments.position = parseCoordinates(tokens[next]);
        if (arguments.position.has_value())
            ++next;
        else if (tokens[next].find(',') != std::string::npos)
        {
            // The comma is what makes a token a position, so one that has a
            // comma and will not parse was meant to be one and is worth saying
            // so about rather than being tried as the argument after it.
            console.reportError("Invalid position format. Use x,y (e.g. 123.4,567.8).");
            return std::nullopt;
        }
    }

    if (next < tokens.size())
    {
        arguments.rotation = parseRotationDegrees(tokens[next]);
        if (arguments.rotation.has_value())
            ++next;
    }

    if (next < tokens.size())
        arguments.race = tokens[next++];

    if (takesWeaponCombo && next < tokens.size())
        arguments.weaponCombo = tokens[next++];

    if (next < tokens.size())
    {
        console.reportError(
            "Unexpected argument: " + tokens[next] + ". The level is followed by [position] [rotation] [race]"
            + (takesWeaponCombo ? " [weapon_combo]" : "")
            + ", where a position is x,y and the rotation is a number of degrees.");
        return std::nullopt;
    }

    return arguments;
}

void spawnMachineCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    const std::string& typeStr = std::get<std::string>(request.arguments[0].value);
    const std::string& subTypeStr = std::get<std::string>(request.arguments[1].value);
    const int hwLevel = static_cast<int>(std::get<std::int64_t>(request.arguments[2].value));

    std::optional<MachLog::ObjectType> objType = MachLog::toObjectType(typeStr);
    if (!objType.has_value() || !isMachineObjectType(objType.value()))
    {
        console.reportError("Unknown machine type: " + typeStr);
        return;
    }

    int subType = 0;
    if (objectTypeHasSubType(objType.value()))
        subType = MachLogScenario::objectSubType(objType.value(), subTypeStr);

    if (!isMachineLevelValid(objType.value(), subType, hwLevel))
    {
        console.writeLine(
            "Invalid level " + std::to_string(hwLevel) + " for " + typeStr + " " + subTypeStr
            + ". Valid levels: " + validMachineLevelsString(objType.value(), subType) + ".");
        return;
    }

    const std::optional<SpawnArguments> arguments
        = parseSpawnArguments(request, spawnRequiredArguments, true, console);
    if (!arguments.has_value())
        return;

    const std::optional<MexPoint2d> coords
        = arguments->position.has_value() ? arguments->position : spawnCommandDefaultPosition(pStartup, console);
    if (!coords.has_value())
        return;

    MachPhys::Race race = raceOrPlayer(arguments->race, console);
    if (race == MachPhys::NORACE)
        return;

    std::optional<MachPhys::WeaponCombo> weaponCombo;
    if (arguments->weaponCombo.has_value())
    {
        const std::string& comboStr = arguments->weaponCombo.value();
        weaponCombo = MachPhys::toWeaponCombo(comboStr);
        if (!weaponCombo.has_value())
        {
            console.reportError("Unknown weapon combo: " + comboStr + ".");
            return;
        }
        if (!isWeaponComboValid(objType.value(), subType, hwLevel, weaponCombo.value()))
        {
            const std::string validCombos = validWeaponCombosString(objType.value(), subType, hwLevel);
            if (validCombos.empty())
                console.writeLine("This machine type does not support weapon combos.");
            else
                console.writeLine("Invalid weapon combo for this machine/level. Valid combos: " + validCombos + ".");
            return;
        }
    }
    else
    {
        // No weapon combo specified — pick the first legal one if available
        const auto legal = legalWeaponCombosFor(objType.value(), subType, hwLevel);
        if (!legal.empty())
            weaponCombo = legal[0];
    }

    // Find a free space point near the requested location
    MexPoint2d spawnPos;
    if (MachLogSpacialManipulation::pointIsFree(coords.value(), MachLogMachine::maxHighClearance()))
    {
        spawnPos = coords.value();
    }
    else
    {
        MexTransform3d deployTransform(MexPoint3d(coords->x(), coords->y(), 0));
        MATHEX_SCALAR radius = 10;
        while (!MachLogSpacialManipulation::getNearestFreeSpacePoint(
            deployTransform, radius, MachLogMachine::maxHighClearance(), &spawnPos))
            radius += 20;
    }

    MachLogMachine* pMachine = MachLogActorMaker::newLogMachine(
        objType.value(), subType, hwLevel, 1, race, spawnPos, weaponCombo.value_or(MachPhys::WeaponCombo{}));

    if (arguments->rotation.has_value())
    {
        // The maker leaves the machine standing on the surface, its own Z along
        // the terrain normal, so turning it about that Z aims it without
        // lifting it off a slope.
        MexTransform3d transform = pMachine->physMachine().globalTransform();
        transform.rotate(MexEulerAngles(arguments->rotation.value(), 0.0, 0.0));
        pMachine->physMachine().globalTransform(transform);
    }

    pMachine->teleportIntoWorld();

    console.writeLine(
        "Spawned " + typeStr + " " + subTypeStr + " at " + formatCoordinates(spawnPos.x(), spawnPos.y())
        + ", id=" + std::to_string(pMachine->id()) + ".");
}

void spawnConstructionCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    const std::string& typeStr = std::get<std::string>(request.arguments[0].value);
    const std::string& subTypeStr = std::get<std::string>(request.arguments[1].value);
    const int hwLevel = static_cast<int>(std::get<std::int64_t>(request.arguments[2].value));

    std::optional<MachLog::ObjectType> objType = MachLog::toObjectType(typeStr);
    if (!objType.has_value() || !isConstructionObjectType(objType.value()))
    {
        console.reportError("Unknown building type: " + typeStr);
        return;
    }

    int subType = 0;
    if (objectTypeHasSubType(objType.value()))
        subType = MachLogScenario::objectSubType(objType.value(), subTypeStr);

    if (!isConstructionLevelValid(objType.value(), subType, hwLevel))
    {
        console.writeLine(
            "Invalid level " + std::to_string(hwLevel) + " for " + typeStr + " " + subTypeStr
            + ". Valid levels: " + validConstructionLevelsString(objType.value(), subType) + ".");
        return;
    }

    const std::optional<SpawnArguments> arguments
        = parseSpawnArguments(request, spawnRequiredArguments, false, console);
    if (!arguments.has_value())
        return;

    const std::optional<MexPoint2d> coords
        = arguments->position.has_value() ? arguments->position : spawnCommandDefaultPosition(pStartup, console);
    if (!coords.has_value())
        return;

    MachPhys::Race race = raceOrPlayer(arguments->race, console);
    if (race == MachPhys::NORACE)
        return;

    const MexPoint3d location(coords->x(), coords->y(), 0);
    const MexRadians angle(arguments->rotation.value_or(MexDegrees(0.0)));
    MachLogConstruction* pBuilding
        = MachLogActorMaker::newLogConstruction(objType.value(), subType, hwLevel, location, angle, race);
    pBuilding->makeComplete(MachLogConstruction::FULL_HP_STRENGTH);

    console.writeLine(
        "Spawned " + typeStr + " " + subTypeStr + " at " + formatCoordinates(coords->x(), coords->y())
        + ", id=" + std::to_string(pBuilding->id()) + ".");
}

void reloadDataCommand(MachGuiStartupScreens* pStartup, Console& console)
{
    System::registerMods();
    console.writeLine("- Mods reloaded");

    MachPhysDataImplementation::instance().reloadData();
    console.writeLine("- parmdata reloaded");
    pStartup->reloadUiStrings();
    console.writeLine("- UI strings reloaded");

    // Order matters. Reading the files again comes first, so that the images
    // derived from them are derived from what the mods now say; and both come
    // before the callbacks, so that anything rebuilding itself does so out of
    // pixels that are already current.
    RenDevice::current()->reloadSurfacesFromDisk();
    Gui::rebuildScaledImages();
    RenDevice::current()->fireResourcesInvalidatedCallbacks();
    console.writeLine("- Textures reloaded");

    // New pixels in an image are not new pixels on the screen: a widget draws
    // when something says it has changed, and nothing about it has. Both trees,
    // because the menus and the game each have their own and either may be the
    // one on screen.
    pStartup->changedIncludingChildren();
    pStartup->inGameScreen().changedIncludingChildren();
    console.writeLine("- Screen redrawn");
}

void commandMoveCommand(const Request& request, Console& console)
{
    const std::string& posStr = std::get<std::string>(request.arguments[0].value);

    // Repeating integer arguments starting at index 2 are actor IDs.
    std::vector<UtlId> actorIds;
    for (std::size_t i = 1; i < request.arguments.size(); ++i)
        actorIds.push_back(static_cast<UtlId>(std::get<std::int64_t>(request.arguments[i].value)));

    const std::optional<MexPoint2d> dest = parseCoordinates(posStr);
    if (!dest.has_value())
    {
        console.writeLine("Invalid position format. Use x,y (e.g. 123.4,567.8).");
        return;
    }

    int movedCount = 0;

    for (UtlId id : actorIds)
    {
        if (!MachLogRaces::instance().actorExists(id))
        {
            console.writeLine("Actor " + std::to_string(id) + " does not exist.");
            continue;
        }

        MachActor& actor = MachLogRaces::instance().actor(id);
        if (actor.objectIsMachine())
        {
            MachLogMachine& machine = actor.asMachine();
            machine.newOperation(std::make_unique<MachLogMoveToOperation>(&machine, dest.value()));
            ++movedCount;
        }
        else
        {
            console.writeLine("Actor " + std::to_string(id) + " is not a machine.");
        }
    }

    console.writeLine(
        "Issued move to (" + formatCoordinates(dest->x(), dest->y()) + " for " + std::to_string(movedCount)
        + " machine(s).");
}

// ============================================================
// Save/Load commands
// ============================================================

void listSavesCommand(Console& console)
{
    const uint count = MachGuiDatabase::instance().nSavedGames();
    if (count == 0)
    {
        console.writeLine("No saved games.");
        return;
    }

    console.writeLine("Saved games (" + std::to_string(count) + "):");
    for (uint i = 0; i < count; ++i)
    {
        MachGuiDbSavedGame& sg = MachGuiDatabase::instance().savedGame(i);
        std::string line = "  [" + std::to_string(i) + "] " + sg.userFileName();
        if (sg.hasPlayer())
            line += " (" + sg.player().name() + ")";
        line += " -> " + sg.externalFileName();
        console.writeLine(line);
    }
}

void saveGameCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    if (!pStartup || pStartup->gameType() == MachGuiStartupScreens::NOGAME)
    {
        console.reportError("No game is currently active.");
        return;
    }

    const std::string& saveName = std::get<std::string>(request.arguments[0].value);

    // Check for duplicate name
    const uint count = MachGuiDatabase::instance().nSavedGames();
    for (uint i = 0; i < count; ++i)
    {
        if (strcasecmp(MachGuiDatabase::instance().savedGame(i).userFileName().c_str(), saveName.c_str()) == 0)
        {
            console.writeLine("A saved game with name '" + saveName + "' already exists.");
            return;
        }
    }

    prepareSaveGameDirectory();

    const SysPathName savePathName = nextSaveGamePath();

    MachGuiLoadSaveGameExtras lsgExtras(&pStartup->inGameScreen());
    const bool success = MachLogRaces::instance().saveGame(savePathName, &lsgExtras);

    if (success)
    {
        MachGuiDbScenario* pScenario = pStartup->startupData()->scenario();
        auto* pNewSave = new MachGuiDbSavedGame(saveName, savePathName.c_str(), pScenario);

        if (MachGuiDatabase::instance().hasCurrentPlayer())
            pNewSave->player(&MachGuiDatabase::instance().currentPlayer());

        if (pStartup->gameType() == MachGuiStartupScreens::CAMPAIGNGAME)
            pNewSave->isCampaignGame(true);

        MachGuiDatabase::instance().addSavedGame(pNewSave);
        MachGuiDatabase::instance().writeDatabase();

        console.writeLine("Game saved as '" + saveName + "' (" + std::string(savePathName.c_str()) + ").");
    }
    else
    {
        console.writeLine("Failed to save game.");
    }
}

void loadGameCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    if (!pStartup)
    {
        console.writeLine("Startup screens not available.");
        return;
    }

    const uint count = MachGuiDatabase::instance().nSavedGames();
    if (count == 0)
    {
        console.reportError("No saved games.");
        return;
    }

    MachGuiDbSavedGame* pSavedGame = nullptr;

    {
        const std::string& name = std::get<std::string>(request.arguments[0].value);

        // Try parsing as integer index first
        char* end = nullptr;
        const unsigned long idx = strtoul(name.c_str(), &end, 10);
        if (end != name.c_str() && *end == '\0' && idx < count)
        {
            pSavedGame = &MachGuiDatabase::instance().savedGame(static_cast<uint>(idx));
        }
        else
        {
            // Search by name
            for (uint i = 0; i < count; ++i)
            {
                if (strcasecmp(MachGuiDatabase::instance().savedGame(i).userFileName().c_str(), name.c_str()) == 0)
                {
                    pSavedGame = &MachGuiDatabase::instance().savedGame(i);
                    break;
                }
            }
        }
    }

    if (!pSavedGame)
    {
        console.writeLine("Save game not found. Use list_saves to see available saves.");
        return;
    }

    // Unload current game if active
    pStartup->unloadGame();

    console.writeLine("Loading '" + pSavedGame->userFileName() + "'...");
    pStartup->loadSavedGame(pSavedGame);

    // Set context_ so loopCycleInGame() runs and the 3D world renders
    if (pStartup->gameType() == MachGuiStartupScreens::CAMPAIGNGAME)
        pStartup->setContextForGame(MachGuiStartupScreens::CTX_GAME);
    else
        pStartup->setContextForGame(MachGuiStartupScreens::CTX_SKIRMISH_GAME);

    MachInGameScreen& inGameScreen = pStartup->inGameScreen();
    GuiManager::instance().keyboardFocus(&inGameScreen);
    inGameScreen.becomeRoot();
    inGameScreen.activate();
    SimManager::instance().resume();

    console.writeLine("Game loaded.");
}

// ============================================================
// Pause command
// ============================================================

void pauseCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    if (!getInGameScreen(pStartup, console))
        return;

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        const bool paused = SimManager::instance().isSuspended();
        console.writeLine(std::string("Game is ") + (paused ? "paused." : "running."));
        return;
    }

    const bool pause = std::get<bool>(request.arguments[0].value);
    if (pause)
        SimManager::instance().suspend();
    else
        SimManager::instance().resume();
    console.writeLine(std::string("Game ") + (pause ? "paused." : "resumed."));
}

void gameSpeedCommand(const Request& request, Console& console)
{
    constexpr double minSpeed{0.001};
    constexpr double maxSpeed{8.0};

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        console.writeLine("Game speed: " + formatFloat(SimManager::instance().speed(), 3) + ".");
        return;
    }

    const double speed = std::get<double>(request.arguments[0].value);
    if (speed < minSpeed || speed > maxSpeed)
    {
        console.reportError(
            "Game speed must be between " + formatFloat(minSpeed, 3) + " and " + formatFloat(maxSpeed, 3) + ", got "
            + formatFloat(speed, 3) + ".");
        return;
    }

    SimManager::instance().setSpeed(speed);
    console.writeLine("Game speed set to " + formatFloat(speed, 3) + ".");
}

void setTimeCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    if (!getInGameScreen(pStartup, console))
        return;

    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        console.writeLine("Current simulation time: " + formatFloat(SimManager::instance().currentTime(), 2));
        return;
    }

    const double time = std::get<double>(request.arguments[0].value);
    if (time < 0.0)
    {
        console.writeLine("Time must be non-negative.");
        return;
    }

    SimManager::instance().setSimulationTime(time);
    console.writeLine("Simulation time set to " + formatFloat(time, 2) + ".");
}

// ============================================================
// Tab-completion helpers
// ============================================================

using Metadata = System::IConsole::CommandMetadata;

std::vector<std::string> filterByPrefix(std::string_view partial, const std::vector<std::string>& candidates)
{
    std::vector<std::string> result;
    for (const std::string& c : candidates)
    {
        if (partial.empty() || Utils::startsWithCaseInsensitive(c, partial))
            result.push_back(c);
    }
    return result;
}

std::vector<std::string> availablePlanetNames()
{
    std::vector<std::string> names;
    constexpr std::string_view planetDir = "models/planet";

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(planetDir))
        {
            if (!entry.is_directory())
                continue;

            const std::string name = entry.path().filename().string();
            const std::string psfPath = std::string(planetDir) + "/" + name + "/" + name + ".psf";
            if (SysPathName::existsAsFile(psfPath))
                names.push_back(name);
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
    }

    std::sort(names.begin(), names.end());
    return names;
}

template <std::size_t N>
std::vector<std::string> objectTypeNamesOf(const MachLog::ObjectType (&arr)[N])
{
    std::vector<std::string> names;
    names.reserve(N);
    for (MachLog::ObjectType t : arr)
        names.emplace_back(MachLog::toString(t));
    return names;
}

const std::vector<std::string>& machineTypeNames()
{
    static const std::vector<std::string> names = objectTypeNamesOf(AllMachineObjectTypes);
    return names;
}

const std::vector<std::string>& constructionTypeNames()
{
    static const std::vector<std::string> names = objectTypeNamesOf(AllConstructionObjectTypes);
    return names;
}

const std::vector<std::string>& raceNames()
{
    static const auto names = []
    {
        std::vector<std::string> v;
        v.reserve(std::size(MachPhys::AllRaces));
        for (MachPhys::Race r : MachPhys::AllRaces)
            v.emplace_back(MachPhys::toString(r));
        return v;
    }();
    return names;
}

std::vector<std::string> machineSubtypesFor(MachLog::ObjectType type)
{
    switch (type)
    {
    case MachLog::AGGRESSOR:
        return {"GRUNT", "ASSASSIN", "KNIGHT", "BALLISTA", "NINJA"};
    case MachLog::ADMINISTRATOR:
        return {"BOSS", "OVERSEER", "COMMANDER"};
    case MachLog::CONSTRUCTOR:
        return {"DOZER", "BUILDER", "BEHEMOTH"};
    case MachLog::TECHNICIAN:
        return {"LAB_TECH", "TECH_BOY", "BRAIN_BOX"};
    default:
        return {"NONE"};
    }
}

std::vector<std::string> constructionSubtypesFor(MachLog::ObjectType type)
{
    switch (type)
    {
    case MachLog::FACTORY:
        return {"CIVILIAN", "MILITARY"};
    case MachLog::HARDWARE_LAB:
        return {"LAB_CIVILIAN", "LAB_MILITARY"};
    case MachLog::MISSILE_EMPLACEMENT:
        return {"TURRET", "SENTRY", "LAUNCHER", "ICBM"};
    default:
        return {"NONE"};
    }
}

std::vector<std::string> validMachineLevels(MachLog::ObjectType objType, int subType)
{
    const auto& levels = MachPhysLevels::instance();
    const MachPhys::MachineType machType = MachLogMapper::mapToPhysMachine(objType);
    std::vector<std::string> result;

    if (!objectTypeHasSubType(objType))
    {
        for (size_t i = 0; i < levels.nHardwareLevels(machType); ++i)
            result.push_back(std::to_string(levels.hardwareLevel(machType, i)));
    }
    else
    {
        switch (objType)
        {
        case MachLog::AGGRESSOR:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::AggressorSubType>(subType)); ++i)
                result.push_back(
                    std::to_string(levels.hardwareLevel(static_cast<MachPhys::AggressorSubType>(subType), i)));
            break;
        case MachLog::ADMINISTRATOR:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::AdministratorSubType>(subType)); ++i)
                result.push_back(
                    std::to_string(levels.hardwareLevel(static_cast<MachPhys::AdministratorSubType>(subType), i)));
            break;
        case MachLog::CONSTRUCTOR:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::ConstructorSubType>(subType)); ++i)
                result.push_back(
                    std::to_string(levels.hardwareLevel(static_cast<MachPhys::ConstructorSubType>(subType), i)));
            break;
        case MachLog::TECHNICIAN:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::TechnicianSubType>(subType)); ++i)
                result.push_back(
                    std::to_string(levels.hardwareLevel(static_cast<MachPhys::TechnicianSubType>(subType), i)));
            break;
        default:
            break;
        }
    }
    return result;
}

std::vector<std::string> validConstructionLevels(MachLog::ObjectType objType, int subType)
{
    const auto& levels = MachPhysLevels::instance();
    const MachPhys::ConstructionType conType = MachLogMapper::mapToPhysConstruction(objType);
    std::vector<std::string> result;

    if (!objectTypeHasSubType(objType))
    {
        for (size_t i = 0; i < levels.nHardwareLevels(conType); ++i)
            result.push_back(std::to_string(levels.hardwareLevel(conType, i)));
    }
    else
    {
        switch (objType)
        {
        case MachLog::FACTORY:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::FactorySubType>(subType)); ++i)
                result.push_back(
                    std::to_string(levels.hardwareLevel(static_cast<MachPhys::FactorySubType>(subType), i)));
            break;
        case MachLog::HARDWARE_LAB:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::HardwareLabSubType>(subType)); ++i)
                result.push_back(
                    std::to_string(levels.hardwareLevel(static_cast<MachPhys::HardwareLabSubType>(subType), i)));
            break;
        case MachLog::MISSILE_EMPLACEMENT:
            for (size_t i = 0; i < levels.nHardwareLevels(static_cast<MachPhys::MissileEmplacementSubType>(subType));
                 ++i)
                result.push_back(
                    std::to_string(levels.hardwareLevel(static_cast<MachPhys::MissileEmplacementSubType>(subType), i)));
            break;
        default:
            break;
        }
    }
    return result;
}

std::vector<std::string> weaponComboCompletions(
    MachLog::ObjectType objType, int subType, size_t hwLevel, std::string_view partial)
{
    const auto legal = legalWeaponCombosFor(objType, subType, hwLevel);
    std::vector<std::string> result;
    for (const auto& combo : legal)
    {
        std::string name(MachPhys::toString(combo));
        if (partial.empty() || Utils::startsWithCaseInsensitive(name, partial))
            result.push_back(name);
    }
    return result;
}

// The weapon combos legal for the machine the preceding arguments describe.
std::vector<std::string>
weaponComboCompletionsFor(const std::vector<std::string>& precedingArgs, std::string_view partial)
{
    // Needs type (arg0), subtype (arg1) and hwlevel (arg2).
    if (precedingArgs.size() < 3)
        return {};

    std::optional<MachLog::ObjectType> objType = MachLog::toObjectType(precedingArgs[0]);
    if (!objType.has_value() || !isMachineObjectType(objType.value()))
        return {};

    int subType = 0;
    if (objectTypeHasSubType(objType.value()))
        subType = MachLogScenario::objectSubType(objType.value(), precedingArgs[1]);

    std::size_t hwLevel = 1;
    try
    {
        hwLevel = static_cast<std::size_t>(std::stoi(precedingArgs[2]));
    }
    catch (...)
    {
    }

    return weaponComboCompletions(objType.value(), subType, hwLevel, partial);
}

// True while the race is still the next optional spawn argument to be typed.
// Neither a position nor a rotation has anything to complete to, so counting how
// many of the tokens already typed were one of those -- by the same shapes the
// command itself reads them by -- is all it takes to know what the one being
// typed is.
bool raceIsNextSpawnArgument(const std::vector<std::string>& precedingArgs)
{
    std::size_t next = spawnRequiredArguments;
    if (next < precedingArgs.size() && parseCoordinates(precedingArgs[next]).has_value())
        ++next;
    if (next < precedingArgs.size() && parseRotationDegrees(precedingArgs[next]).has_value())
        ++next;

    return next == precedingArgs.size();
}

std::vector<std::string> spawnMachineCompleter(
    const Metadata& /*metadata*/,
    std::size_t argIndex,
    std::string_view partial,
    const std::vector<std::string>& precedingArgs)
{
    // arg0=type, arg1=subtype, arg2=hwlevel, then [pos] [rotation] [race]
    // [weapon_combo] -- each one left out moves the rest up a slot, so which
    // argument a slot holds is worked out from what was typed before it.
    switch (argIndex)
    {
    case 0:
        return filterByPrefix(partial, machineTypeNames());
    case 1:
    {
        // Use the type from precedingArgs[0] if available.
        if (!precedingArgs.empty())
        {
            std::optional<MachLog::ObjectType> objType = MachLog::toObjectType(precedingArgs[0]);
            if (objType.has_value() && isMachineObjectType(objType.value()))
                return filterByPrefix(partial, machineSubtypesFor(objType.value()));
        }
        // Fallback: offer union of all machine subtypes.
        std::vector<std::string> all;
        for (MachLog::ObjectType t : AllMachineObjectTypes)
        {
            if (!objectTypeHasSubType(t))
                continue;
            for (const auto& s : machineSubtypesFor(t))
            {
                if (std::find(all.begin(), all.end(), s) == all.end())
                    all.push_back(s);
            }
        }
        all.emplace_back("NONE");
        return filterByPrefix(partial, all);
    }
    case 2:
    {
        // HW level: need type (arg0) and subtype (arg1).
        if (precedingArgs.size() >= 2)
        {
            std::optional<MachLog::ObjectType> objType = MachLog::toObjectType(precedingArgs[0]);
            if (objType.has_value() && isMachineObjectType(objType.value()))
            {
                int subType = 0;
                if (objectTypeHasSubType(objType.value()))
                    subType = MachLogScenario::objectSubType(objType.value(), precedingArgs[1]);
                return filterByPrefix(partial, validMachineLevels(objType.value(), subType));
            }
        }
        return {};
    }
    case 3:
    case 4:
    case 5:
    case 6:
        return raceIsNextSpawnArgument(precedingArgs) ? filterByPrefix(partial, raceNames())
                                                      : weaponComboCompletionsFor(precedingArgs, partial);
    default:
        return {};
    }
}

std::vector<std::string> spawnConstructionCompleter(
    const Metadata& /*metadata*/,
    std::size_t argIndex,
    std::string_view partial,
    const std::vector<std::string>& precedingArgs)
{
    // arg0=type, arg1=subtype, arg2=hwlevel, then [pos] [rotation] [race] --
    // each one left out moves the rest up a slot, so which argument a slot holds
    // is worked out from what was typed before it.
    switch (argIndex)
    {
    case 0:
        return filterByPrefix(partial, constructionTypeNames());
    case 1:
    {
        // Use the type from precedingArgs[0] if available.
        if (!precedingArgs.empty())
        {
            std::optional<MachLog::ObjectType> objType = MachLog::toObjectType(precedingArgs[0]);
            if (objType.has_value() && isConstructionObjectType(objType.value()))
                return filterByPrefix(partial, constructionSubtypesFor(objType.value()));
        }
        // Fallback: offer union of all construction subtypes.
        std::vector<std::string> all;
        for (MachLog::ObjectType t : AllConstructionObjectTypes)
        {
            if (!objectTypeHasSubType(t))
                continue;
            for (const auto& s : constructionSubtypesFor(t))
            {
                if (std::find(all.begin(), all.end(), s) == all.end())
                    all.push_back(s);
            }
        }
        all.emplace_back("NONE");
        return filterByPrefix(partial, all);
    }
    case 2:
    {
        // HW level: need type (arg0) and subtype (arg1).
        if (precedingArgs.size() >= 2)
        {
            std::optional<MachLog::ObjectType> objType = MachLog::toObjectType(precedingArgs[0]);
            if (objType.has_value() && isConstructionObjectType(objType.value()))
            {
                int subType = 0;
                if (objectTypeHasSubType(objType.value()))
                    subType = MachLogScenario::objectSubType(objType.value(), precedingArgs[1]);
                return filterByPrefix(partial, validConstructionLevels(objType.value(), subType));
            }
        }
        return {};
    }
    case 3:
    case 4:
    case 5:
        return raceIsNextSpawnArgument(precedingArgs) ? filterByPrefix(partial, raceNames())
                                                      : std::vector<std::string>();
    default:
        return {};
    }
}

void screenshotCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    if (!pStartup)
    {
        console.reportError("The startup screens are not up yet.");
        return;
    }

    std::string fileName;
    if (!request.arguments.empty() && request.arguments[0].provided)
    {
        fileName = std::get<std::string>(request.arguments[0].value);
        const std::optional<std::string> complaint = Gui::screenshotNameComplaint(fileName);
        if (complaint.has_value())
        {
            console.reportError(complaint.value());
            return;
        }
    }

    pStartup->requestScreenShot(fileName);

    // The shot is of the next frame, so where it will go is all there is to say
    // about it yet. Whether it got written is in the log.
    if (fileName.empty())
        console.writeLine("Photographing the next frame.");
    else
        console.writeLine("Photographing the next frame into " + Gui::screenshotPath(fileName).pathname() + ".");
}

void loadPlanetCommand(MachGuiStartupScreens* pStartup, const Request& request, Console& console)
{
    if (!pStartup)
    {
        console.reportError("The startup screens are not up yet.");
        return;
    }

    if (pStartup->gameType() != MachGuiStartupScreens::NOGAME)
    {
        console.reportError("A game is already loaded. There is no way back to the menus from here yet.");
        return;
    }

    const std::string& name = std::get<std::string>(request.arguments[0].value);
    const std::vector<std::string> available = availablePlanetNames();
    if (std::find(available.begin(), available.end(), name) == available.end())
    {
        console.reportError("No such planet: " + name + ". Available: " + Utils::join(available, ", "));
        return;
    }

    MachPhys::Race race = MachPhys::RED;
    if (request.arguments.size() > 1 && request.arguments[1].provided)
    {
        const std::string& raceName = std::get<std::string>(request.arguments[1].value);
        const std::optional<MachPhys::Race> parsed = MachPhys::toRace(raceName);
        if (!parsed.has_value())
        {
            console.reportError("Unknown race: " + raceName + ". Use red, blue, green, or yellow.");
            return;
        }

        race = parsed.value();
    }

    pStartup->requestBarePlanet(name, race);

    console.writeLine("Loading " + name + " for " + std::string(MachPhys::toString(race)) + ".");
}

} // anonymous namespace

} // namespace ConsoleImpl

void registerConsoleCommands(System::IConsole& console, MachGuiStartupScreens* pStartup)
{
    using namespace ConsoleImpl;

    console.registerCommand(
        {
         .name = "dev_mode",
         .description = "Get/set developer mode.",
         .arguments = { {.name = "state", .type = Arg::Boolean, .optional = true, .description = "on or off. Omit to print current state." } },
         },
        setDevModeCommand);

    // ---- Camera commands ----

    console.registerCommand(
        {
            .name = "cam_type",
            .description = "Get/set camera type (zenith, ground, 1stperson).",
            .arguments = {{.name = "type", .type = Arg::String, .optional = true, .description = "Camera type."}},
        },
        [pStartup](const Request& request, Console& console) { camTypeCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "cam_pos",
            .description = "Get/set camera position. Zenith: x,y [zoom]. Ground: x,y [z]. 1stperson: x,y [z].",
            .arguments = {
                { .name = "pos", .type = Arg::String, .optional = true, .description = "Position as x,y." },
                { .name = "z", .type = Arg::Float, .optional = true, .description = "Z or zoom distance." },
            },
        },
        [pStartup](const Request& request, Console& console) { camPosCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "cam_dir",
            .description = "Get/set camera direction in degrees. Zenith/ground: yaw. 1stperson: yaw [pitch] [roll].",
            .arguments = {
                { .name = "yaw", .type = Arg::Float, .optional = true, .description = "Yaw in degrees." },
                { .name = "pitch", .type = Arg::Float, .optional = true, .description = "Pitch in degrees (1stperson only)." },
                { .name = "roll", .type = Arg::Float, .optional = true, .description = "Roll in degrees (1stperson only)." },
            },
        },
        [pStartup](const Request& request, Console& console) { camDirCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "fov",
            .description = "Get/set the camera's vertical field of view in degrees; horizontal follows the window.",
            .arguments = {{
                .name = "vertical",
                .type = Arg::Float,
                .optional = true,
                .description = "10 to 140 degrees. Omit to print current.",
            }},
            .cheat = true,
            .devOnly = true,
        },
        [pStartup](const Request& request, Console& console) { fovCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "cam_lookat",
            .description = "Point camera at a world position.",
            .arguments = {
                { .name = "pos", .type = Arg::String, .description = "Position as x,y." },
            },
        },
        [pStartup](const Request& request, Console& console) { camLookatCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "fow",
            .description = "Get/set fog of war (on/off).",
            .arguments = {{
                .name = "state",
                .type = Arg::Boolean,
                .optional = true,
                .description = "on or off. Omit to print current.",
            }},
            .cheat = true,
        },
        [pStartup](const Request& request, Console& console) { fowCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "ui",
            .description = "Get/set whether anything is drawn over the world (on/off).",
            .arguments = {{
                .name = "state",
                .type = Arg::Boolean,
                .optional = true,
                .description = "on or off. Omit to print current.",
            }},
        },
        [pStartup](const Request& request, Console& console) { uiCommand(pStartup, request, console); });

    // ---- Resource commands ----

    console.registerCommand(
        {
            .name = "give_bmu",
            .description = "Add BMUs to a race. Defaults to player race.",
            .arguments = {
                { .name = "amount", .type = Arg::Integer, .description = "Amount of BMUs to add." },
                { .name = "race", .type = Arg::Identifier, .optional = true, .description = "Race: red, blue, green, yellow." },
            },
            .cheat = true,
        },
        [](const Request& request, Console& console) { giveBmuCommand(request, console); });

    console.registerCommand(
        {
            .name = "set_bmu",
            .description = "Set BMUs to an exact value. Defaults to player race.",
            .arguments = {
                { .name = "amount", .type = Arg::Integer, .description = "BMU amount." },
                { .name = "race", .type = Arg::Identifier, .optional = true, .description = "Race: red, blue, green, yellow." },
            },
            .cheat = true,
        },
        [](const Request& request, Console& console) { setBmuCommand(request, console); });

    console.registerCommand(
        {
            .name = "get_bmu",
            .description = "Print current BMU count. Defaults to player race.",
            .arguments
            = {
               {.name = "race", .type = Arg::Identifier, .optional = true, .description = "Race: red, blue, green, yellow."}
            },
        },
        [](const Request& request, Console& console) { getBmuCommand(request, console); });

    // ---- Spawn commands ----

    console.registerCommand(
        {
            .name = "spawn_machine",
            .description = "Spawn a machine at a position, turned to a rotation.",
            .arguments = {
                { .name = "type", .type = Arg::Identifier, .description = "Machine type (aggressor, constructor, etc.)." },
                { .name = "subtype", .type = Arg::Identifier, .description = "Subtype (grunt, dozer, etc.)." },
                { .name = "hwlevel", .type = Arg::Integer, .description = "Hardware level." },
                { .name = "pos", .type = Arg::String, .optional = true, .description = "Position as x,y. Omit to spawn where the camera is looking." },
                { .name = "rotation", .type = Arg::String, .optional = true, .description = "Rotation in degrees. Omit to face along +X." },
                { .name = "race", .type = Arg::String, .optional = true, .description = "Race: red, blue, green, yellow. Omit for the player race." },
                { .name = "weapon_combo", .type = Arg::String, .optional = true, .description = "Weapon combo (e.g. l_auto_cannon). Uses .scn names.", },
            },
            .cheat = true,
            .devOnly = true,
        },
        [pStartup](const Request& request, Console& console) { spawnMachineCommand(pStartup, request, console); },
        spawnMachineCompleter);

    console.registerCommand(
        {
            .name = "spawn_construction",
            .description = "Spawn a construction at a position, turned to a rotation.",
            .arguments = {
                { .name = "type", .type = Arg::Identifier, .description = "Construction type (factory, smelter, etc.)." },
                { .name = "subtype", .type = Arg::Identifier, .description = "Subtype (civilian, military, etc.)." },
                { .name = "hwlevel", .type = Arg::Integer, .description = "Hardware level." },
                { .name = "pos", .type = Arg::String, .optional = true, .description = "Position as x,y. Omit to spawn where the camera is looking." },
                { .name = "rotation", .type = Arg::String, .optional = true, .description = "Rotation in degrees. Omit to face along +X." },
                { .name = "race", .type = Arg::String, .optional = true, .description = "Race: red, blue, green, yellow. Omit for the player race." },
            },
            .cheat = true,
            .devOnly = true,
        },
        [pStartup](const Request& request, Console& console) { spawnConstructionCommand(pStartup, request, console); },
        spawnConstructionCompleter);

    // ---- Game commands ----

    console.registerCommand(
        {
            .name = "pause",
            .description = "Get/set game pause (on/off).",
            .arguments = {
                {.name = "state", .type = Arg::Boolean, .optional = true, .description = "on or off. Omit to print current state."},
            },
        },
        [pStartup](const Request& request, Console& console) { pauseCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "game_speed",
            .description = "Get/set the rate at which simulation time advances relative to real time.",
            .arguments = {
                {.name = "speed", .type = Arg::Float, .optional = true, .description = "Multiplier from 0.001 to 20. Omit to print the current speed."},
            },
        },
        [](const Request& request, Console& console) { gameSpeedCommand(request, console); });

    console.registerCommand(
        {
            .name = "time",
            .description = "Get/set current simulation time used by stars, satellites, and other time-based systems.",
            .arguments = {
               {.name = "time", .type = Arg::Float, .optional = true, .description = "Absolute simulation time in seconds. Omit to print current time."}
            },
            .cheat = true,
            .devOnly = true,
        },
        [pStartup](const Request& request, Console& console)
        { setTimeCommand(pStartup, request, console); });

    // ---- Save/Load commands ----

    console.registerCommand(
        {
            .name = "list_saves",
            .description = "List all saved games.",
        },
        [](const Request&, Console& console) { listSavesCommand(console); });

    console.registerCommand(
        {
            .name = "save_game",
            .description = "Save the current game with the given name.",
            .arguments = {
                { .name = "name", .type = Arg::String, .description = "Display name for the save." },
            },
        },
        [pStartup](const Request& request, Console& console)
        { saveGameCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "load_game",
            .description = "Load a saved game by index or name. Use list_saves to see available saves.",
            .arguments = {
                { .name = "name_or_index", .type = Arg::String, .description = "Save name or index from list_saves." },
            },
        },
        [pStartup](const Request& request, Console& console)
        { loadGameCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "screenshot",
            .description = "Photograph the next frame into the screenshots directory.",
            .arguments = { { .name = "name", .type = Arg::String, .optional = true,
                             .description = "File name, letters, digits, underscores and dots only, no directory."
                                            " Omit to number it after the shots already taken." } },
        },
        [pStartup](const Request& request, Console& console) { screenshotCommand(pStartup, request, console); });

    console.registerCommand(
        {
            .name = "load_planet",
            .description = "Load a planet's terrain with nothing on it.",
            .arguments = {
                { .name = "planet", .type = Arg::String, .description = "Planet name, e.g. m_desert." },
                { .name = "race", .type = Arg::Identifier, .optional = true,
                  .description = "Race to play. Defaults to red." },
            },
            .devOnly = true,
        },
        [pStartup](const Request& request, Console& console) { loadPlanetCommand(pStartup, request, console); },
        [](const System::IConsole::CommandMetadata&,
           std::size_t argIndex,
           std::string_view partialValue,
           const std::vector<std::string>&) -> std::vector<std::string>
    {
        if (argIndex == 0)
            return filterByPrefix(partialValue, availablePlanetNames());
        if (argIndex == 1)
            return filterByPrefix(partialValue, raceNames());

        return {};
    });

    // ---- Data reload commands ----

    console.registerCommand(
        {
            .name = "reload_data",
            .description = "Reload gameplay data files (parmdata.dat and mod overrides).",
            .cheat = true,
        },
        [pStartup](const Request&, Console& console) { reloadDataCommand(pStartup, console); });

    // ---- Order commands ----

    console.registerCommand(
        {
            .name = "command_move",
            .description = "Order machines to move. Usage: command_move x y id1 [id2] [id3] ...",
            .arguments = {
                { .name = "pos", .type = Arg::String, .description = "Destination as x,y." },
            },
            .cheat = true,
            .devOnly = true,
        },
        [](const Request& request, Console& console) { commandMoveCommand(request, console); });
}

} // namespace MachGui
