#pragma once

#include "render/render.hpp"

namespace MachGui
{

struct LoadingScreenLayout
{
    Ren::Rect backdrop;
    Ren::Point animationOffset;
    Ren::Point noticeBottom;
    Ren::Rect progressBar;
};

// Positions in the original 640x480 artwork share one scale and origin during
// both asset loading and the short robot animation that follows it.
LoadingScreenLayout loadingScreenLayout(Ren::Size windowSize, double scale);

} // namespace MachGui
