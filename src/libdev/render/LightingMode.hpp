#pragma once

#include <string_view>

enum class LightingMode
{
    Legacy,
    PerVertex,
    PerPixel,
};

inline constexpr LightingMode AllLightingModes[] = {
    LightingMode::Legacy,
    LightingMode::PerVertex,
    LightingMode::PerPixel,
};

std::string_view toString(LightingMode mode);
