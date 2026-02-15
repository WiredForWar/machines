#pragma once

#include <string_view>

enum class ShadowQuality
{
    Static,
    Hard,
    Soft,
};

inline constexpr ShadowQuality AllShadowQualities[] = {
    ShadowQuality::Static,
    ShadowQuality::Hard,
    ShadowQuality::Soft,
};

std::string_view toString(ShadowQuality quality);
