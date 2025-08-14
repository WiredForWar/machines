/*
 * K E Y C O M N D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "device/keycomnd.hpp"

DevKeyToCommand::DevKeyToCommand(
    DevKeyToCommand::ScanCode sc,
    DevKeyToCommand::CommandId comId,
    DevKeyToCommand::Modifier ctrl,
    DevKeyToCommand::Modifier shift,
    DevKeyToCommand::Modifier alt)
{
    commandId_ = comId;
    bind_ = {
        .keyWithMods = { sc
                         | KeyModifierFlags::fromCtrlAltShiftState(
                             ctrl == Modifier::PRESSED,
                             alt == Modifier::PRESSED,
                             shift == Modifier::PRESSED) },
        .releasedModifiers = KeyModifierFlags::fromCtrlAltShiftState(
            ctrl == Modifier::RELEASED,
            alt == Modifier::RELEASED,
            shift == Modifier::RELEASED),
    };
}

DevKeyToCommand::DevKeyToCommand(
    ScanCode sc,
    CommandId comId,
    CtrlModifier ctrl /*= CTRL_EITHER*/,
    ShiftModifier shift /*= SHIFT_EITHER*/,
    AltModifier alt /*= ALT_EITHER*/)
{
    commandId_ = comId;
    bind_ = {
        .keyWithMods = { sc
                         | KeyModifierFlags::fromCtrlAltShiftState(
                             ctrl == CTRLKEY_PRESSED,
                             alt == ALTKEY_PRESSED,
                             shift == SHIFTKEY_PRESSED) },
        .releasedModifiers = KeyModifierFlags::fromCtrlAltShiftState(
            ctrl == CTRLKEY_RELEASED,
            alt == ALTKEY_RELEASED,
            shift == SHIFTKEY_RELEASED),
    };
}

DevKeyToCommand::CommandId DevKeyToCommand::commandId() const
{
    return commandId_;
}

std::ostream& operator<<(std::ostream& o, const DevKeyToCommand& t)
{

    o << "DevKeyToCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "DevKeyToCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End KEYCOMND.CPP **************************************************/
