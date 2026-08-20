#pragma once

#include "device/Key.hpp"
#include "device/KeyWithModifiers.hpp"

#include <optional>
#include <ostream>
#include <string>
#include <string_view>

// Text forms of a button code, in the enumerator spelling -- KEY_A,
// PAGE_UP_PAD, MOUSE_LEFT. Never translated, so these are the diagnostic form
// rather than a form to show a player.
namespace Device
{

void writeAsString(std::ostream&, KeyCode code);
std::string toString(KeyCode code);

// Matching ignores case. The mouse buttons also answer to the short names
// left, right and middle.
std::optional<KeyCode> codeFromString(std::string_view name);

// A key with its modifiers, as "Ctrl+Shift+KEY_W". Modifiers may be given in
// any order and are drawn from Ctrl, Alt and Shift.
std::optional<KeyWithModifiers> chordFromString(std::string_view text);

} // namespace Device
