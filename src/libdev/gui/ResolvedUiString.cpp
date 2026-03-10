#include "ResolvedUiString.hpp"

#include "gui/ResourceString.hpp"

ResolvedUiString::ResolvedUiString(Gui::StringId stringId) :
    id_(stringId)
{
}

bool ResolvedUiString::empty() const
{
    return asStringView().empty();
}

std::string ResolvedUiString::asString() const
{
    fillValue();
    return str_.value();
}

std::string_view ResolvedUiString::asStringView() const
{
    fillValue();
    return str_.value();
}

void ResolvedUiString::fillValue() const
{
    if (!str_.has_value())
        str_ = id_ ? GuiResourceString(id_).asString() : std::string{};
}
