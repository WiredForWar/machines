/*
 * C T X M U L T I . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxMultiplayer

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXMULTI_HPP
#define _MACHGUI_CTXMULTI_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiSingleLineEditBox;

class MachGuiCtxMultiplayer : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxMultiplayer(MachGuiStartupScreens*);
    ~MachGuiCtxMultiplayer() override;

    bool okayToSwitchContext() override;
    void update() override;

    void CLASS_INVARIANT;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxMultiplayer& t);

    MachGuiCtxMultiplayer(const MachGuiCtxMultiplayer&);
    MachGuiCtxMultiplayer& operator=(const MachGuiCtxMultiplayer&);

    MachGuiSingleLineEditBox* pSingleLineEditBox_;
};

#endif

/* End CTXMULTI.HPP *************************************************/
