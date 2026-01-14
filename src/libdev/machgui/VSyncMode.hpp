#pragma once

#include "system/Variable.hpp"

namespace MachGui
{

enum class VSyncMode
{
    Auto,
    Enabled,
    Disabled,
};

std::string toString(const MachGui::VSyncMode& value);

} // namespace MachGui

namespace Config
{

extern Variable<MachGui::VSyncMode> gfxVSyncMode;

} // namespace Config
