/*
 * K E Y C O M N D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "device/KeyToCommand.hpp"

#include "base/PrePost.hpp"

DevKeyToCommand::DevKeyToCommand(CommandId command, KeyBindsPtr binds)
    : commandId_(command)
    , bindsRef_(binds)
{
    PRE(binds != nullptr);
}

DevKeyToCommand::DevKeyToCommand(
    DevKeyToCommand::ScanCode sc,
    DevKeyToCommand::CommandId comId,
    DevKeyToCommand::Modifier ctrl,
    DevKeyToCommand::Modifier shift,
    DevKeyToCommand::Modifier alt)
{
    commandId_ = comId;
    binds_ = {{
        .keyWithMods = { sc
                         | KeyModifierFlags::fromCtrlAltShiftState(
                             ctrl == Modifier::PRESSED,
                             alt == Modifier::PRESSED,
                             shift == Modifier::PRESSED) },
        .releasedModifiers = KeyModifierFlags::fromCtrlAltShiftState(
            ctrl == Modifier::RELEASED,
            alt == Modifier::RELEASED,
            shift == Modifier::RELEASED),
    }};
}

DevKeyToCommand::DevKeyToCommand(
    ScanCode sc,
    CommandId comId,
    CtrlModifier ctrl /*= CTRL_EITHER*/,
    ShiftModifier shift /*= SHIFT_EITHER*/,
    AltModifier alt /*= ALT_EITHER*/)
{
    commandId_ = comId;
    binds_ = {{
        .keyWithMods = { sc
                         | KeyModifierFlags::fromCtrlAltShiftState(
                             ctrl == CTRLKEY_PRESSED,
                             alt == ALTKEY_PRESSED,
                             shift == SHIFTKEY_PRESSED) },
        .releasedModifiers = KeyModifierFlags::fromCtrlAltShiftState(
            ctrl == CTRLKEY_RELEASED,
            alt == ALTKEY_RELEASED,
            shift == SHIFTKEY_RELEASED),
    }};
}

DevKeyToCommand::CommandId DevKeyToCommand::commandId() const
{
    return commandId_;
}

const KeyBinds& DevKeyToCommand::binds() const
{
    return bindsRef_ ? *bindsRef_ : binds_;
}

bool DevKeyToCommand::matches(const KeyWithModifiers& key) const
{
    return binds().matches(key);
}
