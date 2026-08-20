#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "device/KeyWithModifiers.hpp"
#include "device/KeyToCommandTranslator.hpp"

using ::testing::TypedEq;
using ::testing::Return;

TEST(DevKeyToCommandTranslator, TranslateNormalKeys)
{
    DevKeyToCommand::CommandId cmd0 = 0;
    DevKeyToCommand::CommandId cmd1 = 1;
    DevKeyToCommandTranslator translator;

    translator.addTranslation(DevKeyToCommand(Device::KeyCode::KEY_A, cmd0));
    translator.addTranslation(DevKeyToCommand(Device::KeyCode::KEY_B, cmd1));

    {
        DevButtonEvent event1 {
            Device::KeyCode::KEY_A, DevButtonEvent::PRESS, false, KeyModifierFlags(), 10000.0, 20, 20, 1, 'A'
        };
        DevButtonEvent event2 {
            Device::KeyCode::KEY_A, DevButtonEvent::RELEASE, false, KeyModifierFlags(), 10000.0, 20, 20, 1, 'A'
        };

        DevKeyToCommandTranslator::CommandId commandId;
        bool found = translator.translate(event1, &commandId);
        ASSERT_TRUE(found);
        ASSERT_EQ(commandId, cmd0);

        found = translator.translate(event2, &commandId);
        ASSERT_TRUE(found);
        ASSERT_EQ(commandId, cmd0);
    }

    {
        DevButtonEvent event1 {
            Device::KeyCode::KEY_B, DevButtonEvent::PRESS, false, KeyModifierFlags(), 10000.0, 20, 20, 1, 'B'
        };
        DevButtonEvent event2 {
            Device::KeyCode::KEY_B, DevButtonEvent::RELEASE, false, KeyModifierFlags(), 10000.0, 20, 20, 1, 'B'
        };

        DevKeyToCommandTranslator::CommandId commandId;
        bool found = translator.translate(event1, &commandId);
        ASSERT_TRUE(found);
        ASSERT_EQ(commandId, cmd1);

        found = translator.translate(event2, &commandId);
        ASSERT_TRUE(found);
        ASSERT_EQ(commandId, cmd1);
    }
}

TEST(DevKeyToCommandTranslator, TranslateListNormalKeys)
{
    DevKeyToCommand::CommandId cmd0 = 0;
    DevKeyToCommand::CommandId cmd1 = 1;
    DevKeyToCommandTranslator translator;

    translator.addTranslation(DevKeyToCommand(Device::KeyCode::KEY_A, cmd0));
    translator.addTranslation(DevKeyToCommand(Device::KeyCode::KEY_B, cmd1));

    DevKeyToCommandTranslator::CommandList commandList(2);
    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    {
        DevButtonEvent evPressed {
            Device::KeyCode::KEY_A, DevButtonEvent::PRESS, false, KeyModifierFlags(), 10000.0, 20, 20, 1, 'A'
        };

        bool found = translator.translate(evPressed, &commandList);
        ASSERT_TRUE(found);
    }

    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    translator.resetCommands(&commandList);

    // Check that the reset didn't change anything
    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    {
        DevButtonEvent evReleased {
            Device::KeyCode::KEY_A, DevButtonEvent::RELEASE, false, KeyModifierFlags(), 10000.0, 20, 20, 1, 'A'
        };

        bool found = translator.translate(evReleased, &commandList);
        ASSERT_TRUE(found);
    }

    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    translator.resetCommands(&commandList);

    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    {
        DevButtonEvent evPressed {
            Device::KeyCode::KEY_B, DevButtonEvent::PRESS, false, KeyModifierFlags(), 10000.0, 20, 20, 1, 'B'
        };

        bool found = translator.translate(evPressed, &commandList);
        ASSERT_TRUE(found);
    }

    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_TRUE(commandList[cmd1].on());

    translator.resetCommands(&commandList);

    // Check that the reset didn't change anything
    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_TRUE(commandList[cmd1].on());

    {
        DevButtonEvent evReleased {
            Device::KeyCode::KEY_B, DevButtonEvent::RELEASE, false, KeyModifierFlags(), 10000.0, 20, 20, 1, 'B'
        };

        bool found = translator.translate(evReleased, &commandList);
        ASSERT_TRUE(found);

        ASSERT_FALSE(commandList[cmd0].on());
        ASSERT_TRUE(commandList[cmd1].on());
    }

    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_TRUE(commandList[cmd1].on());

    translator.resetCommands(&commandList);

    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());
}

