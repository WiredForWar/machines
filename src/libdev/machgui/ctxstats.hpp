/*
 * C T X S T A T S . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxStatistics

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXSTATS_HPP
#define _MACHGUI_CTXSTATS_HPP

#include "base/base.hpp"
#include "machgui/startctx.hpp"
#include "ctl/pvector.hpp"

class MachGuiStatistics;

class MachGuiCtxStatistics : public MachGuiStartupScreenContext
// Canonical form revoked
{
public:
    MachGuiCtxStatistics(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxStatistics() override;

    void CLASS_INVARIANT;

    void update() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxStatistics& t);

    MachGuiCtxStatistics(const MachGuiCtxStatistics&);
    MachGuiCtxStatistics& operator=(const MachGuiCtxStatistics&);

    using Statistics = ctl_pvector<MachGuiStatistics>;
    Statistics statistics_;
    int delayBarsGrowth_;
};

#endif

/* End CTXSTATS.HPP *************************************************/
