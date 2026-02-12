#pragma once

#include <string_view>

enum class LightingMode
{
    Legacy,
    PerVertex,
};

inline constexpr LightingMode AllLightingModes[] = {
    LightingMode::Legacy,
    LightingMode::PerVertex,
};

std::string_view toString(LightingMode mode);
