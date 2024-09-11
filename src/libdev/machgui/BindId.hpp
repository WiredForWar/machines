#pragma once

#include <string_view>

namespace MachGui
{

// Utility class which allows to find all binds in the source code
class BindId : public std::string_view
{
public:
    constexpr explicit BindId(std::string_view asView)
        : std::string_view(asView)
    {
    }
};

inline constexpr BindId NoBind = BindId("no-bind");

} // namespace MachGui

inline MachGui::BindId operator""_bind(const char* str, std::size_t length)
{
    return MachGui::BindId(std::string_view(str, length));
}
