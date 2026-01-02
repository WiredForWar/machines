#include "KeyToString.hpp"

#include "gui/ResolvedUiString.hpp"
#include "gui/font.hpp"

#include "machgui/internal/strings.hpp"

#include <sstream>

namespace MachGui
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
        case Device::KeyCode::RIGHT_CONTROL:
            return ResolvedUiString(IDS_KEYNAME_RIGHT_CONTROL);
        case Device::KeyCode::LEFT_CONTROL:
            return ResolvedUiString(IDS_KEYNAME_LEFT_CONTROL);
        case Device::KeyCode::UP_ARROW:
            return ResolvedUiString(IDS_KEYNAME_ARROW_UP);
        case Device::KeyCode::DOWN_ARROW:
            return ResolvedUiString(IDS_KEYNAME_ARROW_DOWN);
        case Device::KeyCode::LEFT_ARROW:
            return ResolvedUiString(IDS_KEYNAME_ARROW_LEFT);
        case Device::KeyCode::RIGHT_ARROW:
            return ResolvedUiString(IDS_KEYNAME_ARROW_RIGHT);
        case Device::KeyCode::INSERT:
            return ResolvedUiString(IDS_KEYNAME_INSERT);
        case Device::KeyCode::DELETE:
            return ResolvedUiString(IDS_KEYNAME_DELETE);
        case Device::KeyCode::HOME:
            return ResolvedUiString(IDS_KEYNAME_HOME);
        case Device::KeyCode::END:
            return ResolvedUiString(IDS_KEYNAME_END);
        case Device::KeyCode::PAGE_UP:
            return ResolvedUiString(IDS_KEYNAME_PAGE_UP);
        case Device::KeyCode::PAGE_DOWN:
            return ResolvedUiString(IDS_KEYNAME_PAGE_DOWN);
        case Device::KeyCode::BREAK:
            return ResolvedUiString(IDS_KEYNAME_BREAK);
        case Device::KeyCode::ESCAPE:
            return ResolvedUiString(IDS_KEYNAME_ESCAPE);
        case Device::KeyCode::ENTER:
            return ResolvedUiString(IDS_KEYNAME_ENTER);
        case Device::KeyCode::SPACE:
            return ResolvedUiString(IDS_KEYNAME_SPACE);
        case Device::KeyCode::BACK_SPACE:
            return ResolvedUiString(IDS_KEYNAME_BACKSPACE);
        case Device::KeyCode::TAB:
            return ResolvedUiString(IDS_KEYNAME_TAB);
        case Device::KeyCode::GRAVE:
            return ResolvedUiString(IDS_KEYNAME_GRAVE);
        case Device::KeyCode::LEFT_SHIFT:
            return ResolvedUiString(IDS_KEYNAME_LEFT_SHIFT);
        case Device::KeyCode::RIGHT_SHIFT:
            return ResolvedUiString(IDS_KEYNAME_RIGHT_SHIFT);
        case Device::KeyCode::LEFT_ALT:
            return ResolvedUiString(IDS_KEYNAME_LEFT_ALT);
        case Device::KeyCode::RIGHT_ALT:
            return ResolvedUiString(IDS_KEYNAME_RIGHT_ALT);

        case Device::KeyCode::MOUSE_LEFT:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_LEFT);
        case Device::KeyCode::MOUSE_RIGHT:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_RIGHT);
        case Device::KeyCode::MOUSE_MIDDLE:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_MIDDLE);
        case Device::KeyCode::MOUSE_EXTRA1:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_EXTRA1);
        case Device::KeyCode::MOUSE_EXTRA2:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_EXTRA2);
        case Device::KeyCode::MOUSE_EXTRA3:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_EXTRA3);
        case Device::KeyCode::MOUSE_EXTRA4:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_EXTRA4);
        case Device::KeyCode::MOUSE_EXTRA5:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_EXTRA5);
        case Device::KeyCode::MOUSE_EXTRA6:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_EXTRA6);
        case Device::KeyCode::MOUSE_EXTRA7:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_EXTRA7);
        case Device::KeyCode::MOUSE_EXTRA8:
            return ResolvedUiString(IDS_KEYNAME_MOUSE_EXTRA8);

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
            return ResolvedUiString(IDS_KEYNAME_ALT);
        case Device::KeyModifier::Ctrl:
            return ResolvedUiString(IDS_KEYNAME_CTRL);
        case Device::KeyModifier::Shift:
            return ResolvedUiString(IDS_KEYNAME_SHIFT);
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
    const bool keyFirst = format == KeysDisplayFormat::Compact && isAlphaNumKey(bind.keyCode());

    if (keyFirst)
    {
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
        addKey(toDisplayString(bind.keyCode(), format));
    }

    return result;
}

} // namespace MachGui
