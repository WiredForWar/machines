#pragma once

#include <utility>

#include "render/render.hpp"
#include "render/internal/BackendTypes.hpp"

namespace Ren
{

inline std::pair<BackendBlendFactor, BackendBlendFactor> blendFactorsForBlitMode(BlitMode mode)
{
    using BF = BackendBlendFactor;
    switch (mode)
    {
    case BlitMode::AlphaBlend:
        return {BF::SrcAlpha, BF::OneMinusSrcAlpha};
    case BlitMode::Replace:
        return {BF::One, BF::Zero};
    case BlitMode::DstMulOneMinusSrcAlpha:
        return {BF::Zero, BF::OneMinusSrcAlpha};
    case BlitMode::ZeroZero:
        return {BF::Zero, BF::Zero};
    }
    return {BF::One, BF::Zero};
}

} // namespace Ren
