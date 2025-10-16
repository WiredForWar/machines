#pragma once

#include "device/KeyBind.hpp"

#include <string>

namespace Gui
{

enum class KeysDisplayFormat
{
    Verbose,
    Compact,
};

std::string toDisplayString(Device::KeyCode keyCode, KeysDisplayFormat format = {});
std::string toDisplayString(Device::KeyModifier mod, KeysDisplayFormat format = {});
std::string toDisplayString(const KeyBind& bind, KeysDisplayFormat format = {});

} // namespace Gui
