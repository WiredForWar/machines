#pragma once

#include "render/render.hpp"

namespace MachGui
{

constexpr Ren::Size MenuSize{640, 480};

// Zero follows the window size; explicit choices preserve the original pixels
// at whole-number scales.
inline constexpr int AllScaleFactorValues[] = {0, 100, 200, 300, 400};

bool scaleFactorFits(int scaleFactorPercents, Ren::Size windowSize);

// Honour an explicit scale when it fits. Otherwise use the largest supported
// scale that fits both dimensions, with 100% as the minimum.
int resolveScaleFactor(int preferredPercents, Ren::Size windowSize);

} // namespace MachGui
