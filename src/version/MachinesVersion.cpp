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
