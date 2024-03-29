/*
 * P I N G H E L P . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "network/pinghelp.hpp"

NetPingHelper::NetPingHelper()
{

    TEST_INVARIANT;
}

NetPingHelper::~NetPingHelper()
{
    TEST_INVARIANT;
}

void NetPingHelper::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const NetPingHelper& t)
{

    o << "NetPingHelper " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "NetPingHelper " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void NetPingHelper::pingAll() const
{
    //?void NetNode::ping( const NetNodeUid& recipientUid )
    // NetNode::ping( const NetNodeUid& recipientUid );
}

/* End PINGHELP.CPP *************************************************/
