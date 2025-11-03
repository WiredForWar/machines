#pragma once

#include "system/Variable.hpp"

namespace MachGui
{

enum class InputLayout
{
    Legacy,
    WASD,
};

std::string toString(const MachGui::InputLayout& value);

} // namespace MachGui

namespace Config
{

extern Variable<MachGui::InputLayout> inputBaseLayout;

} // namespace Config
