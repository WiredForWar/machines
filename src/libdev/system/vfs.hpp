#pragma once

#include <string>

namespace System
{

bool registerFsOverride(std::string extraPath);
void clearFsOverrides();

std::string findFile(std::string path);

} // namespace System
