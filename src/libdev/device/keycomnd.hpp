/*
 * K E Y C O M N D . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    DevKeyToCommand

    Used in conjunction with DevKeyToCommandTranslator.
    Indicates what key presses are required to turn a command on. For example, you
    might want Ctrl+Alt+Delete to indicate a Delete command. To do this construct
    using :
    DevKeyToCommand( Device::KeyCode::KEY_DELETE, MYDELETECOMMANDID, Modifier::PRESSED, Modifier::EITHER, Modifier::PRESSED );
*/

#pragma once

#include "device/KeyBind.hpp"

using KeyBindsPtr = const KeyBinds*;

class DevKeyToCommand
// Canonical form
{
public:
    using ScanCode = Device::KeyCode;
    using CommandId = size_t;

    enum Modifier
    {
        PRESSED,
        RELEASED,
        EITHER
    };
    enum CtrlModifier
    {
        CTRLKEY_PRESSED,
        CTRLKEY_RELEASED,
        CTRLKEY_EITHER
    };
    enum ShiftModifier
    {
        SHIFTKEY_PRESSED,
        SHIFTKEY_RELEASED,
        SHIFTKEY_EITHER
    }; //
    enum AltModifier
    {
        ALTKEY_PRESSED,
        ALTKEY_RELEASED,
        ALTKEY_EITHER
    };

    // Old constructor. Kept in for now so that client code doesn't have to change.
    DevKeyToCommand(ScanCode, CommandId, Modifier ctrl, Modifier shift, Modifier alt);
    DevKeyToCommand(CommandId command, KeyBindsPtr binds);

    DevKeyToCommand(
        ScanCode,
        CommandId,
        CtrlModifier = CTRLKEY_EITHER,
        ShiftModifier = SHIFTKEY_EITHER,
        AltModifier = ALTKEY_EITHER);

    CommandId commandId() const;

    const KeyBinds& binds() const;
    bool matches(const KeyWithModifiers& key) const;

private:
    // Revoked
    bool operator==(const DevKeyToCommand&);

    friend std::ostream& operator<<(std::ostream& o, const DevKeyToCommand& t);

    // data members
    CommandId commandId_{};
    const KeyBindsPtr bindsRef_{};
    KeyBinds binds_{};
};
