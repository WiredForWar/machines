/*
 * S Y S M E S S . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogSystemMessageHandler

    Handles network system messages at the machlog level - e.g. reassign host etc.
*/

#ifndef _MACHLOG_SYSMESS_HPP
#define _MACHLOG_SYSMESS_HPP

#include "base/base.hpp"
#include "network/sysmess.hpp"
#include "machphys/machphys.hpp"

class MachLogSystemMessageHandler : public NetSystemMessageHandler
// Canonical form revoked
{
public:
    MachLogSystemMessageHandler();
    ~MachLogSystemMessageHandler() override;

    bool handleHostMessage() override;
    bool handleDestroyPlayerMessage(const std::string&) override;
    bool handleSessionLostMessage() override;

    void CLASS_INVARIANT;

protected:
    bool logicalHandleDestroyPlayerMessage(MachPhys::Race);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachLogSystemMessageHandler& t);

    MachLogSystemMessageHandler(const MachLogSystemMessageHandler&);
    MachLogSystemMessageHandler& operator=(const MachLogSystemMessageHandler&);
};

#endif

/* End SYSMESS.HPP **************************************************/
