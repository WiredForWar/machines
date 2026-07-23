#ifndef SYSTEM_SYSINFO_HPP
#define SYSTEM_SYSINFO_HPP

#include <string>

std::string getCpuId();
const std::string& getOsVersion();

// The packaging format the application runs in, e.g. "Flatpak".
// Empty for a native/portable installation.
const std::string& getPackagingInfo();

#endif // SYSTEM_SYSINFO_HPP
