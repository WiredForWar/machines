#pragma once

#include "system/Variable.hpp"

enum class FogMode;
enum class LightingMode;
enum class ShadowQuality;

namespace Config
{

extern Variable<LightingMode> gfxLightingMode;
extern Variable<ShadowQuality> gfxShadowQuality;
extern Variable<bool> gfxToneMapping;
extern Variable<FogMode> gfxFogMode;
extern Variable<bool> gfxModernRendering;

} // namespace Config
