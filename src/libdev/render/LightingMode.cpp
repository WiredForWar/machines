#include "LightingMode.hpp"

std::string_view toString(LightingMode mode)
{
    switch (mode)
    {
    case LightingMode::Legacy:
        return "Legacy";
    case LightingMode::PerVertex:
        return "PerVertex";
    case LightingMode::PerPixel:
        return "PerPixel";
    }

    return {};
}
