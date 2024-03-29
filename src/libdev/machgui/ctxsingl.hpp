/*
 * C T X S I N G L . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxSinglePlayer

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXSINGL_HPP
#define _MACHGUI_CTXSINGL_HPP

#include "base/base.hpp"
#include "machgui/startctx.hpp"
#include "machgui/animatio.hpp"

class MachGuiCtxSinglePlayer : public MachGuiStartupScreenContext
// Canonical form revoked
{
public:
    MachGuiCtxSinglePlayer(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxSinglePlayer() override;

    void CLASS_INVARIANT;

    void update() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxSinglePlayer& t);

    MachGuiCtxSinglePlayer(const MachGuiCtxSinglePlayer&);
    MachGuiCtxSinglePlayer& operator=(const MachGuiCtxSinglePlayer&);

    MachGuiAnimations animations_;
};

#endif

/* End CTXSINGL.HPP *************************************************/
