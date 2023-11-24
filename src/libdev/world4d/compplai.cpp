/*
 * C O M P P L A I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "world4d/entyplan.hpp"
#include "world4d/planentr.hpp"
#include "world4d/internal/compplai.hpp"

PER_DEFINE_PERSISTENT(W4dCompositePlanImpl);

W4dCompositePlanImpl::W4dCompositePlanImpl()
    : pCompositePlan_(nullptr)
    , cachedFinishTime_(0.0)
{

    TEST_INVARIANT;
}

W4dCompositePlanImpl::~W4dCompositePlanImpl()
{
    TEST_INVARIANT;

    // Delete the composite's plan
    if (pCompositePlan_)
        delete pCompositePlan_;

    // Delete all the link entries
    while (entries_.size() != 0)
    {
        delete entries_.front();
        entries_.pop_front();
    }
}

void W4dCompositePlanImpl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const W4dCompositePlanImpl& t)
{

    o << "W4dCompositePlanImpl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "W4dCompositePlanImpl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const W4dCompositePlanImpl& ob)
{
    ostr << ob.pCompositePlan_;
    ostr << ob.entries_;
    ostr << ob.name_;
    ostr << ob.cachedFinishTime_;
}

void perRead(PerIstream& istr, W4dCompositePlanImpl& ob)
{
    istr >> ob.pCompositePlan_;
    istr >> ob.entries_;
    istr >> ob.name_;
    istr >> ob.cachedFinishTime_;
}

/* End COMPPLAI.CPP *************************************************/
