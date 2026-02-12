#pragma once

#include <string_view>

enum class LightingMode
{
    Legacy,
    PerPixel,
};

inline constexpr LightingMode AllLightingModes[] = {
    LightingMode::Legacy,
    LightingMode::PerPixel,
};

std::string_view toString(LightingMode mode);
