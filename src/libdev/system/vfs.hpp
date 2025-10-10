#pragma once

#include <string>
#include <vector>

namespace System
{

std::vector<std::string> listAvailableMods();
std::vector<std::string> listMods();
void addFsOverride(std::string extraPath);

std::string findFile(std::string path);

} // namespace System
