#pragma once

#include "system/Variable.hpp"

enum class FogMode;
enum class LightingMode;
enum class ShadowQuality;

namespace Ren
{

enum class BackendType;
enum class WindowMode;

} // namespace Ren

namespace Config
{

extern Variable<Ren::BackendType> gfxBackendType;
extern Variable<Ren::WindowMode> gfxWindowMode;

// True when no window mode has been settled on yet, so that a choice recorded
// under an older name can still be honoured.
bool windowModeIsUnset();

extern Variable<LightingMode> gfxLightingMode;
extern Variable<ShadowQuality> gfxShadowQuality;
// Takes effect at the next start.
extern Variable<bool> gfxToneMapping;
extern Variable<FogMode> gfxFogMode;
extern Variable<bool> gfxModernRendering;

} // namespace Config
