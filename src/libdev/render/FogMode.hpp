#pragma once

#include <string_view>

enum class FogMode
{
    Linear,
    Exponential2,
};

inline constexpr FogMode AllFogModes[] = {
    FogMode::Linear,
    FogMode::Exponential2,
};

std::string_view toString(FogMode mode);
