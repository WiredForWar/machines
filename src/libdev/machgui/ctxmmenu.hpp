/*
 * C T X M M E N U . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxMainMenu

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXMMENU_HPP
#define _MACHGUI_CTXMMENU_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiCtxMainMenu : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxMainMenu(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxMainMenu() override;

    void CLASS_INVARIANT;

    void update() override;

    bool okayToSwitchContext() override;

    void quit();

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxMainMenu& t);

    MachGuiCtxMainMenu(const MachGuiCtxMainMenu&);
    MachGuiCtxMainMenu& operator=(const MachGuiCtxMainMenu&);

    // Data members...
    bool quit_{};
};

#endif

/* End CTXMMENU.HPP *************************************************/
