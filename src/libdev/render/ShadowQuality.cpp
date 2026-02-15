#include "ShadowQuality.hpp"

std::string_view toString(ShadowQuality quality)
{
    switch (quality)
    {
    case ShadowQuality::Static:
        return "None";
    case ShadowQuality::Hard:
        return "Hard";
    case ShadowQuality::Soft:
        return "Soft";
    }

    return {};
}
