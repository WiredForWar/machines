/*
 * C T X H O T K Y . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxHotKeys

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXHOTKY_HPP
#define _MACHGUI_CTXHOTKY_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiCtxHotKeys : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxHotKeys(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxHotKeys() override;

    void CLASS_INVARIANT;

    void update() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxHotKeys& t);

    MachGuiCtxHotKeys(const MachGuiCtxHotKeys&);
    MachGuiCtxHotKeys& operator=(const MachGuiCtxHotKeys&);

    void readHotkeyData(const string& hotKeyDataFileName, string& hotkeyString, uint& linesInString);
};

#endif

/* End CTXHOTKY.HPP *************************************************/
