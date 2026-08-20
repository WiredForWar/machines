/*
 * K E Y T R A N S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "device/KeyToCommandTranslator.hpp"
#include "device/KeyToCommand.hpp"
#include "device/ButtonEvent.hpp"
#include "device/EventQueue.hpp"

bool DevKeyToCommandTranslator::Command::on() const
{
    return on_;
}

void DevKeyToCommandTranslator::Command::forceOn()
{
    if (! on_)
    {
        on_ = true;
        reset_ = true;
    }
}

DevKeyToCommandTranslator::DevKeyToCommandTranslator()
{
    commandTranslations_.reserve(256);
    TEST_INVARIANT;
}

DevKeyToCommandTranslator::~DevKeyToCommandTranslator()
{
    TEST_INVARIANT;

    for (size_t i = 0; i < commandTranslations_.size(); ++i)
    {
        delete commandTranslations_[i];
    }
}

void DevKeyToCommandTranslator::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

void DevKeyToCommandTranslator::addTranslation(const DevKeyToCommand& newTrans)
{
    commandTranslations_.push_back(new DevKeyToCommand(newTrans));
}

bool DevKeyToCommandTranslator::translate(const DevButtonEvent& buttonEvent, CommandId* pCommandId)
{
    PRE(pCommandId);

    const KeyWithModifiers keyWithMods(buttonEvent.scanCode(), buttonEvent.modifiers());

    for (const DevKeyToCommand* pTranslation : commandTranslations_)
    {
        if (pTranslation->matches(keyWithMods))
        {
            *pCommandId = pTranslation->commandId();
            return true;
        }
    }

    return false;
}

bool DevKeyToCommandTranslator::translate(const DevButtonEvent& buttonEvent, CommandList* pCommandList)
{
    PRE(pCommandList);

    CommandList& commandList = *pCommandList;
    bool found = false; // Returned to caller to indicate that a command using the
                        // buttonEvent was found.

    const KeyWithModifiers keyWithMods(buttonEvent.scanCode(), buttonEvent.modifiers());

    // Process key presses...
    if (buttonEvent.action() == DevButtonEvent::PRESS)
    {
        for (const DevKeyToCommand* pTranslation : commandTranslations_)
        {
            ASSERT(pTranslation->commandId() < commandList.size(), "command list does not contain commandId")
            for (const KeyBind& bind : pTranslation->binds())
            {
                if (keyWithMods.modifiers() & bind.releasedModifiers)
                {
                    commandList[pTranslation->commandId()].reset_ = true;
                    continue;
                }

                if (bind.keyCode() == buttonEvent.scanCode())
                {
                    found = true;

                    // The button has been pressed but the command isn't necessarily turned on. This is
                    // dependant on whether or not the shift, control and alt keys are in the correct
                    // state.
                    commandList[pTranslation->commandId()].pressed_ = true;
                }

                if (bind.modifiersMatch(keyWithMods))
                {
                    if (commandList[pTranslation->commandId()].pressed_)
                    {
                        commandList[pTranslation->commandId()].on_ = true;
                        commandList[pTranslation->commandId()].reset_ = false;
                    }
                }
                else
                {
                    commandList[pTranslation->commandId()].reset_ = true;
                }
            }
        }
    }
    else // Key released...
    {
        for (const DevKeyToCommand* pTranslation : commandTranslations_)
        {
            ASSERT(pTranslation->commandId() < commandList.size(), "command list does not contain commandId");
            for (const KeyBind& bind : pTranslation->binds())
            {
                if (bind.keyCode() == buttonEvent.scanCode())
                {
                    commandList[pTranslation->commandId()].reset_ = true;
                    commandList[pTranslation->commandId()].pressed_ = false;
                    found = true;
                }
                else
                {
                    if (bind.modifiersMatch(keyWithMods))
                    {
                        if (commandList[pTranslation->commandId()].pressed_)
                        {
                            commandList[pTranslation->commandId()].on_ = true;
                            commandList[pTranslation->commandId()].reset_ = false;
                        }
                    }
                    else
                    {
                        commandList[pTranslation->commandId()].reset_ = true;
                    }
                }
            }
        }
    }

    return found;
}

void DevKeyToCommandTranslator::initEventQueue()
{
    for (const DevKeyToCommand* pTranslation : commandTranslations_)
    {
        for (const auto bind : pTranslation->binds())
        {
            DevEventQueue::instance().queueEvents(bind.keyCode());
        }
    }

    DevEventQueue::instance().queueEvents(Device::KeyCode::LEFT_SHIFT);
    DevEventQueue::instance().queueEvents(Device::KeyCode::RIGHT_SHIFT);
    DevEventQueue::instance().queueEvents(Device::KeyCode::LEFT_CONTROL);
    DevEventQueue::instance().queueEvents(Device::KeyCode::RIGHT_CONTROL);
    DevEventQueue::instance().queueEvents(Device::KeyCode::LEFT_ALT);
    DevEventQueue::instance().queueEvents(Device::KeyCode::RIGHT_ALT);
}

void DevKeyToCommandTranslator::resetCommands(CommandList* pCommandList, bool forceReset /* = false*/)
{
    CommandList& commandList = *pCommandList;
    for (Command& command : commandList)
    {
        if (forceReset)
        {
            command.on_ = false;
            command.pressed_ = false;
        }
        else
        {
            command.on_ = command.on_ && !command.reset_;
        }

        command.reset_ = false;
    }
}

std::ostream& operator<<(std::ostream& o, const DevKeyToCommandTranslator& t)
{

    o << "DevKeyToCommandTranslator " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "DevKeyToCommandTranslator " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End MCKEYBRD.CPP *************************************************/
