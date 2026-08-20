#include "device/KeyNames.hpp"

#include <algorithm>
#include <sstream>

#include <cctype>

void Device::writeAsString(std::ostream& o, KeyCode code)
{
    if (code == KeyCode::UNKNOWN || !isValidCode(code))
    {
        o << static_cast<int>(code);
        return;
    }

    switch (code)
    {
        case KeyCode::UP_ARROW_PAD:
            o << "UP_ARROW_PAD";
            break;
        case KeyCode::DOWN_ARROW_PAD:
            o << "DOWN_ARROW_PAD";
            break;
        case KeyCode::LEFT_ARROW_PAD:
            o << "LEFT_ARROW_PAD";
            break;
        case KeyCode::RIGHT_ARROW_PAD:
            o << "RIGHT_ARROW_PAD";
            break;
        case Device::KeyCode::ENTER_PAD:
            o << "ENTER_PAD";
            break;
        case Device::KeyCode::INSERT_PAD:
            o << "INSERT_PAD";
            break;
        case Device::KeyCode::DELETE_PAD:
            o << "DELETE_PAD";
            break;
        case Device::KeyCode::HOME_PAD:
            o << "HOME_PAD";
            break;
        case Device::KeyCode::END_PAD:
            o << "END_PAD";
            break;
        case Device::KeyCode::PAGE_UP_PAD:
            o << "PAGE_UP_PAD";
            break;
        case Device::KeyCode::PAGE_DOWN_PAD:
            o << "PAGE_DOWN_PAD";
            break;
        case Device::KeyCode::FIVE_PAD:
            o << "FIVE_PAD";
            break;
        case Device::KeyCode::ASTERISK_PAD:
            o << "ASTERISK_PAD";
            break;
        case Device::KeyCode::FORWARD_SLASH_PAD:
            o << "FORWARD_SLASH_PAD";
            break;
        case Device::KeyCode::PLUS_PAD:
            o << "PLUS_PAD";
            break;
        case Device::KeyCode::MINUS_PAD:
            o << "MINUS_PAD";
            break;
        case Device::KeyCode::PAD_0:
            o << "PAD_0";
            break;
        case Device::KeyCode::PAD_1:
            o << "PAD_1";
            break;
        case Device::KeyCode::PAD_2:
            o << "PAD_2";
            break;
        case Device::KeyCode::PAD_3:
            o << "PAD_3";
            break;
        case Device::KeyCode::PAD_4:
            o << "PAD_4";
            break;
        case Device::KeyCode::PAD_5:
            o << "PAD_5";
            break;
        case Device::KeyCode::PAD_6:
            o << "PAD_6";
            break;
        case Device::KeyCode::PAD_7:
            o << "PAD_7";
            break;
        case Device::KeyCode::PAD_8:
            o << "PAD_8";
            break;
        case Device::KeyCode::PAD_9:
            o << "PAD_9";
            break;
        case Device::KeyCode::FORWARD_SLASH:
            o << "FORWARD_SLASH";
            break;
        case Device::KeyCode::NUM_LOCK:
            o << "NUM_LOCK";
            break;
        case Device::KeyCode::SCROLL_LOCK:
            o << "SCROLL_LOCK";
            break;
        case Device::KeyCode::RIGHT_CONTROL:
            o << "RIGHT_CONTROL";
            break;
        case Device::KeyCode::LEFT_CONTROL:
            o << "LEFT_CONTROL";
            break;
        case Device::KeyCode::UP_ARROW:
            o << "UP_ARROW";
            break;
        case Device::KeyCode::DOWN_ARROW:
            o << "DOWN_ARROW";
            break;
        case Device::KeyCode::LEFT_ARROW:
            o << "LEFT_ARROW";
            break;
        case Device::KeyCode::RIGHT_ARROW:
            o << "RIGHT_ARROW";
            break;
        case Device::KeyCode::INSERT:
            o << "INSERT";
            break;
        case Device::KeyCode::DELETE:
            o << "DELETE";
            break;
        case Device::KeyCode::HOME:
            o << "HOME";
            break;
        case Device::KeyCode::END:
            o << "END";
            break;
        case Device::KeyCode::BREAK:
            o << "BREAK";
            break;
        case Device::KeyCode::PAGE_UP:
            o << "PAGE_UP";
            break;
        case Device::KeyCode::PAGE_DOWN:
            o << "PAGE_DOWN";
            break;
        case Device::KeyCode::ESCAPE:
            o << "ESCAPE";
            break;
        case Device::KeyCode::ENTER:
            o << "ENTER";
            break;
        case Device::KeyCode::SPACE:
            o << "SPACE";
            break;
        case Device::KeyCode::BACK_SPACE:
            o << "BACK_SPACE";
            break;
        case Device::KeyCode::TAB:
            o << "TAB";
            break;
        case Device::KeyCode::GRAVE:
            o << "GRAVE";
            break;
        case Device::KeyCode::LEFT_SHIFT:
            o << "LEFT_SHIFT";
            break;
        case Device::KeyCode::RIGHT_SHIFT:
            o << "RIGHT_SHIFT";
            break;
        case Device::KeyCode::CAPS_LOCK:
            o << "CAPS_LOCK";
            break;
        case Device::KeyCode::F1:
            o << "F1";
            break;
        case Device::KeyCode::F2:
            o << "F2";
            break;
        case Device::KeyCode::F3:
            o << "F3";
            break;
        case Device::KeyCode::F4:
            o << "F4";
            break;
        case Device::KeyCode::F5:
            o << "F5";
            break;
        case Device::KeyCode::F6:
            o << "F6";
            break;
        case Device::KeyCode::F7:
            o << "F7";
            break;
        case Device::KeyCode::F8:
            o << "F8";
            break;
        case Device::KeyCode::F9:
            o << "F9";
            break;
        case Device::KeyCode::F10:
            o << "F10";
            break;
        case Device::KeyCode::F11:
            o << "F11";
            break;
        case Device::KeyCode::F12:
            o << "F12";
            break;
        case Device::KeyCode::KEY_A:
            o << "KEY_A";
            break;
        case Device::KeyCode::KEY_B:
            o << "KEY_B";
            break;
        case Device::KeyCode::KEY_C:
            o << "KEY_C";
            break;
        case Device::KeyCode::KEY_D:
            o << "KEY_D";
            break;
        case Device::KeyCode::KEY_E:
            o << "KEY_E";
            break;
        case Device::KeyCode::KEY_F:
            o << "KEY_F";
            break;
        case Device::KeyCode::KEY_G:
            o << "KEY_G";
            break;
        case Device::KeyCode::KEY_H:
            o << "KEY_H";
            break;
        case Device::KeyCode::KEY_I:
            o << "KEY_I";
            break;
        case Device::KeyCode::KEY_J:
            o << "KEY_J";
            break;
        case Device::KeyCode::KEY_K:
            o << "KEY_K";
            break;
        case Device::KeyCode::KEY_L:
            o << "KEY_L";
            break;
        case Device::KeyCode::KEY_M:
            o << "KEY_M";
            break;
        case Device::KeyCode::KEY_N:
            o << "KEY_N";
            break;
        case Device::KeyCode::KEY_O:
            o << "KEY_O";
            break;
        case Device::KeyCode::KEY_P:
            o << "KEY_P";
            break;
        case Device::KeyCode::KEY_Q:
            o << "KEY_Q";
            break;
        case Device::KeyCode::KEY_R:
            o << "KEY_R";
            break;
        case Device::KeyCode::KEY_S:
            o << "KEY_S";
            break;
        case Device::KeyCode::KEY_T:
            o << "KEY_T";
            break;
        case Device::KeyCode::KEY_U:
            o << "KEY_U";
            break;
        case Device::KeyCode::KEY_V:
            o << "KEY_V";
            break;
        case Device::KeyCode::KEY_W:
            o << "KEY_W";
            break;
        case Device::KeyCode::KEY_X:
            o << "KEY_X";
            break;
        case Device::KeyCode::KEY_Y:
            o << "KEY_Y";
            break;
        case Device::KeyCode::KEY_Z:
            o << "KEY_Z";
            break;
        case Device::KeyCode::KEY_0:
            o << "KEY_0";
            break;
        case Device::KeyCode::KEY_1:
            o << "KEY_1";
            break;
        case Device::KeyCode::KEY_2:
            o << "KEY_2";
            break;
        case Device::KeyCode::KEY_3:
            o << "KEY_3";
            break;
        case Device::KeyCode::KEY_4:
            o << "KEY_4";
            break;
        case Device::KeyCode::KEY_5:
            o << "KEY_5";
            break;
        case Device::KeyCode::KEY_6:
            o << "KEY_6";
            break;
        case Device::KeyCode::KEY_7:
            o << "KEY_7";
            break;
        case Device::KeyCode::KEY_8:
            o << "KEY_8";
            break;
        case Device::KeyCode::KEY_9:
            o << "KEY_9";
            break;
        case Device::KeyCode::LEFT_ALT:
            o << "LEFT_ALT";
            break;
        case Device::KeyCode::RIGHT_ALT:
            o << "RIGHT_ALT";
            break;
        case Device::KeyCode::MOUSE_LEFT:
            o << "MOUSE_LEFT";
            break;
        case Device::KeyCode::MOUSE_RIGHT:
            o << "MOUSE_RIGHT";
            break;
        case Device::KeyCode::MOUSE_MIDDLE:
            o << "MOUSE_MIDDLE";
            break;

        case Device::KeyCode::MOUSE_EXTRA1:
            o << "MOUSE_EXTRA1";
            break;
        case Device::KeyCode::MOUSE_EXTRA2:
            o << "MOUSE_EXTRA2";
            break;
        case Device::KeyCode::MOUSE_EXTRA3:
            o << "MOUSE_EXTRA3";
            break;
        case Device::KeyCode::MOUSE_EXTRA4:
            o << "MOUSE_EXTRA4";
            break;
        case Device::KeyCode::MOUSE_EXTRA5:
            o << "MOUSE_EXTRA5";
            break;
        case Device::KeyCode::MOUSE_EXTRA6:
            o << "MOUSE_EXTRA6";
            break;
        case Device::KeyCode::MOUSE_EXTRA7:
            o << "MOUSE_EXTRA7";
            break;
        case Device::KeyCode::MOUSE_EXTRA8:
            o << "MOUSE_EXTRA8";
            break;

        case KeyCode::UNKNOWN:
        case KeyCode::COUNT:
            // Processed before this switch
            break;
    }
}

