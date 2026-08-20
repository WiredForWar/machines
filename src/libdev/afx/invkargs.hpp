#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The command line, one token per element, without the program name.
class AfxInvokeArgs : public std::vector<std::string>
{
public:
    // True if the exact token was given, as for "--skip-logos".
    bool contains(std::string_view flag) const;

    // The text after the '=' of a "--flag=value" token. Nothing if the flag was
    // not given, or was given on its own.
    std::optional<std::string_view> value(std::string_view flag) const;
};
