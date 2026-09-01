#include "ConfigVariables.hpp"

namespace Config
{

Variable<int32_t> watchdogTimeout("Diagnostics/WatchdogTimeout", 20);

Variable<int32_t> gfxMsaaBuffers("Options/MSAA/Buffers", 1);
Variable<int32_t> gfxMsaaSamples("Options/MSAA/Samples", 2);

Variable<int32_t> gfxResolutionWidth("Screen Resolution/Width", 0);
Variable<int32_t> gfxResolutionHeight("Screen Resolution/Height", 0);
Variable<int32_t> gfxRefreshRate("Screen Resolution/Refresh Rate", 0);

Variable<int32_t> uiZenithCameraAcceleration("Options/Zenith Camera/Acceleration", 15);
Variable<int32_t> uiZenithCameraSpeedLimit("Options/Zenith Camera/Speed Limit", 15);

Variable<int32_t> uiGroundCameraAcceleration("Options/Ground Camera/Acceleration", 15);

Variable<bool> uiUse2DSelectionMarker("Options/Cursor Type/2D", false);

Variable<int32_t> uiScaleFactor("Options/Scale Factor/Value", 0);
Variable<bool> grabCursor("Options/Grab Cursor/on", true);

Variable<bool> debugShowRenderStats({}, false);

Variable<uint32_t> soundVolume("Options/Sound/Volume", 95);
Variable<uint32_t> musicVolume("Options/CD/Volume", 30);

Variable<bool> consoleEnabled("Options/Console/Enabled", false);
Variable<bool> consoleHistoryPersistence("Options/Console/History Persistence", true);

Variable<bool> devMode({}, {});

} // namespace Config