std::string Device::toString(KeyCode code)
{
    std::ostringstream stream;
    writeAsString(stream, code);
    return stream.str();
}

namespace
{

bool equalIgnoringCase(std::string_view left, std::string_view right)
{
    return left.size() == right.size()
        && std::equal(left.begin(), left.end(), right.begin(), [](char a, char b) {
               return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
           });
}

std::string_view trimmed(std::string_view text)
{
    const auto isSpace = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };

    while (!text.empty() && isSpace(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && isSpace(text.back()))
        text.remove_suffix(1);

    return text;
}

} // namespace

std::optional<Device::KeyCode> Device::codeFromString(std::string_view name)
{
    name = trimmed(name);
    if (name.empty())
        return std::nullopt;

    if (equalIgnoringCase(name, "left"))
        return KeyCode::MOUSE_LEFT;
    if (equalIgnoringCase(name, "right"))
        return KeyCode::MOUSE_RIGHT;
    if (equalIgnoringCase(name, "middle"))
        return KeyCode::MOUSE_MIDDLE;

    for (int code = 0; code != MAX_CODE; ++code)
    {
        const KeyCode candidate = static_cast<KeyCode>(code);
        if (candidate == KeyCode::UNKNOWN)
            continue;

        if (equalIgnoringCase(name, toString(candidate)))
            return candidate;
    }

    return std::nullopt;
}

std::optional<KeyWithModifiers> Device::chordFromString(std::string_view text)
{
    text = trimmed(text);
    if (text.empty())
        return std::nullopt;

    KeyModifierFlags modifiers;

    // Everything before the last separator is a modifier; the tail is the key.
    std::size_t start = 0;
    while (true)
    {
        const std::size_t separator = text.find('+', start);
        if (separator == std::string_view::npos)
            break;

        const std::string_view token = trimmed(text.substr(start, separator - start));
        if (equalIgnoringCase(token, "ctrl"))
            modifiers = modifiers | KeyModifier::Ctrl;
        else if (equalIgnoringCase(token, "alt"))
            modifiers = modifiers | KeyModifier::Alt;
        else if (equalIgnoringCase(token, "shift"))
            modifiers = modifiers | KeyModifier::Shift;
        else
            return std::nullopt;

        start = separator + 1;
    }

    const std::optional<KeyCode> code = codeFromString(text.substr(start));
    if (!code)
        return std::nullopt;

    return KeyWithModifiers(*code, modifiers);
}
