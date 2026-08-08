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

extern Variable<int32_t> uiZenithCameraAcceleration;
extern Variable<int32_t> uiZenithCameraSpeedLimit;

extern Variable<int32_t> uiGroundCameraAcceleration;

extern Variable<bool> uiUse2DSelectionMarker;

extern Variable<int32_t> uiScaleFactor;
extern Variable<bool> grabCursor;

extern Variable<bool> debugShowRenderStats;

extern Variable<uint32_t> soundVolume;
extern Variable<uint32_t> musicVolume;

extern Variable<bool> consoleEnabled;
extern Variable<bool> consoleHistoryPersistence;

extern Variable<bool> devMode;

} // namespace Config
