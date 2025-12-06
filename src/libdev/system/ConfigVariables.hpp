#pragma once

#include "Variable.hpp"

#include <cstdint>

namespace Config
{

extern Variable<int32_t> gfxMsaaBuffers;
extern Variable<int32_t> gfxMsaaSamples;

extern Variable<int32_t> uiScaleFactor;
extern Variable<bool> grabCursor;

extern Variable<uint32_t> soundVolume;
extern Variable<uint32_t> musicVolume;

} // namespace Config
