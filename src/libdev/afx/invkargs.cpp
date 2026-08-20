#include "afx/invkargs.hpp"

#include <algorithm>

bool AfxInvokeArgs::contains(std::string_view flag) const
{
    return std::find(begin(), end(), flag) != end();
}

std::optional<std::string_view> AfxInvokeArgs::value(std::string_view flag) const
{
    for (auto it = begin(); it != end(); ++it)
    {
        const std::string_view token{*it};
        if (token.size() > flag.size() && token.compare(0, flag.size(), flag) == 0
            && token[flag.size()] == '=')
        {
            return token.substr(flag.size() + 1);
        }
    }

    return std::nullopt;
}
