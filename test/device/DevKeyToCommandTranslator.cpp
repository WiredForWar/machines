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
            Device::KeyCode::KEY_A, DevButtonEvent::PRESS, false, false, false, false, 10000.0, 20, 20, 1, 'A'
        };
        DevButtonEvent event2 {
            Device::KeyCode::KEY_A, DevButtonEvent::RELEASE, false, false, false, false, 10000.0, 20, 20, 1, 'A'
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
            Device::KeyCode::KEY_B, DevButtonEvent::PRESS, false, false, false, false, 10000.0, 20, 20, 1, 'B'
        };
        DevButtonEvent event2 {
            Device::KeyCode::KEY_B, DevButtonEvent::RELEASE, false, false, false, false, 10000.0, 20, 20, 1, 'B'
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
            Device::KeyCode::KEY_A, DevButtonEvent::PRESS, false, false, false, false, 10000.0, 20, 20, 1, 'A'
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
            Device::KeyCode::KEY_A, DevButtonEvent::RELEASE, false, false, false, false, 10000.0, 20, 20, 1, 'A'
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
            Device::KeyCode::KEY_B, DevButtonEvent::PRESS, false, false, false, false, 10000.0, 20, 20, 1, 'B'
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
            Device::KeyCode::KEY_B, DevButtonEvent::RELEASE, false, false, false, false, 10000.0, 20, 20, 1, 'B'
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
            pressedModifiers & Device::KeyModifier::Shift,
            pressedModifiers & Device::KeyModifier::Ctrl,
            pressedModifiers & Device::KeyModifier::Alt,
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
            pressedModifiers & Device::KeyModifier::Shift,
            pressedModifiers & Device::KeyModifier::Ctrl,
            pressedModifiers & Device::KeyModifier::Alt,
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
            pressedModifiers & Device::KeyModifier::Shift,
            pressedModifiers & Device::KeyModifier::Ctrl,
            pressedModifiers & Device::KeyModifier::Alt,
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
            pressedModifiers & Device::KeyModifier::Shift,
            pressedModifiers & Device::KeyModifier::Ctrl,
            pressedModifiers & Device::KeyModifier::Alt,
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
