#include "render/WindowMode.hpp"

namespace Ren
{

std::string_view toString(WindowMode mode)
{
    switch (mode)
    {
    case WindowMode::Fullscreen:
        return "Fullscreen";
    case WindowMode::Borderless:
        return "Borderless";
    case WindowMode::Windowed:
        return "Windowed";
    }

    return {};
}

} // namespace Ren
