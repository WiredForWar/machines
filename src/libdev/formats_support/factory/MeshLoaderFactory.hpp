#pragma once

#include "formats_support/IMeshLoader.hpp"

#include <memory>
#include <vector>

namespace FormatSupport
{

std::vector<std::unique_ptr<IMeshLoader>> createMeshLoaders();

} // namespace FormatSupport
