/*
 * K E Y T R A N S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "device/keytrans.hpp"
#include "device/keycomnd.hpp"
#include "device/butevent.hpp"
#include "device/eventq.hpp"

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

DevKeyToCommandTranslator::Command::Command()
    : on_(false)
    , pressed_(false)
    , reset_(false)
{
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

    bool found = false; // Returned to caller to indicate that a command using the
                        // buttonEvent was found.

    for (size_t i = 0; i < commandTranslations_.size() && ! found; ++i)
    {
        DevKeyToCommand* pEvent = commandTranslations_[i];
        if (pEvent->scanCode() == buttonEvent.scanCode())
        {
            bool ctrlCorrect = (pEvent->ctrlReq() == DevKeyToCommand::EITHER)
                || (pEvent->ctrlReq() == DevKeyToCommand::PRESSED && buttonEvent.wasCtrlPressed())
                || (pEvent->ctrlReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasCtrlPressed());
            bool shiftCorrect = (pEvent->shiftReq() == DevKeyToCommand::EITHER)
                || (pEvent->shiftReq() == DevKeyToCommand::PRESSED && buttonEvent.wasShiftPressed())
                || (pEvent->shiftReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasShiftPressed());
            bool altCorrect = (pEvent->altReq() == DevKeyToCommand::EITHER)
                || (pEvent->altReq() == DevKeyToCommand::PRESSED && buttonEvent.wasAltPressed())
                || (pEvent->altReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasAltPressed());

            if (ctrlCorrect && shiftCorrect && altCorrect)
            {
                *pCommandId = pEvent->commandId();
                found = true;
            }
        }
    }

    return found;
}

bool DevKeyToCommandTranslator::translate(const DevButtonEvent& buttonEvent, CommandList* pCommandList)
{
    PRE(pCommandList);

    CommandList& commandList = *pCommandList;
    bool found = false; // Returned to caller to indicate that a command using the
                        // buttonEvent was found.

    // Process key presses...
    if (buttonEvent.action() == DevButtonEvent::PRESS)
    {
        for (const DevKeyToCommand* pTranslation : commandTranslations_)
        {
            ASSERT(pTranslation->commandId() < commandList.size(), "command list does not contain commandId")

            // Find out if modifier keys are in correct state for this command
            bool ctrlCorrect = (pTranslation->ctrlReq() == DevKeyToCommand::EITHER)
                || (pTranslation->ctrlReq() == DevKeyToCommand::PRESSED && buttonEvent.wasCtrlPressed())
                || (pTranslation->ctrlReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasCtrlPressed());
            bool shiftCorrect = (pTranslation->shiftReq() == DevKeyToCommand::EITHER)
                || (pTranslation->shiftReq() == DevKeyToCommand::PRESSED && buttonEvent.wasShiftPressed())
                || (pTranslation->shiftReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasShiftPressed());
            bool altCorrect = (pTranslation->altReq() == DevKeyToCommand::EITHER)
                || (pTranslation->altReq() == DevKeyToCommand::PRESSED && buttonEvent.wasAltPressed())
                || (pTranslation->altReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasAltPressed());

            if (pTranslation->scanCode() == buttonEvent.scanCode())
            {
                found = true;

                // The button has been pressed but the command isn't necessarily turned on. This is
                // dependant on whether or not the shift, control and alt keys are in the correct
                // state.
                commandList[pTranslation->commandId()].pressed_ = true;
            }

            if (ctrlCorrect && shiftCorrect && altCorrect)
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
    else // Key released...
    {
        for (const DevKeyToCommand* pTranslation : commandTranslations_)
        {
            ASSERT(pTranslation->commandId() < commandList.size(), "command list does not contain commandId");

            // Find out if modifier keys are in correct state for this command
            bool ctrlCorrect = (pTranslation->ctrlReq() == DevKeyToCommand::EITHER)
                || (pTranslation->ctrlReq() == DevKeyToCommand::PRESSED && buttonEvent.wasCtrlPressed())
                || (pTranslation->ctrlReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasCtrlPressed());
            bool shiftCorrect = (pTranslation->shiftReq() == DevKeyToCommand::EITHER)
                || (pTranslation->shiftReq() == DevKeyToCommand::PRESSED && buttonEvent.wasShiftPressed())
                || (pTranslation->shiftReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasShiftPressed());
            bool altCorrect = (pTranslation->altReq() == DevKeyToCommand::EITHER)
                || (pTranslation->altReq() == DevKeyToCommand::PRESSED && buttonEvent.wasAltPressed())
                || (pTranslation->altReq() == DevKeyToCommand::RELEASED && ! buttonEvent.wasAltPressed());

            if (pTranslation->scanCode() == buttonEvent.scanCode())
            {
                commandList[pTranslation->commandId()].reset_ = true;
                commandList[pTranslation->commandId()].pressed_ = false;
                found = true;
            }
            else
            {
                if (ctrlCorrect && shiftCorrect && altCorrect)
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

    return found;
}

void DevKeyToCommandTranslator::initEventQueue()
{
    for (size_t i = 0; i < commandTranslations_.size(); ++i)
    {
        DevKeyToCommand* pEvent = commandTranslations_[i];
        DevEventQueue::instance().queueEvents(pEvent->scanCode());
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
