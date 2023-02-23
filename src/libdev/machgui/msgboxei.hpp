/*
 * M S G B O X E I . H P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

/*
    MachGuiExitToInternetMessageBoxResponder

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_MSGBOXEI_HPP
#define _MACHGUI_MSGBOXEI_HPP

#include "base/base.hpp"
#include "machgui/msgbox.hpp"

class MachGuiStartupScreens;

class MachGuiExitToInternetMessageBoxResponder : public MachGuiMessageBoxResponder
// Canonical form revoked
{
public:
    enum UnloadGame
    {
        UNLOAD_GAME,
        DO_NOT_UNLOAD_GAME
    };

    MachGuiExitToInternetMessageBoxResponder(MachGuiStartupScreens*, UnloadGame);

    ~MachGuiExitToInternetMessageBoxResponder() override;

    bool okPressed() override;

    void CLASS_INVARIANT;

private:
    friend ostream& operator<<(ostream& o, const MachGuiExitToInternetMessageBoxResponder& t);

    MachGuiExitToInternetMessageBoxResponder(const MachGuiExitToInternetMessageBoxResponder&);
    MachGuiExitToInternetMessageBoxResponder& operator=(const MachGuiExitToInternetMessageBoxResponder&);

    MachGuiStartupScreens* pStartupScreens_;
    UnloadGame unloadGame_;
};

#endif

/* End MSGBOXEI.HPP *************************************************/
