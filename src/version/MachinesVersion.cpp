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
