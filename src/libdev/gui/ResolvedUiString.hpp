#pragma once

#include <string>

class ResolvedUiString
{
public:
    ResolvedUiString(std::string str)
        : str_(std::move(str))
    {
    }

    ResolvedUiString(unsigned int stringId);

    operator std::string() const { return str_; }

private:
    std::string str_;
};
