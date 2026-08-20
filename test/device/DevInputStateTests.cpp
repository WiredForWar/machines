#include <gtest/gtest.h>

#include "device/InputState.hpp"

namespace
{

Device::InputState& freshState()
{
    Device::InputState& state = Device::InputState::instance();
    state.releaseAllButtons();
    state.takePointerMotion();
    state.setPointerPosition({});
    return state;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(DevInputStateTests, AButtonReadsAsHeldUntilReleased)
{
    Device::InputState& state = freshState();

    ASSERT_FALSE(state.isButtonPressed(Device::KeyCode::KEY_W));

    state.pressButton(Device::KeyCode::KEY_W);
    ASSERT_TRUE(state.isButtonPressed(Device::KeyCode::KEY_W));

    state.releaseButton(Device::KeyCode::KEY_W);
    ASSERT_FALSE(state.isButtonPressed(Device::KeyCode::KEY_W));
}

TEST(DevInputStateTests, AMouseButtonIsNotAKey)
{
    Device::InputState& state = freshState();

    state.pressButton(Device::KeyCode::MOUSE_LEFT);

    // Both live in one map and one code space...
    ASSERT_TRUE(state.isButtonPressed(Device::KeyCode::MOUSE_LEFT));

    // ...but a held mouse button must not read as a held key. The keyboard
    // repeat timer restarts on any key being down, and a button held for a
    // drag would otherwise hold it at zero.
    ASSERT_FALSE(state.isAnyKeyPressed());

    state.pressButton(Device::KeyCode::KEY_W);
    ASSERT_TRUE(state.isAnyKeyPressed());
}

TEST(DevInputStateTests, PressingATwiceHeldButtonChangesNothing)
{
    Device::InputState& state = freshState();

    state.pressButton(Device::KeyCode::KEY_W);
    state.pressButton(Device::KeyCode::KEY_W);
    state.releaseButton(Device::KeyCode::KEY_W);

    // One release answers two presses: the count and the map cannot drift
    // apart, which is what the class invariant asserts.
    ASSERT_FALSE(state.isButtonPressed(Device::KeyCode::KEY_W));
    ASSERT_FALSE(state.isAnyKeyPressed());
}

TEST(DevInputStateTests, ReleasingAnUnheldButtonChangesNothing)
{
    Device::InputState& state = freshState();

    state.releaseButton(Device::KeyCode::KEY_W);

    ASSERT_FALSE(state.isButtonPressed(Device::KeyCode::KEY_W));
    ASSERT_FALSE(state.isAnyKeyPressed());
}

TEST(DevInputStateTests, HeldModifiersReportBothSidesOfEachKey)
{
    Device::InputState& state = freshState();

    ASSERT_FALSE(state.heldModifiers());

    state.pressButton(Device::KeyCode::RIGHT_SHIFT);
    ASSERT_EQ(KeyModifierFlags(Device::KeyModifier::Shift), state.heldModifiers());

    state.pressButton(Device::KeyCode::LEFT_CONTROL);
    ASSERT_EQ(Device::KeyModifier::Shift | Device::KeyModifier::Ctrl, state.heldModifiers());

    state.releaseButton(Device::KeyCode::RIGHT_SHIFT);
    ASSERT_EQ(KeyModifierFlags(Device::KeyModifier::Ctrl), state.heldModifiers());
}

TEST(DevInputStateTests, ReleaseAllButtonsClearsEverything)
{
    Device::InputState& state = freshState();

    state.pressButton(Device::KeyCode::KEY_W);
    state.pressButton(Device::KeyCode::MOUSE_LEFT);
    state.pressButton(Device::KeyCode::LEFT_ALT);

    state.releaseAllButtons();

    ASSERT_FALSE(state.isButtonPressed(Device::KeyCode::KEY_W));
    ASSERT_FALSE(state.isButtonPressed(Device::KeyCode::MOUSE_LEFT));
    ASSERT_FALSE(state.isAnyKeyPressed());
    ASSERT_FALSE(state.heldModifiers());
}

TEST(DevInputStateTests, ThePointerPositionIsWhatWasLastSet)
{
    Device::InputState& state = freshState();

    state.setPointerPosition({ .x = 120, .y = 240 });

    ASSERT_EQ(120, state.pointerPosition().x);
    ASSERT_EQ(240, state.pointerPosition().y);
}

TEST(DevInputStateTests, PointerTravelAccumulatesAndIsTakenOnce)
{
    Device::InputState& state = freshState();

    ASSERT_TRUE(state.takePointerMotion().isZero());

    state.addPointerMotion(3.0, -2.0);
    state.addPointerMotion(1.5, -0.5);

    const Device::PointerMotion travelled = state.takePointerMotion();
    ASSERT_DOUBLE_EQ(4.5, travelled.x);
    ASSERT_DOUBLE_EQ(-2.5, travelled.y);

    ASSERT_TRUE(state.takePointerMotion().isZero());
}

TEST(DevInputStateTests, TravelIsIndependentOfThePosition)
{
    Device::InputState& state = freshState();

    // A captured pointer travels without its position changing, so neither
    // value may be derived from the other.
    state.setPointerPosition({ .x = 10, .y = 10 });
    state.addPointerMotion(50.0, -50.0);

    ASSERT_EQ(10, state.pointerPosition().x);
    ASSERT_EQ(10, state.pointerPosition().y);
    ASSERT_DOUBLE_EQ(50.0, state.takePointerMotion().x);
}
