#pragma once

#include "Variable.hpp"

#include <cstdint>

namespace Config
{

extern Variable<int32_t> gfxMsaaBuffers;
extern Variable<int32_t> gfxMsaaSamples;

extern Variable<int32_t> gfxResolutionWidth;
extern Variable<int32_t> gfxResolutionHeight;
extern Variable<int32_t> gfxRefreshRate;
extern Variable<bool> gfxWindowed;
extern Variable<bool> gfxLockResolution;

extern Variable<int32_t> uiScaleFactor;
extern Variable<bool> grabCursor;

extern Variable<uint32_t> soundVolume;
extern Variable<uint32_t> musicVolume;

} // namespace Config
