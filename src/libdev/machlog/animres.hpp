/*
 * A N I M R E S . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogResearchAnimation

    Players a research animation..typing keyboard etc
*/

#ifndef _MACHLOG_ANIMRES_HPP
#define _MACHLOG_ANIMRES_HPP

#include "base/base.hpp"
#include "machlog/animatio.hpp"

class MachLogTechnician;

class MachLogResearchAnimation : public MachLogAnimation
// cannonical form revoked
{
public:
    MachLogResearchAnimation(MachLogTechnician* pActor);

    ~MachLogResearchAnimation() override;

    ///////////////////////////////
    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogResearchAnimation);
    PER_FRIEND_READ_WRITE(MachLogResearchAnimation);

protected:
    PhysRelativeTime doStartAnimation() override;

    PhysRelativeTime doUpdate() override;

    void doOutputOperator(std::ostream&) const override;

private:
    MachLogResearchAnimation(const MachLogResearchAnimation&);
    MachLogResearchAnimation& operator=(const MachLogResearchAnimation&);
    bool operator==(const MachLogResearchAnimation&) const;

    MachLogTechnician* pActor_;
};

PER_DECLARE_PERSISTENT(MachLogResearchAnimation);

#endif

/* End ANIMRES.HPP **************************************************/
