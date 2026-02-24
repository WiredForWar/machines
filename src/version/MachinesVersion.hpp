#include <cstdint>
#include <string>

std::string machinesVersion();
std::string machinesBuildVersion();

// Packs version as (0, major, minor, patch) bytes in a uint32_t.
uint32_t machinesVersionNumber();
std::string versionNumberToString(uint32_t version);
