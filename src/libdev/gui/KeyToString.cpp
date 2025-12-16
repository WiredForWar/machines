#include "KeyToString.hpp"

#include "gui/font.hpp"

#include <sstream>

namespace Gui
{

namespace
{

bool inRange(int value, int minValue, int maxValue)
{
    return value >= minValue && value <= maxValue;
}

template <typename T>
bool inRange(T value, T minValue, T maxValue)
{
    return inRange(static_cast<int>(value), static_cast<int>(minValue), static_cast<int>(maxValue));
}

} // namespace

std::string toDisplayString(Device::KeyCode keyCode, KeysDisplayFormat format)
{
    if (inRange(keyCode, Device::KeyCode::KEY_A, Device::KeyCode::KEY_Z))
        return std::string(1, static_cast<int>(keyCode) - static_cast<int>(Device::KeyCode::KEY_A) + 'A');
    else if (inRange(keyCode, Device::KeyCode::KEY_0, Device::KeyCode::KEY_9))
        return std::string(1, static_cast<int>(keyCode) - static_cast<int>(Device::KeyCode::KEY_0) + '0');
    else if (inRange(keyCode, Device::KeyCode::PAD_0, Device::KeyCode::PAD_9))
        return "NumPad " + std::string(1, static_cast<int>(keyCode) - static_cast<int>(Device::KeyCode::PAD_0) + '0');

    if (format == KeysDisplayFormat::Compact)
    {
        switch (keyCode)
        {
            case Device::KeyCode::UP_ARROW:
                return {GuiBmpFont::arrowUpIndex()};
            case Device::KeyCode::DOWN_ARROW:
                return {GuiBmpFont::arrowDownIndex()};
            case Device::KeyCode::LEFT_ARROW:
                return {GuiBmpFont::arrowLeftIndex()};
            case Device::KeyCode::RIGHT_ARROW:
                return {GuiBmpFont::arrowRightIndex()};
            default:
                break;
        }
    }

    switch (keyCode)
    {
        case Device::KeyCode::FORWARD_SLASH:
            return "/";
        case Device::KeyCode::UP_ARROW:
            return "Up";
        case Device::KeyCode::DOWN_ARROW:
            return "Down";
        case Device::KeyCode::LEFT_ARROW:
            return "Left";
        case Device::KeyCode::RIGHT_ARROW:
            return "Right";
        case Device::KeyCode::INSERT:
            return "Insert";
        case Device::KeyCode::DELETE:
            return "Delete";
        case Device::KeyCode::HOME:
            return "Home";
        case Device::KeyCode::END:
            return "End";
        case Device::KeyCode::ESCAPE:
            return "Esc";
        case Device::KeyCode::BACK_SPACE:
            return "Backspace";
        case Device::KeyCode::TAB:
            return "Tab";

        case Device::KeyCode::MOUSE_LEFT:
            return "Left Mouse Button";
        case Device::KeyCode::MOUSE_RIGHT:
            return "Right Mouse Button";

        default:
            break;
    }

    std::ostringstream stream;
    Device::writeAsString(stream, keyCode);
    return stream.str();
}

std::string toDisplayString(Device::KeyModifier mod, KeysDisplayFormat format)
{
    switch (mod)
    {
        case Device::KeyModifier::Alt:
            return "Alt";
        case Device::KeyModifier::Ctrl:
            return "Ctrl";
        case Device::KeyModifier::Shift:
            return "Shift";
            break;
        case Device::KeyModifier::None:
            break;
    }

    return {};
}

std::string toDisplayString(const KeyBind& bind, KeysDisplayFormat format)
{
    std::string result;
    const char joiner = format == KeysDisplayFormat::Verbose ? '+' : ' ';
    auto addKey = [&result, joiner](std::string key)
    {
        if (result.empty())
            result = key;
        else
            result += joiner + key;
    };

    using KeyModifier = Device::KeyModifier;
    const bool keyFirst = format == KeysDisplayFormat::Compact;

    if (keyFirst)
    {
        if (bind.keyCode() != Device::KeyCode::UNKNOWN)
            addKey(toDisplayString(bind.keyCode(), format));
    }

    const KeyModifierFlags pressedMods = bind.keyWithMods.modifiers();
    for (const KeyModifier mod : KeyModifierFlags::Order)
    {
        if (pressedMods & mod)
            addKey(toDisplayString(mod, format));
    }

    if (!keyFirst)
    {
        if (bind.keyCode() != Device::KeyCode::UNKNOWN)
            addKey(toDisplayString(bind.keyCode(), format));
    }

    return result;
}

} // namespace Gui
