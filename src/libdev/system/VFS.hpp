#pragma once

#include <string>
#include <vector>

namespace System
{

std::vector<std::string> listAvailableMods();
std::vector<std::string> listMods();
void registerMods();

void addFsOverride(std::string extraPath);

std::string findFile(std::string_view path);

// includes the given path if it points to an existing file
// The files are sorted from the lowest to highest priority
std::vector<std::string> getFileOverrides(std::string path);

} // namespace System
