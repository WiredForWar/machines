#pragma once

#include <string>

#include "gui/StringId.hpp"

#include <optional>

class ResolvedUiString
{
public:
    ResolvedUiString() = default;

    ResolvedUiString(std::string str)
        : str_(std::move(str))
    {
    }

    ResolvedUiString(Gui::StringId stringId);

    bool empty() const;

    std::string asString() const;
    std::string_view asStringView() const;
    operator std::string() const { return asString(); }
    operator std::string_view() const { return asStringView(); }

private:
    void fillValue() const;

    mutable std::optional<std::string> str_;
    Gui::StringId id_{};
};
