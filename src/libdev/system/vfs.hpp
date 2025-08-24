#pragma once

#include <string>

namespace System
{

void registerFsOverride(std::string extraPath);

std::string findFile(std::string path);

} // namespace System
