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
    operator std::string() const { return asString(); }

    // The view is good for as long as this object is, so ask for it by name
    // rather than letting a temporary convert to one and leave it dangling.
    std::string_view asStringView() const;

private:
    void fillValue() const;

    mutable std::optional<std::string> str_;
    Gui::StringId id_{};
};
