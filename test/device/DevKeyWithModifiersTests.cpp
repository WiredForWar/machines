#include <gtest/gtest.h>

#include "device/KeyWithModifiers.hpp"

///////////////////////////////////////////////////////////////////////////////

TEST(DevKeyWithModifiersTests, ACodeBeyondAByteKeepsItsModifiers)
{
    // The code and the modifiers no longer share a word, so a code above 255
    // cannot collide with a modifier bit. Nothing in KeyCode reaches this far
    // yet; a pad or hand controller will.
    const Device::KeyCode farCode = static_cast<Device::KeyCode>(4000);
    const KeyWithModifiers chord(farCode, Device::KeyModifier::Ctrl | Device::KeyModifier::Shift);

    ASSERT_EQ(farCode, chord.keyCode());
    ASSERT_EQ(KeyModifierFlags(Device::KeyModifier::Ctrl) | Device::KeyModifier::Shift, chord.modifiers());
}

TEST(DevKeyWithModifiersTests, ModifiersDefaultToNone)
{
    const KeyWithModifiers chord(Device::KeyCode::KEY_W);

    ASSERT_EQ(Device::KeyCode::KEY_W, chord.keyCode());
    ASSERT_FALSE(chord.modifiers());
    ASSERT_EQ(KeyModifierFlags(), chord.modifiers());
}

TEST(DevKeyWithModifiersTests, TwoChordsDifferWhenOnlyTheModifiersDo)
{
    const KeyWithModifiers plain(Device::KeyCode::KEY_W);
    const KeyWithModifiers withCtrl(Device::KeyCode::KEY_W, Device::KeyModifier::Ctrl);

    ASSERT_NE(plain, withCtrl);
    ASSERT_EQ(plain.keyCode(), withCtrl.keyCode());
}

TEST(DevKeyWithModifiersTests, FromCtrlAltShiftStateSetsWhatItIsGiven)
{
    const KeyModifierFlags flags = KeyModifierFlags::fromCtrlAltShiftState(true, false, true);

    ASSERT_TRUE(flags & KeyModifierFlags(Device::KeyModifier::Ctrl));
    ASSERT_FALSE(flags & KeyModifierFlags(Device::KeyModifier::Alt));
    ASSERT_TRUE(flags & KeyModifierFlags(Device::KeyModifier::Shift));
}
