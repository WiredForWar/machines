#pragma once

#include "Variable.hpp"

#include <cstdint>

namespace Config
{

extern Variable<int32_t> gfxMsaaBuffers;
extern Variable<int32_t> gfxMsaaSamples;

extern Variable<int32_t> uiScaleFactor;

} // namespace Config