// A release reporting no modifiers must switch a command off even when the
// press that switched it on had a modifier held. This is what a key released
// outside the window looks like once focus comes back.
TEST(DevKeyToCommandTranslator, ReleaseWithoutModifiersUnlatchesModifiedCommand)
{
    DevKeyToCommand::CommandId cmd0 = 0;
    DevKeyToCommandTranslator translator;

    translator.addTranslation(DevKeyToCommand(
        Device::KeyCode::KEY_A,
        cmd0,
        DevKeyToCommand::CTRLKEY_PRESSED,
        DevKeyToCommand::SHIFTKEY_RELEASED));

    DevKeyToCommandTranslator::CommandList commandList(1);

    // Ctrl+A goes down and latches the command on.
    {
        DevButtonEvent pressWithCtrl {
            Device::KeyCode::KEY_A, DevButtonEvent::PRESS, false, KeyModifierFlags(Device::KeyModifier::Ctrl), 10000.0, 20, 20, 1, 'A'
        };
        ASSERT_TRUE(translator.translate(pressWithCtrl, &commandList));
    }

    ASSERT_TRUE(commandList[cmd0].on());

    translator.resetCommands(&commandList);
    ASSERT_TRUE(commandList[cmd0].on());

    // The release carries no modifiers, unlike the press that turned it on.
    {
        DevButtonEvent releaseWithoutCtrl {
            Device::KeyCode::KEY_A, DevButtonEvent::RELEASE, true, KeyModifierFlags(), 10001.0, 20, 20, 1, 'A'
        };
        ASSERT_TRUE(translator.translate(releaseWithoutCtrl, &commandList));
    }

    translator.resetCommands(&commandList);
    ASSERT_FALSE(commandList[cmd0].on());
}

TEST(DevKeyToCommandTranslator, TranslateListKeyWithMods)
{
    DevKeyToCommand::CommandId cmd0 = 0;
    DevKeyToCommand::CommandId cmd1 = 1;
    DevKeyToCommandTranslator translator;

    translator.addTranslation(DevKeyToCommand(Device::KeyCode::KEY_A, cmd0, DevKeyToCommand::CTRLKEY_EITHER, DevKeyToCommand::SHIFTKEY_RELEASED));
    translator.addTranslation(DevKeyToCommand(Device::KeyCode::KEY_A, cmd1, DevKeyToCommand::CTRLKEY_RELEASED, DevKeyToCommand::SHIFTKEY_PRESSED));

    DevKeyToCommandTranslator::CommandList commandList(2);
    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    KeyModifierFlags pressedModifiers{};
    {
        DevButtonEvent buttonEvent {
            Device::KeyCode::KEY_A,
            DevButtonEvent::PRESS,
            false,
            pressedModifiers,
            10000.0,
            20,
            20,
            1,
            'A',
        };

        bool found = translator.translate(buttonEvent, &commandList);
        ASSERT_TRUE(found);
    }

    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    translator.resetCommands(&commandList);

    // Check that the reset didn't change anything
    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    pressedModifiers = Device::KeyModifier::Shift;
    {
        DevButtonEvent buttonEvent {
            Device::KeyCode::RIGHT_SHIFT,
            DevButtonEvent::PRESS,
            false,
            pressedModifiers,
            10000.0,
            20,
            20,
            1,
            'A',
        };

        bool found = translator.translate(buttonEvent, &commandList);
        ASSERT_FALSE(found);
    }

    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_TRUE(commandList[cmd1].on());

    translator.resetCommands(&commandList);

    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_TRUE(commandList[cmd1].on());

    pressedModifiers = Device::KeyModifier::None;
    {
        DevButtonEvent buttonEvent {
            Device::KeyCode::RIGHT_SHIFT,
            DevButtonEvent::RELEASE,
            false,
            pressedModifiers,
            10000.0,
            20,
            20,
            1,
            'A',
        };

        bool found = translator.translate(buttonEvent, &commandList);
        ASSERT_FALSE(found);
    }

    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_TRUE(commandList[cmd1].on());

    translator.resetCommands(&commandList);

    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    {
        DevButtonEvent buttonEvent {
            Device::KeyCode::KEY_A,
            DevButtonEvent::RELEASE,
            false,
            pressedModifiers,
            10000.0,
            20,
            20,
            1,
            'A',
        };

        bool found = translator.translate(buttonEvent, &commandList);
        ASSERT_TRUE(found);
    }

    ASSERT_TRUE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());

    translator.resetCommands(&commandList);

    ASSERT_FALSE(commandList[cmd0].on());
    ASSERT_FALSE(commandList[cmd1].on());
}
