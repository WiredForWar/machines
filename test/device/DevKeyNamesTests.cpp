#include <gtest/gtest.h>

#include "device/KeyNames.hpp"

///////////////////////////////////////////////////////////////////////////////

TEST(DevKeyNamesTests, EveryCodeRoundTrips)
{
    for (int value = 0; value != Device::MAX_CODE; ++value)
    {
        const Device::KeyCode code = static_cast<Device::KeyCode>(value);
        if (code == Device::KeyCode::UNKNOWN)
            continue;

        const std::string name = Device::toString(code);
        ASSERT_FALSE(name.empty()) << "code " << value << " has no name";

        const std::optional<Device::KeyCode> parsed = Device::codeFromString(name);
        ASSERT_TRUE(parsed.has_value()) << name << " does not parse back";
        ASSERT_EQ(code, *parsed) << name << " parses back to something else";
    }
}

TEST(DevKeyNamesTests, MatchingIgnoresCaseAndSurroundingSpace)
{
    ASSERT_EQ(Device::KeyCode::KEY_W, Device::codeFromString("key_w"));
    ASSERT_EQ(Device::KeyCode::KEY_W, Device::codeFromString("KeY_w"));
    ASSERT_EQ(Device::KeyCode::ESCAPE, Device::codeFromString("  escape  "));
}

TEST(DevKeyNamesTests, TheMouseAnswersToItsShortNames)
{
    ASSERT_EQ(Device::KeyCode::MOUSE_LEFT, Device::codeFromString("left"));
    ASSERT_EQ(Device::KeyCode::MOUSE_RIGHT, Device::codeFromString("RIGHT"));
    ASSERT_EQ(Device::KeyCode::MOUSE_MIDDLE, Device::codeFromString("Middle"));

    // The enumerator spelling still works.
    ASSERT_EQ(Device::KeyCode::MOUSE_LEFT, Device::codeFromString("MOUSE_LEFT"));
    ASSERT_EQ(Device::KeyCode::MOUSE_EXTRA3, Device::codeFromString("mouse_extra3"));
}

TEST(DevKeyNamesTests, AnUnknownNameIsRejected)
{
    ASSERT_FALSE(Device::codeFromString("").has_value());
    ASSERT_FALSE(Device::codeFromString("   ").has_value());
    ASSERT_FALSE(Device::codeFromString("KEY_OF_C_MINOR").has_value());
    ASSERT_FALSE(Device::codeFromString("UNKNOWN").has_value());
}

TEST(DevKeyNamesTests, AChordCarriesItsModifiers)
{
    const std::optional<KeyWithModifiers> plain = Device::chordFromString("KEY_W");
    ASSERT_TRUE(plain.has_value());
    ASSERT_EQ(Device::KeyCode::KEY_W, plain->keyCode());
    ASSERT_EQ(KeyModifierFlags(), plain->modifiers());

    const std::optional<KeyWithModifiers> chord = Device::chordFromString("Ctrl+Shift+F10");
    ASSERT_TRUE(chord.has_value());
    ASSERT_EQ(Device::KeyCode::F10, chord->keyCode());
    ASSERT_EQ(Device::KeyModifier::Ctrl | Device::KeyModifier::Shift, chord->modifiers());
}

TEST(DevKeyNamesTests, ModifierOrderDoesNotMatter)
{
    ASSERT_EQ(Device::chordFromString("Ctrl+Alt+DELETE"), Device::chordFromString("alt+ctrl+delete"));
}

TEST(DevKeyNamesTests, TheKeypadPlusIsAKeyNotASeparator)
{
    const std::optional<KeyWithModifiers> chord = Device::chordFromString("Shift+PLUS_PAD");
    ASSERT_TRUE(chord.has_value());
    ASSERT_EQ(Device::KeyCode::PLUS_PAD, chord->keyCode());
    ASSERT_EQ(KeyModifierFlags(Device::KeyModifier::Shift), chord->modifiers());
}

TEST(DevKeyNamesTests, ABadChordIsRejected)
{
    ASSERT_FALSE(Device::chordFromString("Hyper+KEY_W").has_value());
    ASSERT_FALSE(Device::chordFromString("Ctrl+").has_value());
    ASSERT_FALSE(Device::chordFromString("+KEY_W").has_value());
    ASSERT_FALSE(Device::chordFromString("").has_value());
}
