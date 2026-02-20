#include "FogMode.hpp"

std::string_view toString(FogMode mode)
{
    switch (mode)
    {
    case FogMode::Linear:
        return "Linear";
    case FogMode::Exponential2:
        return "Exponential2";
    }

    return {};
}
