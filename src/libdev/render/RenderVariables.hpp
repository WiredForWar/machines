#pragma once

#include "system/Variable.hpp"

enum class LightingMode;
enum class ShadowQuality;

namespace Config
{

extern Variable<LightingMode> gfxLightingMode;
extern Variable<ShadowQuality> gfxShadowQuality;
extern Variable<bool> gfxToneMapping;

} // namespace Config
