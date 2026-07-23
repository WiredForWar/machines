#include <cstdint>
#include <string>

std::string machinesVersion();
std::string machinesBuildVersion();

// Compiler, architecture, and build type, e.g. "MSVC 19.44.35211, x86_64, Debug".
std::string machinesBuildInfo();

// Packs version as (0, major, minor, patch) bytes in a uint32_t.
uint32_t machinesVersionNumber();
std::string versionNumberToString(uint32_t version);
