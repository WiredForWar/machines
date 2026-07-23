#include "MachinesVersion.hpp"

#include "machines_version.h"

std::string machinesVersion()
{
    return Machines_VERSION;
}

std::string machinesBuildVersion()
{
    return Machines_COMMIT_INFO;
}

#ifndef MACHINES_BUILD_CONFIG
#define MACHINES_BUILD_CONFIG ""
#endif

std::string machinesBuildInfo()
{
    std::string info = Machines_BUILD_COMPILER ", " Machines_BUILD_ARCH;
    if (MACHINES_BUILD_CONFIG[0] != '\0')
    {
        info += ", ";
        info += MACHINES_BUILD_CONFIG;
    }
    return info;
}

uint32_t machinesVersionNumber()
{
    return (Machines_VERSION_MAJOR << 16) | (Machines_VERSION_MINOR << 8) | Machines_VERSION_PATCH;
}

std::string versionNumberToString(uint32_t version)
{
    return std::to_string((version >> 16) & 0xFF) + "."
        + std::to_string((version >> 8) & 0xFF) + "."
        + std::to_string(version & 0xFF);
}
