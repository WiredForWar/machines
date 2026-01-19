#include "InputLayout.hpp"

#include "system/Variable_p.hpp"

namespace Config
{

namespace Impl
{

template <>
std::string toString(const MachGui::InputLayout& value)
{
    return MachGui::toString(value);
}

template <>
std::optional<MachGui::InputLayout> toValue(const std::string& asString)
{
    using InputLayout = MachGui::InputLayout;
    for (InputLayout layout : {
             InputLayout::Legacy,
             InputLayout::WASD,
         })
    {
        if (asString == toString(layout))
            return layout;
    }

    return std::nullopt;
}

} // namespace Impl

template class Config::Variable<MachGui::InputLayout>;

Variable<MachGui::InputLayout> inputBaseLayout("Input/BaseLayout", MachGui::InputLayout::WASD);

} // namespace Config

namespace MachGui
{

std::string toString(const InputLayout& value)
{
    switch (value)
    {
        case MachGui::InputLayout::Legacy:
            return "Legacy";
        case MachGui::InputLayout::WASD:
            return "WASD";
    }

    return std::to_string(static_cast<int32_t>(value));
}

} // namespace MachGui
