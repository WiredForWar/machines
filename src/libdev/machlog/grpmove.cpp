/*
 * G R P M O V E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/grpmove.hpp"
#include "machlog/internal/grpmovei.hpp"

MachLogGroupSimpleMove::MachLogGroupSimpleMove(
    const Actors& actors,
    const Points& points,
    size_t commandId,
    string* pReason,
    PhysPathFindingPriority pathFindingPriority)
    : pImpl_(new MachLogGroupSimpleMoveImplementation(actors, points, commandId, pReason, pathFindingPriority))
{
    TEST_INVARIANT;
}

MachLogGroupSimpleMove::~MachLogGroupSimpleMove()
{
    TEST_INVARIANT;

    delete pImpl_;
}

bool MachLogGroupSimpleMove::moveOK() const
{
    PRE(pImpl_ != nullptr);

    return pImpl_->moveOK();
}

void MachLogGroupSimpleMove::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogGroupSimpleMove& t)
{

    o << "MachLogGroupSimpleMove " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogGroupSimpleMove " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End GRPMOVE.CPP **************************************************/
