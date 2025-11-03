#pragma once

#include <string>

#include "gui/StringId.hpp"

class ResolvedUiString
{
public:
    ResolvedUiString() = default;

    ResolvedUiString(std::string str)
        : str_(std::move(str))
    {
    }

    ResolvedUiString(Gui::StringId stringId);

    operator std::string() const { return str_; }

private:
    std::string str_;
};
