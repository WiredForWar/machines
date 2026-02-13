#pragma once

#include "system/Variable.hpp"

enum class FogMode;
enum class LightingMode;
enum class ShadowQuality;

namespace Ren
{

enum class BackendType;

} // namespace Ren

namespace Config
{

extern Variable<Ren::BackendType> gfxBackendType;

extern Variable<LightingMode> gfxLightingMode;
extern Variable<ShadowQuality> gfxShadowQuality;
extern Variable<bool> gfxToneMapping;
extern Variable<FogMode> gfxFogMode;
extern Variable<bool> gfxModernRendering;

} // namespace Config
